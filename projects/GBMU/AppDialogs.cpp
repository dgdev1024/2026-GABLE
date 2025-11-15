/**
 * @file    GBMU/AppDialogs.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application
 *          class's dialog methods.
 */

/* Private Includes ***********************************************************/

#include <filesystem>
#include <GBMU/Application.hpp>

/* Public Methods - Dialogs ***************************************************/

namespace gbmu
{

    auto Application::showOpenCartridgeDialog () -> void
    {
        auto result = pfd::open_file {
            "Open Cartridge ROM",
            std::filesystem::current_path().string(),
            {
                "Game Boy ROM Image Files (*.gb, *.gbc)", "*.gb *.gbc",
                "All Files", "*"
            }
        }.result();

        if (!result.empty())
        {
            loadCartridge(result.front());
        }
    }

}
