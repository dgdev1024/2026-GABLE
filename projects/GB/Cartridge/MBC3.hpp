/**
 * @file    GB/Cartridge/MBC3.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's MBC3 cartridge
 *          type.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Cartridge/ICartridge.hpp>

/* Public Classes *************************************************************/

namespace gb
{

    /**
     * @brief   Defines a class representing an MBC3 Game Boy cartridge with
     *          memory bank controller for ROM and RAM banking, and real-time
     *          clock functionality.
     */
    class GB_API MBC3Cartridge final : public ICartridge
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs an `MBC3Cartridge` instance with the specified
         *          cartridge header.
         * 
         * @param   header  The cartridge header information.
         */
        explicit MBC3Cartridge (
            const CartridgeHeader& header
        ) noexcept :
            ICartridge { header }
        {}

        /**
         * @brief   Reads a byte from the cartridge's read-only memory (ROM)
         *          at the specified address.
         * 
         * @param   relativeAddress The address from which to read the byte. Must
         *                          be between `0x0000` and `0x7FFF`.
         * 
         * @return  The byte read from the specified address.
         */
        auto readROM (
            const std::uint16_t relativeAddress
        ) noexcept -> std::uint8_t override;

        /**
         * @brief   Writes a byte to the cartridge's read-only memory (ROM) at
         *          the specified address.
         * 
         * @param   relativeAddress The address to which to write the byte. Must
         *                          be between `0x0000` and `0x7FFF`.
         * @param   value           The byte value to write.
         * 
         * @return  The actual byte written to the specified address, which may
         *          differ from the input value depending on the cartridge type.
         */
        auto writeROM (
            const std::uint16_t relativeAddress,
            const std::uint8_t  value
        ) noexcept -> std::uint8_t override;

        /**
         * @brief   Reads a byte from the cartridge's external random-access
         *          memory (RAM) at the specified address.
         * 
         * @param   relativeAddress The address from which to read the byte. Must
         *                          be between `0x0000` and `0x1FFF`.
         * 
         * @return  The byte read from the specified address.
         */
        auto readExternalRAM (
            const std::uint16_t relativeAddress
        ) noexcept -> std::uint8_t override;

        /**
         * @brief   Writes a byte to the cartridge's external random-access
         *          memory (RAM) at the specified address.
         * 
         * @param   relativeAddress The address to which to write the byte. Must
         *                          be between `0x0000` and `0x1FFF`.
         * @param   value           The byte value to write.
         * 
         * @return  The actual byte written to the specified address, which may
         *          differ from the input value depending on the cartridge type.
         */
        auto writeExternalRAM (
            const std::uint16_t relativeAddress,
            const std::uint8_t  value
        ) noexcept -> std::uint8_t override;

        /**
         * @brief   For MBC3 cartridges with real-time clock functionality, this
         *          method updates the unlatched RTC registers based on the
         *          elapsed time since the last update.
         */
        auto updateRTC () noexcept -> void;

    private: /* Private Methods ***********************************************/

        /**
         * @brief   Performs type-specific validation of the cartridge's header
         *          data.
         * 
         * @return  On success, returns `void`. 
         *          On failure, returns an error message.
         */
        auto validateByType () const noexcept -> Result<void> override;

        /**
         * @brief   Finalizes the cartridge by allocating necessary memory and
         *          loading the full ROM data from the provided file stream.
         * 
         * @param   file        The file stream from which to read the ROM data.
         * 
         * @return  On success, returns `void`. 
         *          On failure, returns an error message.
         */
        auto finalize (
            std::fstream& file
        ) noexcept -> Result<void> override;

    private: /* Private Members ***********************************************/

        /**
         * @brief   RAM and timer enable flag.
         */
        bool m_ramTimerEnable { false };

        /**
         * @brief   ROM bank register (7 bits).
         */
        std::uint8_t m_romBank { 1 };

        /**
         * @brief   RAM bank or RTC register select (4 bits).
         */
        std::uint8_t m_ramBank { 0 };

        /**
         * @brief   RTC seconds register (0-59).
         */
        std::uint8_t m_rtcSeconds { 0 };

        /**
         * @brief   RTC minutes register (0-59).
         */
        std::uint8_t m_rtcMinutes { 0 };

        /**
         * @brief   RTC hours register (0-23).
         */
        std::uint8_t m_rtcHours { 0 };

        /**
         * @brief   RTC day counter low register (0-255).
         */
        std::uint8_t m_rtcDayLow { 0 };

        /**
         * @brief   RTC day counter high register (bits: 0=MSB day, 6=halt, 7=carry).
         */
        std::uint8_t m_rtcDayHigh { 0 };

        /**
         * @brief   Latched RTC seconds value.
         */
        std::uint8_t m_latchedSeconds { 0 };

        /**
         * @brief   Latched RTC minutes value.
         */
        std::uint8_t m_latchedMinutes { 0 };

        /**
         * @brief   Latched RTC hours value.
         */
        std::uint8_t m_latchedHours { 0 };

        /**
         * @brief   Latched RTC day counter low value.
         */
        std::uint8_t m_latchedDayLow { 0 };

        /**
         * @brief   Latched RTC day counter high value.
         */
        std::uint8_t m_latchedDayHigh { 0 };

        /**
         * @brief   Latch state for clock data latching sequence.
         */
        std::uint8_t m_latchState { 0 };

        /**
         * @brief   Indicates whether the cartridge has real-time clock functionality.
         */
        bool m_hasTimer { false };

        /**
         * @brief   Indicates the last time the non-latched RTC registers were
         *          updated.
         */
        std::chrono::system_clock::time_point m_lastUpdateTime {};

    };

}
