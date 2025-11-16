/**
 * @file    GBMU/Application.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend's primary
 *          application class.
 */

/* Private Includes ***********************************************************/

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <imgui-SFML.h>
#include <GBMU/Application.hpp>

/* Private Functions **********************************************************/

namespace gbmu
{

    static auto onBusReadStatic (const gbContext* context, uint16_t address,
        uint8_t value) -> void
    {
        Application* app = reinterpret_cast<Application*>(
            gbGetUserdata(context)
        );

        if (app != nullptr)
        {
            app->onBusRead(context, address, value);
        }
    }

    static auto onBusWriteStatic (gbContext* context, uint16_t address,
        uint8_t value, uint8_t actual) -> void
    {
        Application* app = reinterpret_cast<Application*>(
            gbGetUserdata(context)
        );

        if (app != nullptr)
        {
            app->onBusWrite(context, address, value, actual);
        }
    }

}

/* Public Methods *************************************************************/

namespace gbmu
{

    Application::Application (int argc, char** argv)
    {
        // - Create the Game Boy Emulator Core context.
        m_gb = gbCreateContext(false);
        if (m_gb == nullptr)
        {
            throw std::runtime_error { "Error creating GB context!" };
        }

        // - Make this context the current context, and set this application
        //   as its userdata.
        gbMakeContextCurrent(m_gb);
        gbSetUserdata(m_gb, this);

        // - Set the context's callbacks.
        gbSetBusReadCallback(m_gb, onBusReadStatic);
        gbSetBusWriteCallback(m_gb, onBusWriteStatic);

        // - Initialize the SFML Render Window.
        m_window.create(
            sf::VideoMode { 1280, 720 },
            "GABLE Game Boy Emulator Frontend"
        );
        m_window.setFramerateLimit(60);

        // - Initialize ImGui-SFML.
        m_imguiInit = ImGui::SFML::Init(m_window);

        // - Parse command-line arguments.
        parseArguments(argc, argv);
    }

    Application::~Application ()
    {
        // - Shutdown ImGui-SFML.
        ImGui::SFML::Shutdown();

        // - Destroy the Game Boy Emulator Core context.
        gbDestroyContext(m_gb);
        gbDestroyCartridge(m_cart);
    }

    auto Application::start () -> int32_t
    {
        std::uint32_t ticks = 0;
        while (m_window.isOpen())
        {
            if ((++ticks % 70224) == 0)
            {
                onFrame(m_gb, nullptr, false);
            }
        }

        return 0;
    }

}

/* Public Methods - GB Context Callbacks **************************************/

namespace gbmu
{

    auto Application::onBusRead (const gbContext* context, uint16_t address,
        uint8_t value) -> void
    {
    }

    auto Application::onBusWrite (gbContext* context, uint16_t address,
        uint8_t value, uint8_t actual) -> void
    {
    }

    auto Application::onFrame (const gbContext* context,
        const uint32_t* framebuffer, bool lcdEnabled) -> void
    {
        sf::Time deltaTime = m_clock.restart();

        // - Process SFML events.
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            onEvent(event);
        }
        
        // - Update Application state and ImGui-SFML.
        onUpdate(deltaTime);
        onGUI(deltaTime);

        // - Render Application contents and ImGui-SFML.
        onRender(framebuffer);
    }

}

/* Private Methods - Application Lifecycle ************************************/

namespace gbmu
{

    auto Application::onEvent (const sf::Event& event) -> void
    {
        ImGui::SFML::ProcessEvent(m_window, event);
        
        if (event.type == sf::Event::Closed)
        {
            m_window.close();
        }
    }

    auto Application::onUpdate (const sf::Time& deltaTime) -> void
    {

    }

    auto Application::onGUI (const sf::Time& deltaTime) -> void
    {
        ImGui::SFML::Update(m_window, deltaTime);
        ImGui::DockSpaceOverViewport();

        showMenuBar();
        
        if (m_showDemoWindow)
        {
            ImGui::ShowDemoWindow(&m_showDemoWindow);
        }
    }

    auto Application::onRender (const uint32_t* framebuffer) -> void
    {
        m_window.clear(sf::Color::Black);

        ImGui::SFML::Render(m_window);
        m_window.display();
    }

}

/* Private Methods - Utility Functions ****************************************/

namespace gbmu
{

    auto Application::loadCartridge (const std::string& filepath) -> bool
    {
        gbCartridge* cart = gbCreateCartridge(filepath.c_str());
        if (cart == nullptr)
        {
            pfd::message(
                "Error Loading Cartridge",
                std::format("Could not load cartridge from file '{}'.", filepath),
                pfd::choice::ok,
                pfd::icon::error
            );

            return false;
        }
        
        gbAttachCartridge(m_gb, cart);
        gbDestroyCartridge(m_cart);
        m_cart = cart;

        const auto header = gbGetCartridgeHeader(m_cart);
        const char* title = gbGetCartridgeTitle(header);
        m_window.setTitle(
            std::format(
                "{} - [{}] - GABLE Game Boy Emulator Frontend",
                title != nullptr ? title : "Untitled Cartridge",
                filepath
            )
        );

        return true;
    }

    auto Application::unloadCartridge () -> void
    {
        gbAttachCartridge(m_gb, nullptr);
        gbDestroyCartridge(m_cart);
        m_cart = nullptr;

        m_window.setTitle("GABLE Game Boy Emulator Frontend");
    }

    auto Application::parseArguments (int argc, char** argv) -> void
    {
        // -r <path>, --rom <path> : Load the specified ROM file.
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if ((arg == "-r" || arg == "--rom") && (i + 1) < argc)
            {
                std::string romPath = argv[++i];
                loadCartridge(romPath);
            }
        }
    }

}
