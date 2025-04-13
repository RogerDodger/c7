#pragma once

#include "GameSharedMem.hpp"

class Submenu_Status {
private:
	bool m_tagApplied = false;

public:
	// Helper that contains both an importer and a shared memory manager
	GameSharedMem* gameHelper;

	void Render();
	// Renders the movelist
	void RenderMovesetList(bool canSelectMoveset);
};