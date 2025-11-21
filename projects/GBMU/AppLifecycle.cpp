/**
 * @file    GBMU/AppLifecycle.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application's
 *          lifecycle methods.
 */

/* Private Includes ***********************************************************/

#include <imgui.h>
#include <imgui-SFML.h>
#include <GBMU/Application.hpp>

/* Private Methods - Application Lifecycle ************************************/

namespace gbmu
{

    auto Application::onEvent (const std::optional<sf::Event>& event) -> void
    {
        ImGui::SFML::ProcessEvent(m_window, *event);

        if (event->is<sf::Event::Closed>())
        {
            onClose();
        }
    }

    auto Application::onUpdate (const sf::Time& deltaTime) -> void
    {
        
    }

    auto Application::onGui (const sf::Time& deltaTime) -> void
    {
        ImGui::SFML::Update(m_window, deltaTime);
        ImGui::DockSpaceOverViewport();

        showMainMenuBar();

        if (m_showDemoWindow == true)
        {
            ImGui::ShowDemoWindow(&m_showDemoWindow);
        }
    }

    auto Application::onRender () -> void
    {
        m_window.clear(sf::Color::Black);

        ImGui::SFML::Render(m_window);
        m_window.display();
    }

    auto Application::onClose () -> void
    {  
        m_window.close(); 
    }

}
