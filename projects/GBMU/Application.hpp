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
#include <GB/Executive.hpp>

/* Public Constants ***********************************************************/

namespace gbmu
{
    constexpr const char* APPLICATION_TITLE { "GBMU Game Boy Emulator" };
}

/* Public Classes *************************************************************/

namespace gbmu
{
    class Application final
    {
    public: /* Public Methods *************************************************/

        Application (int argc, char** argv);
        ~Application ();

        auto start () -> std::int32_t;

    private: /* Private Methods ***********************************************/

        auto parseArguments (int argc, char** argv) -> void;
        auto loadCartridge (const std::filesystem::path& path) -> void;
        auto unloadCartridge () -> void;

    private: /* Private Methods - Application Lifecycle ***********************/

        auto onEvent (const std::optional<sf::Event>& event) -> void;
        auto onUpdate (const sf::Time& deltaTime) -> void;
        auto onGui (const sf::Time& deltaTime) -> void;
        auto onRender () -> void;
        auto onClose () -> void;

    private: /* Private Methods - Context Callbacks ***************************/

        auto installCallbacks () -> void;
        auto onBusRead (std::uint16_t address, std::uint8_t& value) -> void;
        auto onBusWrite (std::uint16_t address, const std::uint8_t& value,
            std::uint8_t& actualValue) -> void;
        auto onFrame () -> void;

    private: /* Private Methods - ImGui Main Menu Bar *************************/
    
        auto showMainMenuBar () -> void;
        auto showFileMenu () -> void;
        auto showViewMenu () -> void;
        auto showHelpMenu () -> void;

    private: /* Private Methods - Dialogs *************************************/

        auto showOpenCartridgeDialog () -> void;
        auto showAboutDialog () -> void;

    private: /* Private Members ***********************************************/

        std::unique_ptr<gb::SystemContext>  m_systemContext { nullptr };
        std::shared_ptr<gb::ICartridge>     m_cartridge { nullptr };
        std::filesystem::path               m_cartridgePath {};
        gb::Result<void>                    m_lastTickResult {};
        sf::RenderWindow                    m_window;
        sf::Clock                           m_deltaClock;
        bool                                m_imguiInitialized { false };

    private: /* Private Members - Show Windows ********************************/

        bool                m_showDemoWindow { true };

    };
}
