/**
 * @file    GBMU/Application.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application's
 *          main class and associated methods.
 */

/* Private Includes ***********************************************************/

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <imgui-SFML.h>
#include <GBMU/Application.hpp>

/* Public Methods *************************************************************/

namespace gbmu
{

    Application::Application (int argc, char** argv)
    {
        // - Create Render Window
        sf::VideoMode videoMode { 
            sf::Vector2u { 1280, 720 }
        };
        m_window.create(videoMode, "GBMU Game Boy Emulator", sf::Style::Default);
        m_window.setFramerateLimit(60);

        // - Initialize Dear ImGui
        m_imguiInitialized = ImGui::SFML::Init(m_window);
    
        // - Configure ImGui
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

        installCallbacks();
    }

    Application::~Application ()
    {
        // - Shutdown Dear ImGui
        ImGui::SFML::Shutdown();
    }

    auto Application::start () -> std::int32_t
    {
        std::uint32_t tempTicks = 0;
        while (m_window.isOpen() == true)
        {
            if ((++tempTicks % 70224) == 0)
            {
                onFrame();
            }
        }

        return 0;
    }

}
