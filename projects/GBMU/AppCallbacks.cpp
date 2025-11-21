/**
 * @file    GBMU/AppCallbacks.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application's
 *          context callback and installation methods.
 */

/* Private Includes ***********************************************************/

#include <GBMU/Application.hpp>

/* Private Methods - Context Callbacks ****************************************/

namespace gbmu
{

    auto Application::installCallbacks () -> void
    {
        
    }

    auto Application::onFrame () -> void
    {
        // - Update delta time.
        sf::Time deltaTime = m_deltaClock.restart();

        // - Process Events.
        while (const auto event = m_window.pollEvent())
        {
            onEvent(event);
        }

        // - Update Application State.
        onUpdate(deltaTime);
        onGui(deltaTime);
        onRender();
    }

}
