/**
 * @file    GBMU/AppMenuBar.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application
 *          class's menu bar methods.
 */

/* Private Includes ***********************************************************/

#include <imgui.h>
#include <imgui_internal.h>
#include <GBMU/Application.hpp>

/* Public Methods - ImGui Menu Bar ********************************************/

namespace gbmu
{

    auto Application::showMenuBar () -> void
    {
        if (ImGui::BeginMainMenuBar())
        {
            showFileMenu();
            showEmulationMenu();
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
            {
                showOpenCartridgeDialog();
            }

            if (ImGui::MenuItem("Detach Cartridge", "Ctrl+D", nullptr,
                m_cart != nullptr))
            {
                unloadCartridge();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                m_window.close();
            }

            ImGui::EndMenu();
        }
    }

    auto Application::showEmulationMenu () -> void
    {
        if (ImGui::BeginMenu("Emulation"))
        {
            ImGui::MenuItem("Blargg Mode", nullptr, &m_blarggMode);
            ImGui::EndMenu();
        }
    }

    auto Application::showViewMenu () -> void
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Console Window", nullptr, &m_showConsoleWindow);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo Window", nullptr, &m_showDemoWindow);
            ImGui::EndMenu();
        }
    }

    auto Application::showHelpMenu () -> void
    {
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::EndMenu();
        }
    }

}