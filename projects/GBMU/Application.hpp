/**
 * @file    GBMU/Application.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains declarations for the Game Boy Emulator Frontend application's
 *          main class and associated methods.
 */

#pragma once

/* Public Includes ************************************************************/

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>

/* Public Classes *************************************************************/

namespace gbmu
{
    class Application final
    {
    public: /* Public Methods *************************************************/

        Application (int argc, char** argv);
        ~Application ();

        auto start () -> std::int32_t;

    private: /* Private Methods - Application Lifecycle ***********************/

        auto onEvent (const std::optional<sf::Event>& event) -> void;
        auto onUpdate (const sf::Time& deltaTime) -> void;
        auto onGui (const sf::Time& deltaTime) -> void;
        auto onRender () -> void;
        auto onClose () -> void;

    private: /* Private Methods - Context Callbacks ***************************/

        auto installCallbacks () -> void;
        auto onFrame () -> void;

    private: /* Private Methods - ImGui Main Menu Bar *************************/
    
        auto showMainMenuBar () -> void;
        auto showFileMenu () -> void;
        auto showViewMenu () -> void;
        auto showHelpMenu () -> void;

    private: /* Private Methods - Dialogs *************************************/

        auto showAboutDialog () -> void;

    private: /* Private Members ***********************************************/

        sf::RenderWindow    m_window;
        sf::Clock           m_deltaClock;
        bool                m_imguiInitialized { false };

    private: /* Private Members - Show Windows ********************************/

        bool                m_showDemoWindow { true };

    };
}
