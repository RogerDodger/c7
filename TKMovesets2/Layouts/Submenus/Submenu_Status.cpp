#include <ImGui.h>

#include "Submenu_Status.hpp"
#include "Localization.hpp"
#include "imgui_extras.hpp"
#include "Games.hpp"
#include "GameSharedMem.hpp"

void Submenu_Status::RenderMovesetList(bool canSelectMoveset)
{
	auto availableSpace = ImGui::GetContentRegionAvail();
	availableSpace.y -= ImGui::GetFrameHeightWithSpacing();

	ImGui::SeparatorText(_("status.movesets"));
	if (ImGui::BeginTable("MovesetImportationList", 5, ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY
		| ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, availableSpace))
	{
		ImGui::TableSetupColumn("##", 0, 5.0f);
		ImGui::TableSetupColumn(_("moveset.origin"));
		ImGui::TableSetupColumn(_("moveset.target_character"));
		ImGui::TableSetupColumn(_("moveset.size"));
		ImGui::TableSetupColumn(_("persistent.hash"));
		ImGui::TableHeadersRow();

		ImGui::PushID(&gameHelper);

		// drawList & windowPos are used to display a different row bg
		ImDrawList* drawlist = ImGui::GetWindowDrawList();
		auto windowPos = ImGui::GetWindowPos();

		// Yes, we don't use an iterator here because the vector might actually change size mid-iteration
		for (size_t i = 0; i < gameHelper->storage->extractedMovesets.size(); ++i)
		{
			// Moveset is guaranteed not to be freed until after this loop
			movesetInfo* moveset = gameHelper->storage->extractedMovesets[i];

			if (moveset->origin == "INVALID") {
				continue;
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			bool isImportable = moveset->onlineImportable && gameHelper->currentGame->SupportsGameImport(moveset->gameId);
			int color = !isImportable ? MOVESET_INVALID : moveset->color;
			if (moveset->color != 0)
			{
				// Draw BG
				ImVec2 drawStart = windowPos + ImGui::GetCursorPos();
				drawStart.y -= ImGui::GetScrollY();
				ImVec2 drawArea = ImVec2(availableSpace.x, ImGui::GetFrameHeight());
				drawlist->AddRectFilled(drawStart, drawStart + drawArea, moveset->color);
			}

			ImGui::TextUnformatted(moveset->name.c_str());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(moveset->origin.c_str());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(moveset->target_character.c_str());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(moveset->sizeStr.c_str());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(moveset->hash.c_str());

			ImGui::TableNextColumn();
			ImGui::PushID(moveset->filename.c_str());

			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::EndTable();
	}
}

void Submenu_Status::Render()
{
	bool sharedMemoryLoaded = gameHelper->isMemoryLoaded;
	bool versionMatches = !gameHelper->versionMismatch;
	bool isBusy = gameHelper->IsBusy();
	bool isAttached = gameHelper->IsAttached();

	switch (gameHelper->process.status)
	{
	case GameProcessErrcode_PROC_ATTACHED:
		if (!versionMatches) {
			ImGuiExtra_TextboxWarning(_("process.dll_version_mismatch"));
		}
		break;
	case GameProcessErrcode_PROC_NOT_ATTACHED:
	case GameProcessErrcode_PROC_EXITED:
	case GameProcessErrcode_PROC_ATTACHING:
		ImGuiExtra_TextboxWarning(_("process.game_not_attached"));
		break;
	case GameProcessErrcode_PROC_NOT_FOUND:
		ImGuiExtra_TextboxWarning(_("process.game_not_running"));
		break;
	case GameProcessErrcode_PROC_VERSION_MISMATCH:
		ImGuiExtra_TextboxError(_("process.game_version_mismatch"));
		break;
	case GameProcessErrcode_PROC_ATTACH_ERR:
		ImGuiExtra_TextboxError(_("process.game_attach_err"));
		break;
	}

	// Import progress text.
	if (0 < gameHelper->progress && gameHelper->progress < 100) {
		if (isBusy) {
			ImGui::Text(_("importation.progress"), gameHelper->progress);
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0, 0, 1), _("importation.progress_error"), gameHelper->progress);
		}
	}

	if (isAttached && sharedMemoryLoaded && versionMatches && !isBusy) {
		ImGuiExtra_TextboxTip(_("status.loaded"));
	}

	//ImGuiExtra::RenderTextbox(_("status.tag_applied"));
	//ImGui::SeparatorText(_("select_game"));

	//auto availableSpace = ImGui::GetContentRegionAvail();

	RenderMovesetList(versionMatches);
}