#include "MatchReporting.hpp"
#include "constants.h"

#include <windows.h>
#include <winhttp.h>
#include <thread>
#include <vector>
#include <cstring>

static constexpr const wchar_t* SERVER_HOST = L"192.168.1.126";
static constexpr INTERNET_PORT SERVER_PORT = 8080;
static constexpr int HTTP_TIMEOUT_MS = 5000;

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
static constexpr const wchar_t* USER_AGENT = WIDE(PROGRAM_TITLE) L"/" WIDE(PROGRAM_VERSION);

std::vector<uint8_t> MatchReporter::PostToServer(const wchar_t* path, const void* data, size_t dataSize)
{
	std::vector<uint8_t> response;

	HINTERNET hSession = WinHttpOpen(USER_AGENT, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) { DEBUG_LOG("[MatchReport] WinHttpOpen failed\n"); return response; }

	HINTERNET hConnect = WinHttpConnect(hSession, SERVER_HOST, SERVER_PORT, 0);
	if (!hConnect) { DEBUG_LOG("[MatchReport] WinHttpConnect failed\n"); WinHttpCloseHandle(hSession); return response; }

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!hRequest) { DEBUG_LOG("[MatchReport] WinHttpOpenRequest failed\n"); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return response; }

	WinHttpSetTimeouts(hRequest, HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS);

	BOOL sent = WinHttpSendRequest(hRequest, L"Content-Type: application/octet-stream", -1, (LPVOID)data, (DWORD)dataSize, (DWORD)dataSize, 0);
	if (sent && WinHttpReceiveResponse(hRequest, NULL)) {
		DWORD bytesRead = 0;
		uint8_t buf[256];
		while (WinHttpReadData(hRequest, buf, sizeof(buf), &bytesRead) && bytesRead > 0) {
			response.insert(response.end(), buf, buf + bytesRead);
			bytesRead = 0;
		}
	} else {
		DEBUG_LOG("[MatchReport] HTTP request failed: %lu\n", GetLastError());
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return response;
}

std::vector<uint8_t> MatchReporter::PostToServerWithTicket(const uint8_t* ticket, uint16_t ticketLen,
	const wchar_t* path, const void* data, size_t dataSize)
{
	if (ticketLen == 0) {
		DEBUG_LOG("[MatchReport] No auth ticket available\n");
		return {};
	}

	std::vector<uint8_t> payload(sizeof(ticketLen) + ticketLen + dataSize);
	size_t offset = 0;
	memcpy(payload.data() + offset, &ticketLen, sizeof(ticketLen)); offset += sizeof(ticketLen);
	memcpy(payload.data() + offset, ticket, ticketLen); offset += ticketLen;
	memcpy(payload.data() + offset, data, dataSize);

	return PostToServer(path, payload.data(), payload.size());
}

void MatchReporter::Poll(SharedMemT7_MatchReport& mr)
{
	if (mr.send_start) {
		mr.send_start = false;

		// Copy data out of shared memory for the thread
		MatchStartReport report = {};
		report.reporter_steam_id = mr.reporter_steam_id;
		report.p1_steam_id = mr.p1_steam_id;
		report.p2_steam_id = mr.p2_steam_id;
		report.p1_char_id = mr.p1_char_id;
		report.p2_char_id = mr.p2_char_id;
		report.stage_id = mr.stage_id;
		memcpy(report.reporter_name, mr.reporter_name, sizeof(report.reporter_name));
		memcpy(report.client_version, mr.client_version, sizeof(report.client_version));

		std::vector<uint8_t> ticket(mr.auth_ticket, mr.auth_ticket + mr.auth_ticket_len);
		uint16_t ticketLen = mr.auth_ticket_len;

		DEBUG_LOG("[MatchReport] Sending start: p1=%llu p2=%llu chars=%u/%u stage=%u\n",
			report.p1_steam_id, report.p2_steam_id,
			report.p1_char_id, report.p2_char_id, report.stage_id);

		// mr is a reference into shared memory — capture pointer for the thread to write match_id back
		SharedMemT7_MatchReport* mrPtr = &mr;
		std::thread([report, ticket, ticketLen, mrPtr]() {
			auto response = PostToServerWithTicket(ticket.data(), ticketLen,
				L"/match/start", &report, sizeof(report));
			if (response.size() >= 16) {
				memcpy(mrPtr->match_id, response.data(), 16);
				DEBUG_LOG("[MatchReport] Got match_id (uuid)\n");
			} else {
				DEBUG_LOG("[MatchReport] Failed to get match_id from server\n");
			}
		}).detach();
	}

	if (mr.send_end) {
		mr.send_end = false;

		uint8_t zero_id[16] = {};
		if (memcmp(mr.match_id, zero_id, 16) == 0) {
			DEBUG_LOG("[MatchReport] No match_id, skipping end report\n");
		} else {
			MatchEndReport report = {};
			memcpy(report.match_id, mr.match_id, 16);
			report.reporter_steam_id = mr.reporter_steam_id;
			report.p1_wins = mr.p1_wins;
			report.p2_wins = mr.p2_wins;
			report.end_reason = mr.end_reason;
			memcpy(report.client_version, mr.client_version, sizeof(report.client_version));

			std::vector<uint8_t> ticket(mr.auth_ticket, mr.auth_ticket + mr.auth_ticket_len);
			uint16_t ticketLen = mr.auth_ticket_len;

			DEBUG_LOG("[MatchReport] Sending end: score=%u-%u reason=%u\n",
				report.p1_wins, report.p2_wins, report.end_reason);

			std::thread([report, ticket, ticketLen]() {
				PostToServerWithTicket(ticket.data(), ticketLen,
					L"/match/end", &report, sizeof(report));
			}).detach();

			memset(mr.match_id, 0, sizeof(mr.match_id));
		}
	}

	if (mr.send_heartbeat) {
		mr.send_heartbeat = false;

		uint8_t zero_id[16] = {};
		if (memcmp(mr.match_id, zero_id, 16) != 0) {
			uint8_t id_copy[16];
			memcpy(id_copy, mr.match_id, 16);

			std::thread([id_copy]() {
				PostToServer(L"/match/heartbeat", id_copy, sizeof(id_copy));
			}).detach();
		}
	}
}
