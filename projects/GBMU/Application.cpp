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
#include <pfd.hpp>
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
        m_window.create(videoMode, APPLICATION_TITLE, sf::Style::Default);
        m_window.setFramerateLimit(60);

        // - Initialize Dear ImGui
        m_imguiInitialized = ImGui::SFML::Init(m_window);
    
        // - Configure ImGui
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

        // - Setup System Context
        m_systemContext = std::make_unique<gb::SystemContext>();
        gb::Executive::use(m_systemContext.get());
        installCallbacks();

        // - Parse Command-Line Arguments
        parseArguments(argc, argv);
    }

    Application::~Application ()
    {
        // - Shutdown System Context
        gb::Executive::use(nullptr);
        m_systemContext.reset();

        // - Shutdown Dear ImGui
        ImGui::SFML::Shutdown();
    }

    auto Application::start () -> std::int32_t
    {
        std::uint32_t tempTicks = 0;
        while (m_window.isOpen() == true)
        {
            if (m_lastTickResult.has_value() == false)
            {
                onFrame();
            }
            else
            {
                m_lastTickResult = gb::Executive::tick();
                if (++tempTicks % 70224 == 0)
                {
                    onFrame();
                }
            }
        }

        return 0;
    }

}

/* Private Methods ************************************************************/

namespace gbmu
{

    auto Application::parseArguments (int argc, char** argv) -> void
    {
        // `-r <path>`, `--rom <path>` : Load ROM file at specified path
        std::filesystem::path romPath {};

        for (int i = 1; i < argc; ++i)
        {
            std::string arg { argv[i] };
            if ((arg == "-r" || arg == "--rom") && (i + 1) < argc)
            {
                romPath = std::filesystem::absolute(argv[++i])
                    .lexically_normal();
            }
        }

        if (romPath.empty() == false)
        {
            loadCartridge(romPath);
        }
    }

    auto Application::loadCartridge (const std::filesystem::path& path) -> void
    {
        auto cartridgeResult = gb::ICartridge::make(path);
        if (cartridgeResult.has_value() == true)
        {
            m_cartridge = cartridgeResult.value();
            m_systemContext->attachCartridge(m_cartridge);
            m_cartridgePath = path;

            m_window.setTitle(
                std::format(
                    "{} - {}",
                    m_cartridgePath.filename().string(),
                    APPLICATION_TITLE
                )
            );
        }
        else
        {
            pfd::message(
                "Error Loading Cartridge",
                cartridgeResult.error(),
                pfd::choice::ok,
                pfd::icon::error
            );
        }
    }

    auto Application::unloadCartridge () -> void
    {
        m_systemContext->attachCartridge(nullptr);
        m_cartridge.reset();
        m_cartridgePath.clear();

        m_window.setTitle(
            APPLICATION_TITLE
        );
    }

}
