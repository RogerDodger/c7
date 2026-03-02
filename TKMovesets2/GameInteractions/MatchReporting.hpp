#pragma once

#include <vector>
#include <cstdint>
#include "SharedMemory_t7.h"

// Binary report structs sent over HTTP to the match server
#pragma pack(push, 1)
struct MatchStartReport {
	uint64_t reporter_steam_id;
	uint64_t p1_steam_id;
	uint64_t p2_steam_id;
	uint32_t p1_char_id;
	uint32_t p2_char_id;
	uint32_t stage_id;
	char     reporter_name[128];
	char     client_version[16];
};

struct MatchEndReport {
	uint8_t  match_id[16];
	uint64_t reporter_steam_id;
	uint32_t p1_wins;
	uint32_t p2_wins;
	uint32_t end_reason;
	char     client_version[16];
};
#pragma pack(pop)

class MatchReporter
{
public:
	// Check shared memory flags and dispatch HTTP requests
	void Poll(SharedMemT7_MatchReport& mr);

private:
	// POST raw payload to the server. Returns response body (empty on failure).
	static std::vector<uint8_t> PostToServer(const wchar_t* path, const void* data, size_t dataSize);
	// POST with Steam auth ticket prepended: [u16 ticket_len][ticket][data]
	static std::vector<uint8_t> PostToServerWithTicket(const uint8_t* ticket, uint16_t ticketLen,
		const wchar_t* path, const void* data, size_t dataSize);
};
