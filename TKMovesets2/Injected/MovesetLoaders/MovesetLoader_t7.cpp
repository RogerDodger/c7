#include <stdarg.h>

#include "MovesetLoader_t7.hpp"
#include "Extractor.hpp"
#include "Helpers.hpp"
#include "steamHelper.hpp"

#include "steam_api.h"

using namespace StructsT7;

// Reference to the MovesetLoader
MovesetLoaderT7* g_loader = nullptr;

# define GET_HOOK(x) ((uint64_t)&T7Hooks::x)

// -- Game functions -- //

namespace T7Functions
{
	// Called after every loading screen on each player address to write the moveset to their offsets
	typedef uint64_t(*ApplyNewMoveset)(void* player, MovesetInfo* newMoveset);

	// Returns the address of a player from his playerid (this is character-related stuff)
	typedef uint64_t(*GetPlayerFromID)(unsigned int playerId);

	typedef void(*ExecuteExtraprop)(void* player, Requirement* prop, __ida_int128 a3, char a4, char a5, float a6, __ida_int128 a7, __ida_int128 a8, __ida_int128 a9, __ida_int128 a10, __ida_int128 a11, uint64_t a12);

	namespace CBattleManagerConsole {
		// Called everytime a loading screen starts
		typedef uint64_t(*LoadStart)(uint64_t, uint64_t);
	};

	namespace NetManager {
		typedef uint64_t(*GetSyncBattleStart)(uint64_t);
	};

	namespace NetInterCS {
		// Called at the start of the loading screen for non-host players
		typedef void (*MatchedAsClient)(uint64_t, uint64_t);
		// Called at the start of the loading screen for the host player
		typedef void (*MatchedAsHost)(uint64_t, char, uint64_t);
	};

	// Called when player confirms "Random" in stage select. Returns selected stage ID.
	typedef uint8_t (*UIRandomStage)(uint64_t stageManager, uint8_t playerIndex);
}

// -- Helpers --
// Try to cache variable addresses when possible instead of calling the helpers too much

static uint64_t* GetPlayerList()
{
	uint64_t baseFuncAddr = g_loader->GetFunctionAddr("TK__GetPlayerFromID");

	// Base ourselves on the instructions (lea rcx, ...) to find the address since values are relative form the function start
	uint32_t offset = *(uint32_t*)(baseFuncAddr + 11);
	uint64_t offsetBase = baseFuncAddr + 15;

	return (uint64_t*)(offsetBase + offset);
}

static unsigned int IsLocalPlayerP1()
{
	auto addr = g_loader->addresses.ReadPtrPathInCurrProcess("pid_addr", g_loader->moduleAddr);
	if (addr != 0) {
		return *(unsigned int*)addr == 0;
	}
	DEBUG_LOG("Failed to find p1\n");
	return true;
}

static int GetPlayerIdFromAddress(void* playerAddr)
{
	auto playerList = (uint64_t*)g_loader->variables["gTK_playerList"];
	for (int i = 0; i < 2; ++i) // The list technically goes to 6 but this game doesn't handle it so who cares
	{
		if (playerList[i] == (uint64_t)playerAddr) {
			return i;
		}
	}
	return -1;
}

static void ApplyStaticMotas(void* player, MovesetInfo* newMoveset)
{
	// This is required for some MOTAs to be properly applied (such as camera, maybe more)
	const uint64_t staticCameraOffset = g_loader->addresses.GetValue("static_camera_offset");
	MotaHeader** staticMotas = (MotaHeader**)((char*)player + staticCameraOffset);

	staticMotas[0] = newMoveset->motas.camera_1;
	staticMotas[1] = newMoveset->motas.camera_2;
}

// -- Other helpers -- //

static void InitializeCustomMoveset(SharedMemT7_Char& charData)
{
	MovesetInfo* moveset = (MovesetInfo*)charData.addr;

	// Mark missing motas with bitflag
	for (unsigned int i = 0; i < _countof(moveset->motas.motas); ++i)
	{
		if ((uint64_t)moveset->motas.motas[i] == MOVESET_ADDR_MISSING) {
			charData.SetMotaMissing(i);
		}
	}

	charData.is_initialized = true;
}

static unsigned int GetLobbySelfMemberId()
{
	CSteamID lobbyID = CSteamID(g_loader->ReadVariable<uint64_t>("gTK_roomId_addr"));
	CSteamID mySteamID = SteamHelper::SteamUser()->GetSteamID();

	unsigned int lobbyMemberCount = SteamHelper::SteamMatchmaking()->GetNumLobbyMembers(lobbyID);
	for (unsigned int i = 0; i < lobbyMemberCount; ++i)
	{
		CSteamID memberId = SteamHelper::SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, i);
		if (memberId == mySteamID) {
			return i;
		}
	}
	return 0;
}

static unsigned int GetLobbyOpponentMemberId()
{
	// Note that this function is highly likely to return the wrong player ID in lobbies with more than two players
	// This is because the player order here does not match the displayed player order
	CSteamID lobbyID = CSteamID(g_loader->ReadVariable<uint64_t>("gTK_roomId_addr"));
	CSteamID mySteamID = SteamHelper::SteamUser()->GetSteamID();

	unsigned int lobbyMemberCount = SteamHelper::SteamMatchmaking()->GetNumLobbyMembers(lobbyID);
	for (unsigned int i = 0; i < lobbyMemberCount; ++i)
	{
		CSteamID memberId = SteamHelper::SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, i);
		if (memberId != mySteamID) {
			return i;
		}
	}
	return 0;
}

// -- Hook functions --

namespace T7Hooks
{
	uint64_t ApplyNewMoveset(void* player, MovesetInfo* newMoveset)
	{
		auto player_id = GetPlayerIdFromAddress(player);
		DEBUG_LOG("\n- ApplyNewMoveset on player %llx, moveset is %llx, idx %d -\n", (uint64_t)player, (uint64_t)newMoveset, player_id);

		// First, call the original function to let the game initialize all the important values in their new moveset
		auto retVal = g_loader->CastTrampoline<T7Functions::ApplyNewMoveset>("TK__ApplyNewMoveset")(player, newMoveset);
		DEBUG_LOG("(orig moveset initialized, retVal %d)\n", retVal);

		// If this isn't P1 or P2, stop here
		// This function is called on 5 players on each load
		//
		// 1st call is for P1, player_id 0
		// 2nd call is for P2, player_id 1
		// 3rd call has player_id -1 and char_id 75 (Mokujin)
		// 4th call has player_id 4
		// 5th call has player_id 5
		//
		// For the 4th and 5th call, newMoveset is from P1 and P2's movesets in the previous game respectively
		// These calls, because they come later, would mess up the "missing" motas on the custom moveset
		// For example, if Miguel was on P2 last game, and now Kazumi is on P2
		// Kazumi would end up with Miguel's face and anim motas if we continued
		// Since these are phantom players anyway, we just return here to avoid this bug
		//
		// Ali speculates that the player_id 4 and 5 may be a holdover from Tag 2
		// where each game has 4 loaded characters
		if (player_id != 0 && player_id != 1) {
			return retVal;
		}

		// Obtain custom moveset by this player's char_id
		uint32_t char_id = *((char*)player + g_loader->addresses.GetValue("chara_id_offset"));
		if (char_id >= sizeof(g_loader->sharedMemPtr->chars)) {
			return retVal;
		}

		auto& charData = g_loader->sharedMemPtr->chars[char_id];
		MovesetInfo* customMoveset = (MovesetInfo*)charData.addr;
		if (!customMoveset) {
			DEBUG_LOG("No custom moveset for char_id %d\n", char_id);
			return retVal;
		}

		if (!charData.is_initialized) {
			InitializeCustomMoveset(charData);
		}

		// Copy missing MOTA offsets from newMoveset
		for (unsigned int i = 0; i < _countof(customMoveset->motas.motas); ++i)
		{
			if (charData.isMotaMissing(i)) {
				DEBUG_LOG("Missing mota %d, getting it from game's moveset\n", i);
				customMoveset->motas.motas[i] = newMoveset->motas.motas[i];
			}
		}

		{
			// Apply custom moveset to our character*
			DEBUG_LOG("Applying custom moveset to character...\n");
			auto addr = (char*)player + g_loader->addresses.GetValue("motbin_offset");
			auto motbinOffsetList = (MovesetInfo**)(addr);

			motbinOffsetList[0] = customMoveset;
			motbinOffsetList[1] = customMoveset;
			motbinOffsetList[2] = customMoveset;
			motbinOffsetList[3] = customMoveset;
			motbinOffsetList[4] = customMoveset;

			ApplyStaticMotas(player, customMoveset);
		}

		return retVal;
	}

	// Log function 
	void Log(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		vfprintf(stdout, format, argptr);
		va_end(argptr);
	}


	void ExecuteExtraprop(void* player, Requirement* prop, __ida_int128 a3, char a4, char a5, float a6, __ida_int128 a7, __ida_int128 a8, __ida_int128 a9, __ida_int128 a10, __ida_int128 a11, uint64_t a12)
	{

		if ((uint64_t)player == GetPlayerList()[0]) {

			DEBUG_LOG("TK__ExecuteExtraprop - PROP[%x|%u], %p, ", prop->condition, prop->param_unsigned, player);
			DEBUG_LOG("%u, %u, %.3f, ", a4, a5, a6);
			print_int128(a7);
			print_int128(a8);
			print_int128(a9);
			print_int128(a10);
			print_int128(a11);
			DEBUG_LOG("%llx\n", a12);
		}
		g_loader->CastTrampoline<T7Functions::ExecuteExtraprop>("TK__ExecuteExtraprop")(player, prop, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}


	namespace NetInterCS
	{
		void MatchedAsClient(uint64_t a1, uint64_t a2)
		{
			DEBUG_LOG("-- NetInterCS::MatchedAsHost --\n");
			g_loader->CastTrampoline<T7Functions::NetInterCS::MatchedAsClient>("TK__NetInterCS::MatchedAsClient")(a1, a2);

			if (g_loader->sharedMemPtr->moveset_loader_mode == MovesetLoaderMode_OnlineMode) {
				g_loader->InitMovesetSyncing();
			}
		}

		void MatchedAsHost(uint64_t a1, char a2, uint64_t a3)
		{
			DEBUG_LOG("-- NetInterCS::MatchedAsHost --\n");
			g_loader->CastTrampoline<T7Functions::NetInterCS::MatchedAsHost>("TK__NetInterCS::MatchedAsHost")(a1, a2, a3);

			if (g_loader->sharedMemPtr->moveset_loader_mode == MovesetLoaderMode_OnlineMode) {
				g_loader->InitMovesetSyncing();
			}
		}
	};


	namespace NetManager
	{
		uint64_t GetSyncBattleStart(uint64_t a1)
		{
			uint64_t result = g_loader->CastTrampoline<T7Functions::NetManager::GetSyncBattleStart>("TK__NetManager::GetSyncBattleStart")(a1);
			DEBUG_LOG("-- GetSyncBattleStart = 0x%llx --\n", result);
			if (result != 1 || !g_loader->sharedMemPtr->IsAttemptingOnlinePlay()) {
				return result;
			}

			// Loading is finished no need to hold this up any longer
			if (g_loader->sharedMemPtr->moveset_sync_status == MovesetSyncStatus_Ready) {
				return 1;
			}

			if (g_loader->syncStatus.battle_start_call_time == 0) {
				// Get timestamp of first time 1 was returned
				g_loader->syncStatus.battle_start_call_time = Helpers::getCurrentTimestamp();
			}
			else {
				// Wait up to X seconds before finally giving up on loading
				uint64_t diff = Helpers::getCurrentTimestamp() - g_loader->syncStatus.battle_start_call_time;

				unsigned int maxSyncedWaitTime = 60; // Wait 60 seconds max if moveset sync has started
				unsigned int maxUnsyncedWaitTime = 15; // Wait 15 seconds max if moveset sync has not started

				// Different wait time, smaller if opponent has not locked-in
				const unsigned int maxWaitTime = g_loader->syncStatus.received_opponent_sync_request ? maxSyncedWaitTime : maxUnsyncedWaitTime;

				DEBUG_LOG("GetSyncBattleStart: %llu / %u\n", diff + 1, maxWaitTime);
				if (diff >= maxWaitTime) {
					g_loader->sharedMemPtr->moveset_sync_status = MovesetSyncStatus_NotStarted;
					DEBUG_LOG("GetSyncBattleStart: Moveset still wasn't received after %u seconds\n", maxWaitTime);
					return 1;
				}
			}
			return (uint32_t)0;
		}
	};

	// UI Random stage selection hook - called when player confirms "Random" in stage select
	uint8_t UIRandomStage(uint64_t stageManager, uint8_t playerIndex)
	{
		// Stages to exclude from random selection
		static const uint8_t excludedStages[] = {
			1,   // Forgotten Realm
			3,   // Arctic Snowfall
			40,  // Infinite Azure
			55   // Infinite Azure 2
		};

		auto isExcluded = [](uint8_t stage) {
			for (int i = 0; i < sizeof(excludedStages) / sizeof(excludedStages[0]); i++) {
				if (stage == excludedStages[i]) return true;
			}
			return false;
		};

		// Keep rerolling until we get a non-excluded stage
		uint8_t selectedStage;
		int attempts = 0;
		do {
			selectedStage = g_loader->CastTrampoline<T7Functions::UIRandomStage>
				("TK__UIRandomStage")(stageManager, playerIndex);
			attempts++;
		} while (isExcluded(selectedStage) && attempts < 100);

		DEBUG_LOG("UIRandomStage: player=%u, selected=%u (attempts=%d)\n", playerIndex, selectedStage, attempts);

		return selectedStage;
	}
}

// -- -- //

void ExecuteInstantExtraprop(int playerid, uint32_t propId, uint32_t propValue)
{
	if (!g_loader->HasFunction("TK__ExecuteExtraprop")) {
		return;
	}

	DEBUG_LOG("ExecuteInstantExtraprop(%d, %x, %u)\n", playerid, propId, propValue);

	void* player = (void*)(GetPlayerList()[playerid]);
	Requirement prop = { .condition = propId, .param_unsigned = propValue };

	__ida_int128 a3 = { 0 };
	char a4;
	char a5;
	float a6;
	__ida_int128 a7 = { 0 };
	__ida_int128 a8 = { 0 };
	__ida_int128 a9 = { 0 };
	__ida_int128 a10 = { 0 };
	__ida_int128 a11 = { 0 };
	uint64_t a12 = { 0 };

	SET_INT128(a3, 0, 0x4000000000000000);
	a4 = 0;
	a5 = 0;
	a6 = 0;
	SET_INT128(a7, 0, 0x4000000000000000);
	SET_INT128(a8, 0, 0x4000000000000000);
	SET_INT128(a9, 1, 0x4000000000000000);
	SET_INT128(a10, 1, 0x4000000000000000);
	SET_INT128(a11, 0, 0x4000000000000000);
	a12 = 0;


	g_loader->CastFunction<T7Functions::ExecuteExtraprop>("TK__ExecuteExtraprop")(player, &prop, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

// -- Hooking init --

void MovesetLoaderT7::InitHooks()
{
	// Set the global reference to us
	g_loader = this;

	// Declare which hooks must absolutely be found to initialize this DLL
	m_requiredSymbols = {
		// Hooks
		"TK__ApplyNewMoveset",
		"TK__NetInterCS::MatchedAsClient",
		"TK__NetInterCS::MatchedAsHost",
		"TK__NetManager::GetSyncBattleStart",
		// Functions
		"TK__GetPlayerFromID",
	};

	// Crucial functions / hooks
	RegisterHook("TK__ApplyNewMoveset", m_moduleName, "f_ApplyNewMoveset", GET_HOOK(ApplyNewMoveset));
	RegisterHook("TK__NetInterCS::MatchedAsClient", m_moduleName, "f_NetInterCS::MatchedAsClient", GET_HOOK(NetInterCS::MatchedAsClient));
	RegisterHook("TK__NetInterCS::MatchedAsHost", m_moduleName, "f_NetInterCS::MatchedAsHost", GET_HOOK(NetInterCS::MatchedAsHost));
	RegisterHook("TK__NetManager::GetSyncBattleStart", m_moduleName, "f_NetManager::GetSyncBattleStart", GET_HOOK(NetManager::GetSyncBattleStart));
	RegisterFunction("TK__GetPlayerFromID", m_moduleName, "f_GetPlayerFromID");

	// UI Random stage selection hook - called when player confirms "Random" in stage select
	RegisterHook("TK__UIRandomStage", m_moduleName, "f_UIRandomStage", GET_HOOK(UIRandomStage));

	// Other less important things
	RegisterFunction("TK__ExecuteExtraprop", m_moduleName, "f_ExecuteExtraprop");

	variables["gTK_roomId_addr"] = addresses.ReadPtrPathInCurrProcess("steamLobbyId_addr", moduleAddr);

	{
#ifdef BUILD_TYPE_DEBUG
		// Also hook extraprop to debug print it
		RegisterHook("TK__ExecuteExtraprop", m_moduleName, "f_ExecuteExtraprop", (uint64_t)&T7Hooks::ExecuteExtraprop);

		// Find TK__Log
		auto funcAddr = addresses.ReadPtrPathInCurrProcess("f_Log_addr", moduleAddr);
		if (Helpers::compare_memory_string((void*)funcAddr, addresses.GetString("f_Log")))
		{
			InitHook("TK__Log", funcAddr, (uint64_t)&T7Hooks::Log);
			DEBUG_LOG("Found Log function\n");
		}
#endif
	}
}
void MovesetLoaderT7::OnInitEnd()
{
	sharedMemPtr = (SharedMemT7*)orig_sharedMemPtr;

	// Compute important variable addresses
	variables["gTK_playerList"] = (uint64_t)GetPlayerList();

	// Structure that leaders to playerid (???, 0, 0, 0x78). ??? = the value we get at the end
	/*
	{
		uint64_t functionAddress = m_hooks["TK__NetInterCS::MatchedAsClient"].originalAddress;
		uint64_t postInstructionAddress = functionAddress + 0x184;
		uint64_t offsetAddr = functionAddress + 0x180;

		uint32_t offset = *(uint32_t*)offsetAddr;
		uint64_t endValue = postInstructionAddress + offset;
		variables["______"] = endValue;
	}
	*/

	// Apply the hooks that need to be applied immediately
	HookFunction("TK__ApplyNewMoveset");
	HookFunction("TK__NetInterCS::MatchedAsClient");
	HookFunction("TK__NetInterCS::MatchedAsHost");
	HookFunction("TK__NetManager::GetSyncBattleStart");
	HookFunction("TK__UIRandomStage");

	//HookFunction("TK__ExecuteExtraprop");

	// Other
	//HookFunction("TK__Log");
}

// -- Main -- //

void MovesetLoaderT7::Mainloop()
{
	while (!mustStop)
	{
		if (CanConsumePackets()) {
			ConsumePackets();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(GAME_INTERACTION_THREAD_SLEEP_MS));
	}
}

// -- Interaction -- //

void MovesetLoaderT7::ExecuteExtraprop()
{
	auto& propData = sharedMemPtr->propToPlay;
	ExecuteInstantExtraprop(propData.playerid, propData.id, propData.value);
}

// -- Debug -- //

void MovesetLoaderT7::Debug()
{
#ifdef BUILD_TYPE_DEBUG
	CSteamID lobbyID = CSteamID(ReadVariable<uint64_t>("gTK_roomId_addr"));
	CSteamID mySteamID = SteamHelper::SteamUser()->GetSteamID();

	if (lobbyID == k_steamIDNil) {
		DEBUG_LOG("Invalid lobby ID\n");
	}

	unsigned int lobbyMemberCount = SteamHelper::SteamMatchmaking()->GetNumLobbyMembers(lobbyID);
	for (unsigned int i = 0; i < lobbyMemberCount; ++i)
	{
		CSteamID memberId = SteamHelper::SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, i);
		const char* name = SteamHelper::SteamFriends()->GetFriendPersonaName(memberId);
		DEBUG_LOG("%u. [%s] - self = %d\n", i, name, memberId == mySteamID);
	}
#endif
}