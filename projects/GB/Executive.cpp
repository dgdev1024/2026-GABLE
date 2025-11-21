/**
 * @file    GB/Executive.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's primary executive
 *          class.
 */

/* Public Includes ************************************************************/

#include <GB/Executive.hpp>

/* Private Static Members *****************************************************/

namespace gb
{
    SystemContext* Executive::m_systemContext = nullptr;
}

/* Public Methods *************************************************************/

namespace gb
{

    auto Executive::use (
        SystemContext* context
    ) -> void
    {
        m_systemContext = context;
    }

    auto Executive::tick () -> Result<void>
    {
        // - Check System Context
        if (m_systemContext == nullptr)
        {
            return error("No system context is currently set for the executive.");
        }

        return m_systemContext->tick();
    }

}
