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

/* Private Classes ************************************************************/

namespace gbmu
{

    /**
     * @brief Custom streambuf that writes to two destinations simultaneously.
     * 
     * This class allows output to be "teed" to both the original stream
     * (console) and a secondary buffer (for ImGui console window).
     */
    class TeeStreambuf final : public std::streambuf
    {
    public:
        TeeStreambuf (std::streambuf* sb1, std::streambuf* sb2) : 
            m_sb1   { sb1 },
            m_sb2   { sb2 }
        {}

    protected:
        auto overflow (std::int32_t c) -> std::int32_t override
        {
            if (c == EOF)
            {
                return !EOF;
            }

            const std::int32_t r1 = m_sb1->sputc(static_cast<char>(c));
            const std::int32_t r2 = m_sb2->sputc(static_cast<char>(c));
            return (r1 == EOF || r2 == EOF) ? EOF : c;
        }

        auto sync () -> std::int32_t override
        {
            const std::int32_t r1 = m_sb1->pubsync();
            const std::int32_t r2 = m_sb2->pubsync();
            return (r1 == 0 && r2 == 0) ? 0 : -1;
        }

    private:
        std::streambuf* m_sb1 { nullptr };
        std::streambuf* m_sb2 { nullptr };

    };

}

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

    static auto onInstructionFetchStatic (gbContext* context,
        uint16_t address, uint16_t opcode) -> bool
    {
        Application* app = reinterpret_cast<Application*>(
            gbGetUserdata(context)
        );

        if (app != nullptr)
        {
            return app->onInstructionFetch(context, address, opcode);
        }

        return true;
    }

    static auto onInstructionExecuteStatic (gbContext* context,
        uint16_t address, uint16_t opcode, bool success) -> void
    {
        Application* app = reinterpret_cast<Application*>(
            gbGetUserdata(context)
        );

        if (app != nullptr)
        {
            app->onInstructionExecute(context, address, opcode, success);
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
        gbProcessor* processor = gbGetProcessor(m_gb);

        gbSetBusReadCallback(m_gb, onBusReadStatic);
        gbSetBusWriteCallback(m_gb, onBusWriteStatic);
        gbSetInstructionFetchCallback(processor, onInstructionFetchStatic);
        gbSetInstructionExecuteCallback(processor, onInstructionExecuteStatic);

        // - Initialize the SFML Render Window.
        m_window.create(
            sf::VideoMode { 1280, 720 },
            "GABLE Game Boy Emulator Frontend"
        );
        m_window.setVerticalSyncEnabled(true);

        // - Initialize ImGui-SFML.
        m_imguiInit = ImGui::SFML::Init(m_window);

        // - Enable Docking
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // - Redirect stdout and stderr to console buffer while maintaining
        //   terminal output.
        m_oldCoutBuf = std::cout.rdbuf();
        m_oldCerrBuf = std::cerr.rdbuf();
        m_coutTee = std::make_unique<TeeStreambuf>(m_oldCoutBuf, m_consoleBuffer.rdbuf());
        m_cerrTee = std::make_unique<TeeStreambuf>(m_oldCerrBuf, m_consoleBuffer.rdbuf());
        std::cout.rdbuf(m_coutTee.get());
        std::cerr.rdbuf(m_cerrTee.get());

        // - Parse command-line arguments.
        parseArguments(argc, argv);
    }

    Application::~Application ()
    {
        // - Restore original stream buffers (unique_ptr will auto-cleanup).
        std::cout.rdbuf(m_oldCoutBuf);
        std::cerr.rdbuf(m_oldCerrBuf);

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
            if (m_cart != nullptr)
            {
                gbTick(m_gb);
            }

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
        if (m_blarggMode == true)
        {
            if (address == GB_PR_SB)
            {
                if (std::isspace(value) || std::isprint(value))
                {
                    std::cout << static_cast<char>(value) << std::flush;
                }
            }
        }
    }

    auto Application::onInstructionFetch (gbContext* context,
        uint16_t address, uint16_t opcode) -> bool
    {
        return true;
    }

    auto Application::onInstructionExecute (gbContext* context,
        uint16_t address, uint16_t opcode, bool success) -> void
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
        showConsoleWindow();
        
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

        if (title == nullptr || !std::isprint(title[0]))
        {
            m_window.setTitle(
                std::format(
                    "Untitled Cartridge - [{}] - GABLE Game Boy Emulator Frontend",
                    filepath
                )
            );
        }
        else
        {
            m_window.setTitle(
                std::format(
                    "{} - [{}] - GABLE Game Boy Emulator Frontend",
                    title != nullptr ? title : "Untitled Cartridge",
                    filepath
                )
            );
        }

        // - Clear the console buffer.
        m_consoleBuffer.str(std::string());
        m_consoleBuffer.clear();

        return true;
    }

    auto Application::unloadCartridge () -> void
    {
        gbAttachCartridge(m_gb, nullptr);
        gbDestroyCartridge(m_cart);
        m_cart = nullptr;

        // - Clear the console buffer.
        m_consoleBuffer.str(std::string());
        m_consoleBuffer.clear();

        m_window.setTitle("GABLE Game Boy Emulator Frontend");
    }

    auto Application::parseArguments (int argc, char** argv) -> void
    {
        // -r <path>, --rom <path> : Load the specified ROM file.
        // -s <max ticks>, --steps <max ticks> : Set the maximum tick count.
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
