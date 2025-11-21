/**
 * @file    GB/Executive.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains declarations for the Game Boy Emulator's primary executive
 *          class.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/SystemContext.hpp>

/* Public Classes *************************************************************/

namespace gb
{
    /**
     * @brief   Defines a static class which represents the "executive" of the
     *          Game Boy Emulator.
     * 
     * The executive is responsible for actually driving the operation of the
     * Game Boy Emulator and its components. It manages the main emulation
     * loop, timing, and coordination between various subsystems.
     */
    class GB_API Executive final
    {
    public:

        /**
         * @brief   Configures the executive to use the provided system context
         *          for its operations.
         * 
         * @param   context     A pointer to the system context instance to be
         *                      used by the executive. Pass `nullptr` to un-set
         *                      any currently set context.
         */
        static auto use (
            SystemContext* context
        ) -> void;

        /**
         * @brief   Ticks the system context currently being managed by the
         *          executive, causing it and its components to advance their
         *          state by one emulation step.
         * 
         * @return  On success, returns `void`;
         *          On error, returns a string describing the error that occurred.
         */
        static auto tick () -> Result<void>;

    private:

        /**
         * @brief   Points to the system context instance currently being
         *          managed by the executive.
         */
        static SystemContext* m_systemContext;

    };
}