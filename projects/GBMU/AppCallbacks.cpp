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

/* Private Macros *************************************************************/

#define gbmuBind(function, ...) \
    std::bind(&Application::function, this, __VA_ARGS__)

/* Private Methods - Context Callbacks ****************************************/

namespace gbmu
{

    auto Application::installCallbacks () -> void
    {
        using namespace std::placeholders;
        m_systemContext->setBusReadCallback(gbmuBind(onBusRead, _1, _2));
        m_systemContext->setBusWriteCallback(gbmuBind(onBusWrite, _1, _2, _3));
    }

    auto Application::onBusRead (
        std::uint16_t address,
        std::uint8_t& value
    ) -> void
    {
        
    }

    auto Application::onBusWrite (
        std::uint16_t       address,
        const std::uint8_t& value,
        std::uint8_t&       actualValue
    ) -> void
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
