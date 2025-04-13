#include <ImGui.h>

#include "Localization.hpp"
#include "Submenu_About.hpp"
#include "imgui_extras.hpp"

void RenderSubmenu_About()
{

	{
		ImGui::SeparatorText(_("about.about"));

		ImGui::BeginGroup();
		ImGui::Indent(20.0f);
		ImGui::TextWrapped(_("about.explanation"));
		ImGui::NewLine();
		ImGui::TextWrapped(_("about.explanation_c7"));
		ImGui::EndGroup();
		ImGui::NewLine();
	}

	{
		ImGui::SeparatorText(_("about.credits"));

		ImGui::BeginGroup();
		ImGui::Indent(40.0f);

		{
			ImGui::TextColored(ImVec4(0.7f, 0.4f, 1.0f, 1), "- 6 weetbix -");
			ImGui::TextWrapped(_("about.weetbix_explanation"));
			ImGui::NewLine();
		}

		ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.8f, 1), "- klaraz -");
		ImGui::TextWrapped(_("about.klaraz_explanation"));
		ImGui::NewLine();

		ImGui::TextColored(ImVec4(0.65f, 0.9f, 0.9f, 1), "- beeno -");
		ImGui::TextWrapped(_("about.beeno_explanation"));
		ImGui::NewLine();
		ImGui::NewLine();

		ImGui::EndGroup();
	}

	// Original credits
	{
		ImGui::BeginGroup();
		ImGui::Indent(40.0f);

		{
			ImGui::TextColored(ImVec4(1, 0, 0.2f, 1), "- kilo -");
			ImGui::TextWrapped(_("about.kilo_explanation"));
			ImGui::NewLine();
		}


		ImGui::TextColored(ImVec4(0.65f, 0.9f, 0.9f, 1), "- Mr Fletch -");
		ImGui::TextWrapped(_("about.fletch_explanation"));
		ImGui::NewLine();

		ImGui::TextWrapped("- Ali -");
		ImGui::TextWrapped(_("about.ali_explanation"));
		ImGui::NewLine();

		ImGui::TextWrapped("- koenji -");
		ImGui::TextWrapped(_("about.koenji_explanation"));
		ImGui::NewLine();

		ImGui::TextWrapped("- sadamitsu -");
		ImGui::TextWrapped(_("about.sadamitsu_explanation"));
		ImGui::NewLine();

		ImGui::TextWrapped("- dennis_stanistan -");
		ImGui::TextWrapped(_("about.dennis_stanistan_explanation"));
		ImGui::NewLine();

		ImGui::TextWrapped("- dantarion -");
		ImGui::TextWrapped(_("about.dantarion_explanation"));
		ImGui::NewLine();

		ImGui::EndGroup();
	}
}