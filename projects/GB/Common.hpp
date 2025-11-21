/**
 * @file    GB/Common.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains includes and definitions commonly used by the Game Boy
 *          Emulation Backend library and its client applications.
 */

#pragma once

/* Public Includes ************************************************************/

#include <bit>
#include <chrono>
#include <expected>
#include <format>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <cstdint>

/* Static Assertions **********************************************************/

static_assert(__cplusplus >= 202302L, "This project requires C++23 or later.");
static_assert(std::endian::native == std::endian::little, 
    "This project requires a little-endian architecture.");
static_assert(__CHAR_BIT__ == 8, 
    "This project requires a character (byte) size of 8 bits.");

/* Public Macros - Import/Export Symbols **************************************/

#if defined(GB_BUILD_STATIC)
    #define GB_API
#else
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(GB_BUILDING_SHARED)
            #define GB_API __declspec(dllexport)
        #else
            #define GB_API __declspec(dllimport)
        #endif
    #else
        #if defined(GB_BUILDING_SHARED)
            #define GB_API __attribute__((visibility("default")))
        #else
            #define GB_API
        #endif
    #endif
#endif

/* Public Types ***************************************************************/

namespace gb
{
    /**
     * @brief   Defines the standard result type used throughout the Game Boy
     *          Emulation Backend library.
     * 
     * All functions and methods in the Game Boy Emulation Backend library's
     * public API (and some of its internal API) will return this type to
     * indicate success or failure. On success, the contained value will be of
     * type `T` (this type can also be `void` for functions that do not
     * return a value when successful). On failure, the contained value will be a
     * `std::string` describing the failure.
     * 
     * @tparam  T   The type of the value contained in the `std::expected` on
     *              success.
     */
    template <typename T>
    using Result = std::expected<T, std::string>;

    /**
     * @brief   Defines the error type returned on failure by functions and
     *          methods in the Game Boy Emulation Backend library.
     */
    using Error = std::unexpected<std::string>;
}

/* Public Constants and Enumerations ******************************************/

namespace gb
{
    /**
     * @brief   Defines constants representing the start and end addresses of
     *          important regions in the Game Boy's 16-bit address space, and
     *          the regions' sizes in bytes.
     */
    constexpr std::uint16_t
        ROM0_START              = 0x0000,
        ROM0_END                = 0x3FFF,
        ROM0_SIZE               = ROM0_END - ROM0_START + 1,
        ROMX_START              = 0x4000,
        ROMX_END                = 0x7FFF,
        ROMX_SIZE               = ROMX_END - ROMX_START + 1,
        VRAM_START              = 0x8000,
        VRAM_END                = 0x9FFF,
        VRAM_SIZE               = VRAM_END - VRAM_START + 1,
        EXTRAM_START            = 0xA000,
        EXTRAM_END              = 0xBFFF,
        EXTRAM_SIZE             = EXTRAM_END - EXTRAM_START + 1,
        WRAM0_START             = 0xC000,
        WRAM0_END               = 0xCFFF,
        WRAM0_SIZE              = WRAM0_END - WRAM0_START + 1,
        WRAMX_START             = 0xD000,
        WRAMX_END               = 0xDFFF,
        WRAMX_SIZE              = WRAMX_END - WRAMX_START + 1,
        ECHO_START              = 0xE000,
        ECHO_END                = 0xFDFF,
        ECHO_SIZE               = ECHO_END - ECHO_START + 1,
        OAM_START               = 0xFE00,
        OAM_END                 = 0xFE9F,
        OAM_SIZE                = OAM_END - OAM_START + 1,
        UNUSED_START            = 0xFEA0,
        UNUSED_END              = 0xFEFF,
        UNUSED_SIZE             = UNUSED_END - UNUSED_START + 1,
        HRAM_START              = 0xFF80,
        HRAM_END                = 0xFFFE,
        HRAM_SIZE               = HRAM_END - HRAM_START + 1;

    /**
     * @brief   Enumerates specific addresses in the Game Boy's memory map which
     *          are mapped to port registers managing various hardware
     *          components.
     */
    enum PortRegister : std::uint16_t
    {
        PR_P1       = 0xFF00,   /** @brief `P1/JOYP` - Joypad Input (Mixed). */
        PR_SB       = 0xFF01,   /** @brief `SB` - Serial Transfer Data (R/W). */
        PR_SC       = 0xFF02,   /** @brief `SC` - Serial Transfer Control (Mixed). */
        PR_DIV      = 0xFF04,   /** @brief `DIV` - Divider Register (R/W). */
        PR_TIMA     = 0xFF05,   /** @brief `TIMA` - Timer Counter (R/W). */
        PR_TMA      = 0xFF06,   /** @brief `TMA` - Timer Modulo (R/W). */
        PR_TAC      = 0xFF07,   /** @brief `TAC` - Timer Control (R/W). */
        PR_IF       = 0xFF0F,   /** @brief `IF` - Interrupt Flags (R/W). */
        PR_NR10     = 0xFF10,   /** @brief `NR10` - Sound Channel 1 Sweep (R/W). */
        PR_NR11     = 0xFF11,   /** @brief `NR11` - Sound Channel 1 Length Timer & Duty Cycle (Mixed). */
        PR_NR12     = 0xFF12,   /** @brief `NR12` - Sound Channel 1 Volume & Envelope (R/W). */
        PR_NR13     = 0xFF13,   /** @brief `NR13` - Sound Channel 1 Period Low (W). */
        PR_NR14     = 0xFF14,   /** @brief `NR14` - Sound Channel 1 Period High & Control (Mixed). */
        PR_NR21     = 0xFF16,   /** @brief `NR21` - Sound Channel 2 Length Timer & Duty Cycle (Mixed). */
        PR_NR22     = 0xFF17,   /** @brief `NR22` - Sound Channel 2 Volume & Envelope (R/W). */
        PR_NR23     = 0xFF18,   /** @brief `NR23` - Sound Channel 2 Period Low (W). */
        PR_NR24     = 0xFF19,   /** @brief `NR24` - Sound Channel 2 Period High & Control (Mixed). */
        PR_NR30     = 0xFF1A,   /** @brief `NR30` - Sound Channel 3 DAC Enable (R/W). */
        PR_NR31     = 0xFF1B,   /** @brief `NR31` - Sound Channel 3 Length Timer (W). */
        PR_NR32     = 0xFF1C,   /** @brief `NR32` - Sound Channel 3 Output Level (R/W). */
        PR_NR33     = 0xFF1D,   /** @brief `NR33` - Sound Channel 3 Period Low (W). */
        PR_NR34     = 0xFF1E,   /** @brief `NR34` - Sound Channel 3 Period High & Control (Mixed). */
        PR_NR41     = 0xFF20,   /** @brief `NR41` - Sound Channel 4 Length Timer (W). */
        PR_NR42     = 0xFF21,   /** @brief `NR42` - Sound Channel 4 Volume & Envelope (R/W). */
        PR_NR43     = 0xFF22,   /** @brief `NR43` - Sound Channel 4 Frequency & Randomness (R/W). */
        PR_NR44     = 0xFF23,   /** @brief `NR44` - Sound Channel 4 Control (Mixed). */
        PR_NR50     = 0xFF24,   /** @brief `NR50` - Master Volume & VIN Panning (R/W). */
        PR_NR51     = 0xFF25,   /** @brief `NR51` - Sound Panning (R/W). */
        PR_NR52     = 0xFF26,   /** @brief `NR52` - Sound On/Off (Mixed). */
        PR_LCDC     = 0xFF40,   /** @brief `LCDC` - LCD Control (R/W). */
        PR_STAT     = 0xFF41,   /** @brief `STAT` - LCD Status (Mixed). */
        PR_SCY      = 0xFF42,   /** @brief `SCY` - Viewport Y Position (R/W). */
        PR_SCX      = 0xFF43,   /** @brief `SCX` - Viewport X Position (R/W). */
        PR_LY       = 0xFF44,   /** @brief `LY` - LCD Y Coordinate (R). */
        PR_LYC      = 0xFF45,   /** @brief `LYC` - LY Compare (R/W). */
        PR_DMA      = 0xFF46,   /** @brief `DMA` - OAM DMA Source Address & Start (R/W). */
        PR_BGP      = 0xFF47,   /** @brief `BGP` - BG Palette Data (R/W, DMG only). */
        PR_OBP0     = 0xFF48,   /** @brief `OBP0` - OBJ Palette 0 Data (R/W, DMG only). */
        PR_OBP1     = 0xFF49,   /** @brief `OBP1` - OBJ Palette 1 Data (R/W, DMG only). */
        PR_WY       = 0xFF4A,   /** @brief `WY` - Window Y Position (R/W). */
        PR_WX       = 0xFF4B,   /** @brief `WX` - Window X Position plus 7 (R/W). */
        PR_KEY0     = 0xFF4C,   /** @brief `KEY0/SYS` - CPU Mode Select (Mixed, CGB only). */
        PR_KEY1     = 0xFF4D,   /** @brief `KEY1/SPD` - Prepare Speed Switch (Mixed, CGB only). */
        PR_VBK      = 0xFF4F,   /** @brief `VBK` - VRAM Bank (R/W, CGB only). */
        PR_BANK     = 0xFF50,   /** @brief `BANK` - Boot ROM Mapping Control (W). */
        PR_HDMA1    = 0xFF51,   /** @brief `HDMA1` - VRAM DMA Source High (W, CGB only). */
        PR_HDMA2    = 0xFF52,   /** @brief `HDMA2` - VRAM DMA Source Low (W, CGB only). */
        PR_HDMA3    = 0xFF53,   /** @brief `HDMA3` - VRAM DMA Destination High (W, CGB only). */
        PR_HDMA4    = 0xFF54,   /** @brief `HDMA4` - VRAM DMA Destination Low (W, CGB only). */
        PR_HDMA5    = 0xFF55,   /** @brief `HDMA5` - VRAM DMA Length/Mode/Start (R/W, CGB only). */
        PR_RP       = 0xFF56,   /** @brief `RP` - Infrared Communications Port (Mixed, CGB only). */
        PR_BCPS     = 0xFF68,   /** @brief `BCPS/BGPI` - Background Color Palette Specification (R/W, CGB only). */
        PR_BCPD     = 0xFF69,   /** @brief `BCPD/BGPD` - Background Color Palette Data (R/W, CGB only). */
        PR_OCPS     = 0xFF6A,   /** @brief `OCPS/OBPI` - OBJ Color Palette Specification (R/W, CGB only). */
        PR_OCPD     = 0xFF6B,   /** @brief `OCPD/OBPD` - OBJ Color Palette Data (R/W, CGB only). */
        PR_OPRI     = 0xFF6C,   /** @brief `OPRI` - Object Priority Mode (R/W, CGB only). */
        PR_SVBK     = 0xFF70,   /** @brief `SVBK/WBK` - WRAM Bank (R/W, CGB only). */
        PR_PCM12    = 0xFF76,   /** @brief `PCM12` - Audio Digital Outputs 1 & 2 (R, CGB only). */
        PR_PCM34    = 0xFF77,   /** @brief `PCM34` - Audio Digital Outputs 3 & 4 (R, CGB only). */
        PR_IE       = 0xFFFF    /** @brief `IE` - Interrupt Enable (R/W). */
    };

}

/* Public Unions and Structures ***********************************************/

namespace gb
{

    /**
     * @brief   Defines a helper structure which define check rules to enforce
     *          in order to determine whether or not certain parts of the Game
     *          Boy's memory map can be accessed.
     */
    struct AccessRules final
    {
        /**
         * @brief   Indicates that access rules should be checked for
         *          component-external accesses. This rule should be used for
         *          such accesses as when the CPU is reading from or writing to
         *          memory-mapped components.
         */
        bool external = false;

        /**
         * @brief   Indicates that access rules should be checked for
         *          component-internal accesses. This rule should be used for
         *          when a component (such as the PPU) is accessing its own
         *          memory (such as VRAM or OAM).
         */
        bool internal = false;

        /**
         * @brief   In CGB mode, when the CPU is undergoing a speed switch, the
         *          Game Boy Emulator and its components exist in a strange,
         *          transitional state, during which certain accesses which would
         *          normally be disallowed by other access rules are permitted,
         *          and vice versa. This flag determines whether or not these
         *          overriding, speed-switch access rules should be enforced.
         */
        bool speedSwitch = false;

        /**
         * @brief   Not an access rule, per se, but indicates that, if an MBC3
         *          cartridge with a timer is being used, its real-time clock
         *          registers should be updated before attempting the access.
         */
        bool updateRTC = true;

        /**
         * @brief   Indicates an access rule specific to a particular component.
         */
        bool component1 = false;

        /**
         * @brief   Indicates an access rule specific to a particular component.
         */
        bool component2 = false;
    };

}

/* Public Functions ***********************************************************/

namespace gb
{

    /**
     * @brief   Creates an `Error` object containing a formatted error message.
     * 
     * This function uses C++23's formatting library to create a formatted error
     * message. It accepts a format string and a variable number of arguments to
     * be formatted into the string.
     * 
     * @tparam  As  The types of the arguments to be formatted into the error
     *              message.
     * 
     * @param   format      The format string.
     * @param   arguments   The arguments to be formatted into the error message.
     * 
     * @return  An `Error` object containing the formatted error message.
     */
    template <typename... As>
    inline auto error (
        const std::string&  format,
        As&&...             arguments
    ) -> Error
    {
        return std::unexpected { 
            std::vformat(
                format, 
                std::make_format_args(arguments...)
            ) 
        };
    }

}
