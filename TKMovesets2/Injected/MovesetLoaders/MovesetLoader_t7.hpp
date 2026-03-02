#pragma once

#include "MovesetLoader.hpp"

#include "Structs_t7.h"
#include "SharedMemory_t7.h"
#include "steam_api.h"

// -- Packet -- //

enum PacketT7Type_
{
	PacketT7Type_INVALID,
	PacketT7Type_RequestSync,
	PacketT7Type_AnswerSync,
	PacketT7Type_Ready,
};

struct PacketT7
{
private:
	const char packetMagicBytes[11] = "TKM2PACKET";
public:
	// Type of the packet, will determine the size of the structure
	PacketT7Type_ packetType = PacketT7Type_INVALID;

	bool IsCommunicationPacket() const {
		return strcmp(packetMagicBytes, "TKM2PACKET") == 0;
	}

	bool IsValidSize(uint32_t packetSize) const;
};


struct PacketT7_Ready : PacketT7
{
	// Nothing to add here (for now, at least)

	PacketT7_Ready() {
		packetType = PacketT7Type_Ready;
	};
};

struct PacketT7_RequestMovesetSync: PacketT7
{
	struct {
		// Size of the moveset to send (0 if no moveset to send)
		uint64_t size = 0;
		// CRC32 of the moveset to send
		uint32_t crc32 = 0;
	} local_moveset;

	PacketT7_RequestMovesetSync() {
		packetType = PacketT7Type_RequestSync;
	};
};

struct PacketT7_AnswerMovesetSync: PacketT7
{
	// If true, request the opponent to send their moveset to us
	// If false, tell the opponent that we already have his moveset
	bool requesting_download = false;

	PacketT7_AnswerMovesetSync() {
		packetType = PacketT7Type_AnswerSync;
	};
};

// -- Moveset loader -- //

class MovesetLoaderT7 : public MovesetLoader
{
private:

	const TCHAR* GetSharedMemoryName() override {
		return TEXT("Local\\TKMovesets2T7Mem");
	}

	void SetAddressesGameKey() override
	{
		addresses.gameKey = "t7";
		addresses.minorGameKey = "t7";
	}

public:
	MovesetLoaderT7()
	{
		m_loadSteam = true;
	}

	// Sale ptr to the shared memory as base class but casted
	SharedMemT7* sharedMemPtr = nullptr;

	// Online opponent to send moveset to / receive moveset from. Only used in OnlinePlay mode.
	CSteamID opponentId;

	// Informations about the coming movesets are stored here
	struct {
		Byte* prev_moveset = nullptr;
		Byte* data = nullptr;
		Byte* cursor = nullptr;
		uint64_t size = 0;
		uint64_t remaining_bytes = 0;
	} incoming_moveset;

	// Informations about the syncing status is stored here
	struct {

		bool opponent_ready = false;
		bool loaded_remote_moveset = false;
		bool received_opponent_sync_request = false;
		uint64_t battle_start_call_time = 0;

		void Reset() {
			opponent_ready = false;
			loaded_remote_moveset = false;
			received_opponent_sync_request = false;
			battle_start_call_time = 0;
		}

		bool CanStart() const {
			return opponent_ready && loaded_remote_moveset;
		}
	} syncStatus;

	// Match reporting state
	struct {
		bool active = false;
		bool match_info_ready = false;
		uint32_t match_info_poll_count = 0;
		uint64_t last_heartbeat_time = 0;
		uint64_t match_start_time = 0;
		uint32_t heartbeat_count = 0;
		uint64_t opponent_steam_id = 0; // Set by MatchedAsClient/Host, resolved into p1/p2 at match start
		uint64_t p1_steam_id = 0;
		uint64_t p2_steam_id = 0;
		uint32_t p1_char_id = UINT32_MAX;
		uint32_t p2_char_id = UINT32_MAX;
		uint32_t stage_id = UINT32_MAX;
		uint32_t end_reason = MatchEndReason_Normal;

		void Start(uint64_t now) {
			active = true;
			match_info_ready = false;
			match_info_poll_count = 0;
			match_start_time = now;
			last_heartbeat_time = now;
			heartbeat_count = 0;
			end_reason = MatchEndReason_Normal;
			p1_char_id = UINT32_MAX;
			p2_char_id = UINT32_MAX;
			stage_id = UINT32_MAX;
		}

		void Stop() {
			active = false;
			match_info_ready = false;
			heartbeat_count = 0;
			opponent_steam_id = 0;
			p1_steam_id = 0;
			p2_steam_id = 0;
		}
	} matchState;

	void InitHooks() override;
	void OnInitEnd() override;
	void Mainloop() override;
	void ExecuteExtraprop() override;

	// If packets should be consumed or buffered for now
	bool CanConsumePackets();
	// Consume packets to send them to the appropriate channel
	void ConsumePackets();
	// Send a packet to the opponent set up by InitMovesetSyncing()
	bool SendPacket(void* packetBuffer, uint32_t packetSize, bool sendUnreliable=false);
	// Called whenever a packet is received
	void OnPacketReceive(CSteamID senderId, const Byte* packetBuf, uint32_t packetSize);
	// Called whenever a communication packet is received
	void OnCommunicationPacketReceive(const PacketT7* packet);
	// Called whenever a moveset is expected and a packet that isn't a communication packet is received
	void OnMovesetBytesReceive(const Byte* buf, uint32_t bufSize);
	// Inits the variable that tell that a moveset is expected (if locked in, opponent valid etc)
	void InitMovesetSyncing();
	// Send our moveset to the opponent
	void SendMoveset();
	// Discard any incoming packet until there is no more to discard
	void DiscardIncomingPackets();

	// Moveset importing function
	Byte* ImportForOnline(SharedMemT7_Player& player, Byte* moveset, uint64_t s_moveset);

	// Game-specific Moveset importing functions, returns moveset or nullptr on failure
	Byte* ImportForOnline_FromT7(const TKMovesetHeader* header, Byte* moveset, uint64_t s_moveset);
	Byte* ImportForOnline_FromTTT2(const TKMovesetHeader* header, Byte* moveset, uint64_t s_moveset);
	Byte* ImportForOnline_FromTREV(const TKMovesetHeader* header, Byte* moveset, uint64_t s_moveset);
	Byte* ImportForOnline_FromT6(const TKMovesetHeader* header, Byte* moveset, uint64_t s_moveset);

	// Utils
	CSteamID GetLobbyOpponentSteamId() const;

	void Debug() override;
};