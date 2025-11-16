--
-- @file    premake5.lua
-- @author  Dennis W. Griffin <dgdev1024@gmail.com>
-- @date    2025-11-10
--
-- @brief   The Premake5 build script for the GABLE Workspace.
--

-- Options ---------------------------------------------------------------------

newoption {
    trigger     = "build-static",
    description = "Build static libraries instead of shared libraries"
}

newoption {
    trigger     = "unused-is-error",
    description = "Treat unused variable/function/parameter warnings as errors"
}

-- Workspace -------------------------------------------------------------------

workspace "2026-GABLE"
    
    -- Language and Standard
    cdialect "C23"
    cppdialect "C++23"

    -- Extra Warnings; Treat Warnings as Errors
    warnings "Extra"
    fatalwarnings { "All" }

    -- Ignore warnings involving unused parameters, functions, variables, etc.
    if _OPTIONS["unused-is-error"] == nil then
        filter { "toolset:gcc or clang" }
            buildoptions { "-Wno-unused-parameter", "-Wno-unused-function", 
                "-Wno-unused-variable" }
        filter { "toolset:msc" }
            buildoptions { "/wd4100", "/wd4505", "/wd4189" }
        filter {}
    end

    -- Build Configurations
    configurations { "debug", "release", "distribute" }
    startproject "gablemu"

    -- Configuration Settings
    filter { "configurations:debug" }
        defines { "GB_DEBUG", "DEBUG" }
        symbols "On"
    filter { "configurations:release" }
        defines { "GB_RELEASE", "NDEBUG" }
        optimize "On"
    filter { "configurations:distribute" }
        defines { "GB_DISTRIBUTE", "NDEBUG" }
        optimize "Full"
        symbols "Off"
    filter { "system:linux" }
        defines { "GB_LINUX" }
    filter { "system:windows" }
        defines { "GB_WINDOWS" }
    filter {}

-- Project: `gb` - Game Boy Emulation Core Library -----------------------------

project "gb"
    if _OPTIONS["build-static"] then
        kind "StaticLib"
        pic "On"
        defines { "GB_BUILD_STATIC" }
    else
        kind "SharedLib"
        pic "On"
        defines { "GB_BUILD_SHARED" }
    end

    language "C"
    location "./build"
    targetdir "./build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "./build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    files { "./projects/GB/**.h", "./projects/GB/**.c" }
    includedirs { "./projects" }

-- Project: `gbt` - Game Boy Emulator Core Library Test Suite ------------------

project "gbt"
    kind "ConsoleApp"
    language "C"
    location "./build"
    targetdir "./build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "./build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    files { "./projects/GBT/**.h", "./projects/GBT/**.c" }
    includedirs { "./projects" }
    links { "gb" }

-- Project: `gbmu` - Game Boy Emulator Frontend --------------------------------

project "gbmu"
    kind "ConsoleApp"
    language "C++"
    location "./build"
    targetdir "./build/bin/%{cfg.system}-%{cfg.buildcfg}"
    objdir "./build/obj/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    files { 
        "./projects/GBMU/**.hpp", 
        "./projects/GBMU/**.cpp",
        "./vendor/dear-imgui/**.h",
        "./vendor/dear-imgui/**.cpp",
        "./vendor/dear-imgui-sfml/**.h",
        "./vendor/dear-imgui-sfml/**.cpp",
        "./vendor/pfd/pfd.hpp"
    }
    includedirs { 
        "./projects",
        "./vendor/dear-imgui",
        "./vendor/dear-imgui-sfml",
        "./vendor/pfd"
    }
    links {
        "gb",
        "sfml-audio",
        "sfml-graphics",
        "sfml-window",
        "sfml-network",
        "sfml-system",
    }

    -- Link OpenGL
    filter { "system:linux" }
        links { "GL" }
    filter { "system:windows" }
        links { "opengl32" }
    filter {}
