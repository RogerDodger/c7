#include "GameSharedMem.hpp"

void GameSharedMem::OnProcessAttach()
{
	InstantiateFactory();
	m_plannedImportations.clear();
}

void GameSharedMem::OnProcessDetach()
{
	isMemoryLoaded = false;
	isInjecting = false;
	m_requestedInjection = false;
	lockedIn = false;
	wasDetached = true;
	displayedMovesets.clear();

	// Was in GameSharedMem::InstantiateFactory(), see comment there
	m_toFree_importer = m_importer;
	m_toFree_sharedMemHandler = m_sharedMemHandler;
}

void GameSharedMem::InstantiateFactory()
{
	// Commented code crashes Tekken 7 for some reason
	//
	// Specifically, if the DLL was injected into Tekken 7, and then Tekken 7 is
	// closed and re-opened, Tekken 7 will crash if this is uncommented.
	// It will not crash on the next load.
	//
	// // Delete old instances if needed
	// m_toFree_importer = m_importer;
	// m_toFree_sharedMemHandler = m_sharedMemHandler;
	//
	// It doesn't matter if we do it right away either, this has same effect:
	//
	// delete m_importer;
	// delete m_sharedMemHandler;
	//
	// Without this, there's no crash, but the DLL can't re-inject either
	// Solution appears to be doing this cleanup in OnProcessDetach() instead
	// Not clear why this wasn't how it worked already
	game.SetCurrentGame(currentGame);

	// Every game has its own subtleties so we use polymorphism to manage that
	m_importer = currentGame->importer == nullptr ? nullptr : Games::FactoryGetImporter(currentGame, process, game);
	m_sharedMemHandler = Games::FactoryGetOnline(currentGame, process, game);
}

void GameSharedMem::RunningUpdate()
{
	// Atempt to load the shared memory if needed
	if (!m_sharedMemHandler->IsMemoryLoaded()) {
		// Update cached variable to avoid external code having to access .sharedMemHandler 
		isMemoryLoaded = m_sharedMemHandler->LoadSharedMemory();
		if (isMemoryLoaded) {
			isInjecting = false;
			m_requestedInjection = false;
			versionMismatch = m_sharedMemHandler->versionMismatch;
		}
		else if (isInjecting) {
			// Memory loaded, check if we were requested to inject the DLL in the past
			if (!m_requestedInjection) {
				// Order the DLL to be injected
				m_sharedMemHandler->InjectDll();
				m_requestedInjection = true;
			}
			// DLL was already ordered to be injected in the past
			else if (!m_sharedMemHandler->isInjecting) {
				// Injection was done, however the memory isn't loaded.
				DEBUG_LOG("\n --  GameSharedMem::RunningUpdate() -- ERROR: DLL Injection finished, but memory not loaded. Did the DLL fail to initialize the memory?\n");
				isInjecting = false;
				m_requestedInjection = false;
			}
		}
	}
	else {
		if (synchronizeLockin) {
			m_sharedMemHandler->SetLockIn(lockedIn, movesetLoaderMode);
		}
		if (m_sharedMemHandler->versionMismatch) {
			m_sharedMemHandler->VerifyDllVersionMismatch();
		}
	}

	CheckNameTag();

	while (m_sharedMemHandler->IsMemoryLoaded() && IsBusy())
	{
		auto& [moveset, settings, playerId] = m_plannedImportations[0];

		// The only case where .size can be 0 is if a Submenu purposefully set it that way, to indicate that we want to clear the selection
		if (moveset.size == 0) {
			m_sharedMemHandler->ClearMovesetSelection(playerId);
			m_plannedImportations.erase(m_plannedImportations.begin());
		}
		else
		{
			ImportationErrcode_ errcode;
			errcode = m_importer->Import(moveset.filename.c_str(), 0, settings, progress);

			// Send the successfully loaded moveset to the shared memory manager
			if (errcode == ImportationErrcode_Successful) {
				auto& lastLoaded = m_importer->lastLoaded;
				m_sharedMemHandler->OnMovesetImport(&moveset, playerId, lastLoaded);
				m_plannedImportations.erase(m_plannedImportations.begin());
			} else {
				m_errors.push_back(errcode);
				m_plannedImportations.clear();
			}
		}

		// Make a copy of the displayed movesets to avoid the GUI having to iterate on a vector that might be destroyed at any time
		displayedMovesets = *m_sharedMemHandler->displayedMovesets;


		/*
		* TODO: think about when to free online-used movesets
		* Should check both player offsets and shared memory
		* Importer should return a list of unused movesets and then shared mem handler should be checked too before proceeding
		if (settings & ImportSettings_FreeUnusedMovesets) {
			m_importer->CleanupUnusedMovesets();
		}
		*/
	}
}

void GameSharedMem::QueueCharacterImportation(movesetInfo* moveset, unsigned int playerId, ImportSettings settings)
{
	m_plannedImportations.push_back({
		.moveset = *moveset,
		.settings = settings,
		.playerId = playerId
	});
}

bool GameSharedMem::IsBusy() const
{
	return m_plannedImportations.size() > 0;
}

void GameSharedMem::InjectDll()
{
	isInjecting = true;
}

void GameSharedMem::StopThreadAndCleanup()
{
	ClearNameTag();

	// Order thread to stop
	m_threadStarted = false;
	m_t.join();

	if (m_sharedMemHandler != nullptr) {
		if (process.IsAttached() && m_importer != nullptr) {
			m_importer->CleanupUnusedMovesets();
		}
		delete m_sharedMemHandler;
		delete m_importer;
	}
}

void GameSharedMem::FreeExpiredFactoryClasses()
{
	if (m_toFree_importer) {
		delete m_toFree_importer;
		delete m_toFree_sharedMemHandler;

		if (m_toFree_importer == m_importer) {
			m_importer = nullptr;
		}
		if (m_toFree_sharedMemHandler == m_sharedMemHandler) {
			m_sharedMemHandler = nullptr;
		}

		m_toFree_importer = nullptr;
		m_toFree_sharedMemHandler = nullptr;
	}
}

void GameSharedMem::ResetTargetProcess()
{
	if (process.IsAttached())
	{
		PreProcessDetach();
		process.Detach();
		OnProcessDetach();
	}

	currentGame = nullptr;

	if (m_importer != nullptr) {
		m_toFree_importer = m_importer;
	}

	if (m_sharedMemHandler != nullptr) {
		m_toFree_sharedMemHandler = m_sharedMemHandler;
	}
}

void GameSharedMem::CheckNameTag()
{
	char username[32] = { 0 };
	gameAddr username_addr = game.ReadPtrPath("username_addr");
	process.readBytes(username_addr, &username, sizeof(username));

	if (*username == 0) {
		return;
	}

	if (std::string_view(username, sizeof(username)).find(PROGRAM_NAMETAG) == std::string::npos) {
		size_t offset = sizeof(PROGRAM_NAMETAG) - 1;
		memmove(username + offset, username, sizeof(username) - offset);
		memcpy(username, PROGRAM_NAMETAG, offset);
		DEBUG_LOG("GameSharedMem::CheckNameTag() - changed to %s\n", username);
		process.writeBytes(username_addr, username, sizeof(username));

		DEBUG_LOG("GameSharedMem::CheckNameTag() - injecting DLL\n");
		m_sharedMemHandler->InjectDllAndWaitEnd();
		SetSharedMemDestroyBehaviour(true);
	}
}

void GameSharedMem::ClearNameTag()
{
	char username[32] = { 0 };
	gameAddr username_addr = game.ReadPtrPath("username_addr");
	process.readBytes(username_addr, &username, sizeof(username));

	if (*username == 0) {
		return;
	}

	if (std::string_view(username, sizeof(username)).find(PROGRAM_NAMETAG) == 0) {
		DEBUG_LOG("GameSharedMem::ClearNameTag() - removing name tag from %s\n", username);
		size_t offset = sizeof(PROGRAM_NAMETAG) - 1;
		memmove(username, username + offset, sizeof(username) - offset);
		memset(username + sizeof(username) - offset, 0, offset);
		DEBUG_LOG("GameSharedMem::CheckNameTag() - changed back to %s\n", username);
		process.writeBytes(username_addr, username, sizeof(username));
	}
}