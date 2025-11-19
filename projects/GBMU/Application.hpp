/**
 * @file    GBMU/Application.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains declarations for the Game Boy Emulator Frontend's primary
 *          application class.
 */

#pragma once

#include <GB/GB.h>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <pfd.hpp>

namespace gbmu
{

    class Application final
    {
    public: /* Public Methods *************************************************/
    
        Application (int argc, char** argv);
        ~Application ();
        auto start () -> int32_t;

    public: /* Public Methods - GB Context Callbacks **************************/

        auto onBusRead (const gbContext* context, uint16_t address,
            uint8_t value) -> void;
        auto onBusWrite (gbContext* context, uint16_t address,
            uint8_t value, uint8_t actual) -> void;
        auto onInstructionFetch (gbContext* context,
            uint16_t address, uint16_t opcode) -> bool;
        auto onInstructionExecute (gbContext* context,
            uint16_t address, uint16_t opcode, bool success) -> void;
        auto onFrame (const gbContext* context,
            const uint32_t* framebuffer, bool lcdEnabled) -> void;

    private: /* Private Methods - Application Lifecycle ***********************/

        auto onEvent (const sf::Event& event) -> void;
        auto onUpdate (const sf::Time& deltaTime) -> void;
        auto onGUI (const sf::Time& deltaTime) -> void;
        auto onRender (const uint32_t* framebuffer) -> void;

    private: /* Private Methods - ImGui Menu Bar ******************************/

        auto showMenuBar () -> void;
        auto showFileMenu () -> void;
        auto showEmulationMenu () -> void;
        auto showViewMenu () -> void;
        auto showHelpMenu () -> void;

    private: /* Private Methods - ImGui Console Window ************************/

        auto showConsoleWindow () -> void;

    private: /* Private Methods - Dialogs *************************************/

        auto showOpenCartridgeDialog () -> void;

    private: /* Private Methods - Utility Functions ***************************/

        auto loadCartridge (const std::string& filepath) -> bool;
        auto unloadCartridge () -> void;
        auto parseArguments (int argc, char** argv) -> void;

    private: /* Private Members ***********************************************/

        gbContext*           m_gb { nullptr };
        gbCartridge*         m_cart { nullptr };
        sf::RenderWindow     m_window;
        sf::Clock            m_clock;
        bool                 m_imguiInit { false };

    private: /* Private Members - Emulation Options ***************************/

        bool                 m_blarggMode { true };

    private: /* Private Members - Show Windows ********************************/

        bool                 m_showDemoWindow { false };
        bool                 m_showConsoleWindow { true };

    private: /* Private Members - Console Output Window ***********************/
    
        std::stringstream                  m_consoleBuffer;
        std::streambuf*                    m_oldCoutBuf { nullptr };
        std::streambuf*                    m_oldCerrBuf { nullptr };
        std::unique_ptr<std::streambuf>    m_coutTee;
        std::unique_ptr<std::streambuf>    m_cerrTee;

    };}
