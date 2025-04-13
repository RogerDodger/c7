#include "Online_t7.hpp"

OnlineT7::~OnlineT7()
{
    if (resetMemOnDestroy && m_sharedMemPtr && m_process.CheckRunning()) {
        m_sharedMemPtr->locked_in = false;
    }
}

void OnlineT7::Init()
{
    m_sharedMemPtr = (decltype(m_sharedMemPtr))m_orig_sharedMemPtr;
    if (sizeof(decltype(m_sharedMemPtr)) > SHARED_MEMORY_BUFSIZE) {
        DEBUG_LOG("! Size too big or buffer too small for derived Online::m_sharedMemPtr structure type !\n");
        throw;
    }
}

void OnlineT7::OnMovesetImport(movesetInfo* displayedMoveset, unsigned int playerid, const s_lastLoaded& lastLoadedMoveset)
{
    int32_t charId = lastLoadedMoveset.charId;
    DEBUG_LOG("OnMovesetImport for char_id %d\n", charId);
    if (charId >= 0 && charId < sizeof(m_sharedMemPtr->chars)) {
        m_sharedMemPtr->chars[charId].addr = lastLoadedMoveset.address;
        m_sharedMemPtr->chars[charId].size = lastLoadedMoveset.size;
        m_sharedMemPtr->chars[charId].is_initialized = false;
    }
}

void OnlineT7::ClearMovesetSelection(unsigned int playerid)
{
    movesetInfo emptyMoveset{ .size = 0 };

    while (displayedMovesets->size() <= playerid) {
        displayedMovesets->push_back(emptyMoveset);
    }

    (*displayedMovesets)[playerid] = emptyMoveset;
    m_sharedMemPtr->players[playerid].custom_moveset_addr = 0;
}

void OnlineT7::SetLockIn(bool locked, MovesetLoaderMode_ movesetLoaderMode)
{
    m_sharedMemPtr->locked_in = locked;
    m_sharedMemPtr->moveset_loader_mode = movesetLoaderMode;
}

void OnlineT7::ExecuteExtraprop(uint32_t playerid, uint32_t id, uint32_t value)
{
    if (m_sharedMemPtr != nullptr) {
        m_sharedMemPtr->propToPlay = {
            .playerid = playerid,
            .id = id,
            .value = value
        };
        CallMovesetLoaderFunction("ExecuteExtraprop", true);
    }
    else {
        DEBUG_LOG("ExecuteExtraprop(): m_sharedMemPtr is NULL\n");
    }
}