/**
 * @file    GBMU/AppDialogs.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application's
 *          popup dialog methods.
 */

/* Private Includes ***********************************************************/

#include <pfd.hpp>
#include <GBMU/Application.hpp>

/* Private Methods - Dialogs **************************************************/

namespace gbmu
{
    auto Application::showAboutDialog () -> void
    {
        pfd::message(
            "About GBMU",
            "GBMU - Game Boy Emulator Frontend Application\n"
            "Version 0.1.0\n"
            "By: Dennis W. Griffin <dgdev1024@gmail.com>",
            pfd::choice::ok, pfd::icon::info
        );
    }
}
