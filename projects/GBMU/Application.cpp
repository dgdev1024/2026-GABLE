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

    Application::Application ()
    {
        // - Create the Game Boy Emulator Core context.
        m_gb = gbCreateContext();
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
    }

    Application::~Application ()
    {
        // - Shutdown ImGui-SFML.
        ImGui::SFML::Shutdown();

        // - Destroy the Game Boy Emulator Core context.
        gbDestroyContext(m_gb);
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
        ImGui::ShowDemoWindow();
    }

    auto Application::onRender (const uint32_t* framebuffer) -> void
    {
        m_window.clear(sf::Color::Black);

        ImGui::SFML::Render(m_window);
        m_window.display();
    }

}
