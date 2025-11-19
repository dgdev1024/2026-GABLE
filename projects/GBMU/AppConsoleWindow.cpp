/**
 * @file    GBMU/AppConsoleWindow.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-19
 * 
 * @brief   Contains definitions for the Game Boy Emulator Frontend application
 *          class's console window methods.
 */

/* Private Includes ***********************************************************/

#include <imgui.h>
#include <imgui_internal.h>
#include <GBMU/Application.hpp>

/* Public Methods - ImGui Console Window **************************************/

namespace gbmu
{

    auto Application::showConsoleWindow () -> void
    {
        if (!m_showConsoleWindow)
        {
            return;
        }

        // - Window Flags
        auto windowFlags = ImGuiWindowFlags_None;

        // - Prepare Window
        

        // - Begin Window
        ImGui::Begin("Console Output", &m_showConsoleWindow, windowFlags);
        {
            // - Create scrollable region with horizontal scrollbar support.
            ImGui::BeginChild(
                "ConsoleScrolling",
                ImVec2 { 0.0f, 0.0f },
                false,
                ImGuiWindowFlags_HorizontalScrollbar
            );

            // - Display the console buffer contents.
            std::string consoleText = m_consoleBuffer.str();
            ImGui::TextUnformatted(consoleText.c_str());

            // - Auto-scroll to bottom when new content is added.
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
        }
        ImGui::End();
    }

}
