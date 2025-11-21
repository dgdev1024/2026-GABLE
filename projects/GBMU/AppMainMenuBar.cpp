/**
 * @file    GBMU/AppMainMenuBar.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application's
 *          ImGui main menu bar methods.
 */

/* Private Includes ***********************************************************/

#include <imgui.h>
#include <GBMU/Application.hpp>

/* Private Methods - ImGui Main Menu Bar **************************************/

namespace gbmu
{

    auto Application::showMainMenuBar () -> void
    {
        if (ImGui::BeginMainMenuBar())
        {
            showFileMenu();
            showViewMenu();
            showHelpMenu();
            ImGui::EndMainMenuBar();
        }
    }

    auto Application::showFileMenu () -> void
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Cartridge...", "Ctrl+O"))
                { showOpenCartridgeDialog(); }
            if (ImGui::MenuItem("Close Cartridge", "Ctrl+W", nullptr, m_cartridge != nullptr))
                { unloadCartridge(); }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4"))
                { onClose(); }

            ImGui::EndMenu();
        }
    }

    auto Application::showViewMenu () -> void
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("ImGui Demo Window", nullptr, &m_showDemoWindow);
            ImGui::EndMenu();
        }
    }

    auto Application::showHelpMenu () -> void
    {
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About GBMU"))
            {
                showAboutDialog();
            }

            ImGui::EndMenu();
        }
    }
}
