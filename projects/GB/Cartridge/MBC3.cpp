/**
 * @file    GB/Cartridge/MBC3.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's MBC3 cartridge
 *          type.
 */

/* Public Includes ************************************************************/

#include <GB/Cartridge/MBC3.hpp>

/* Public Methods *************************************************************/

namespace gb
{

    auto MBC3Cartridge::readROM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {
        // - Determine the ROM bank to read from, based on the provided relative
        //   address and the current ROM bank register.
        std::uint8_t bank = 0;
        if (relativeAddress < ROM0_SIZE)
        {
            bank = 0;
        }
        else
        {
            bank = m_romBank;
        }

        // - Calculate the actual offset into the ROM data.
        //   - If the offset is out of bounds, return an open-bus value.
        std::size_t offset = 
            static_cast<std::size_t>(bank) * ROM0_SIZE + (relativeAddress & ROM0_END);
        if (offset >= m_rom.size())
        {
            return 0xFF;
        }

        return m_rom[offset];
    }

    auto MBC3Cartridge::writeROM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        // - Handle a write to one of the MBC3 control registers, based on the
        //   address range specified.
        //
        // `$0000` - `$1FFF`: RAM and Timer Enable
        if (relativeAddress < 0x2000)
        {
            m_ramTimerEnable = ((value & 0x0F) == 0x0A);
        }

        // `$2000` - `$3FFF`: ROM Bank Number
        else if (relativeAddress < 0x4000)
        {
            m_romBank = value & 0x7F;
            if (m_romBank == 0)
            {
                m_romBank = 1;
            }
        }
        
        // `$4000` - `$5FFF`: RAM Bank Number or RTC Register Select
        else if (relativeAddress < 0x6000)
        {
            m_ramBank = value & 0x0F;
        }
        
        // `$6000` - `$7FFF`: Latch Clock Data
        else if (relativeAddress < 0x8000)
        {
            if (m_hasTimer == false)
            {
                // - If the cartridge does not have a real-time clock, ignore
                //   the latch command.
                return 0xFF;
            }

            if (value == 0x00)
            {
                m_latchState = 1;
            }
            else if (value == 0x01 && m_latchState == 1)
            {
                // Latch the current RTC values
                m_latchedSeconds = m_rtcSeconds;
                m_latchedMinutes = m_rtcMinutes;
                m_latchedHours = m_rtcHours;
                m_latchedDayLow = m_rtcDayLow;
                m_latchedDayHigh = m_rtcDayHigh;
                m_latchState = 0;
            }
        }

        // - Nothing is actually written to ROM, so return an open-bus value.
        return 0xFF;
    }

    auto MBC3Cartridge::readExternalRAM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {
        // - Only read from RAM or RTC if it is enabled.
        if (m_ramTimerEnable == false)
        {
            return 0xFF;
        }

        // - Check if we are accessing RAM or RTC registers.
        if (m_ramBank <= 0x07)
        {
            // - Accessing external RAM bank.
            std::size_t offset = 
                static_cast<std::size_t>(m_ramBank) * 0x2000 + 
                (relativeAddress & 0x1FFF);
            if (offset >= m_extram.size())
            {
                return 0xFF;
            }
            return m_extram[offset];
        }
        else if (m_ramBank >= 0x08 && m_ramBank <= 0x0C)
        {
            if (m_hasTimer == false)
            {
                // - If the cartridge does not have a real-time clock, return
                //   an open-bus value.
                return 0xFF;
            }

            // - Accessing RTC register.
            switch (m_ramBank)
            {
                case 0x08: return m_latchedSeconds;
                case 0x09: return m_latchedMinutes;
                case 0x0A: return m_latchedHours;
                case 0x0B: return m_latchedDayLow;
                case 0x0C: return m_latchedDayHigh;
                default: return 0xFF;
            }
        }

        // - Invalid bank selection.
        return 0xFF;
    }

    auto MBC3Cartridge::writeExternalRAM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        // - Only write to RAM or RTC if it is enabled.
        if (m_ramTimerEnable == false)
        {
            return 0xFF;
        }

        // - Check if we are accessing RAM or RTC registers.
        if (m_ramBank <= 0x07)
        {
            // - Writing to external RAM bank.
            std::size_t offset = 
                static_cast<std::size_t>(m_ramBank) * 0x2000 + 
                (relativeAddress & 0x1FFF);
            if (offset >= m_extram.size())
            {
                return 0xFF;
            }
            m_extram[offset] = value;
            return value;
        }
        else if (m_ramBank >= 0x08 && m_ramBank <= 0x0C)
        {
            if (m_hasTimer == false)
            {
                // - If the cartridge does not have a real-time clock, return
                //   an open-bus value.
                return 0xFF;
            }

            // - Writing to RTC register.
            switch (m_ramBank)
            {
                case 0x08: m_rtcSeconds = value & 0x3F; break; // 0-59
                case 0x09: m_rtcMinutes = value & 0x3F; break; // 0-59
                case 0x0A: m_rtcHours = value & 0x1F; break;   // 0-23
                case 0x0B: m_rtcDayLow = value; break;
                case 0x0C: m_rtcDayHigh = value & 0xC1; break; // bits 0,6,7
                default: break;
            }
            return value;
        }

        // - Invalid bank selection.
        return 0xFF;
    }

    auto MBC3Cartridge::updateRTC () noexcept -> void
    {
        // - If the cartridge does not have a real-time clock, do nothing.
        if (m_hasTimer == false)
            { return; }

        // - If bit 6 of the day high register is set, the clock is halted.
        //   Do not update the RTC registers in this case.
        if ((m_rtcDayHigh & 0b01000000) != 0)
            { return; }

        // - Calculate the elapsed time since the last update.
        const auto now = std::chrono::system_clock::now();
        const auto elapsed = now - m_lastUpdateTime;

        // - Convert elapsed time to total seconds.
        const auto totalElapsedSeconds =
            std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

        // - If no time has elapsed, do nothing.
        if (totalElapsedSeconds == 0)
            { return; }

        // - Update the RTC registers by adding the elapsed time.
        //   Start with seconds and carry over to minutes, hours, and days.

        // - Add seconds (0-59 range)
        std::uint32_t seconds = m_rtcSeconds + (totalElapsedSeconds % 60);
        std::uint32_t carryMinutes = seconds / 60;
        m_rtcSeconds = seconds % 60;

        // - Add minutes (0-59 range)  
        std::uint32_t minutes = m_rtcMinutes + (carryMinutes % 60);
        std::uint32_t carryHours = minutes / 60;
        m_rtcMinutes = minutes % 60;

        // - Add hours (0-23 range)
        std::uint32_t hours = m_rtcHours + (carryHours % 24);
        std::uint32_t carryDays = hours / 24;
        m_rtcHours = hours % 24;

        // - Add days (9-bit counter: 0-511)
        std::uint32_t currentDayCounter = 
            (static_cast<std::uint32_t>(m_rtcDayHigh & 0x01) << 8) | m_rtcDayLow;
        std::uint32_t newDayCounter = currentDayCounter + carryDays;

        // - Check for day counter overflow (512 days)
        if (newDayCounter >= 512)
        {
            // - Set the carry bit (bit 7 of day high register)
            m_rtcDayHigh |= 0x80;
            // - Wrap the day counter
            newDayCounter %= 512;
        }

        // - Update the day counter registers
        m_rtcDayLow = newDayCounter & 0xFF;
        m_rtcDayHigh = (m_rtcDayHigh & 0xFE) | ((newDayCounter >> 8) & 0x01);

        // - Update the last update time
        m_lastUpdateTime = now;
    }

}

/* Private Methods ************************************************************/

namespace gb
{

    auto MBC3Cartridge::validateByType () const noexcept -> Result<void>
    {
        // - Validate cartridge type.
        switch (m_header.cartridgeType)
        {
            case CART_MBC3:
            case CART_MBC3_RAM:
            case CART_MBC3_RAM_BATTERY:
            case CART_MBC3_TIMER_BATTERY:
            case CART_MBC3_TIMER_RAM_BATTERY:
                break;

            default:
                return error(
                    "Invalid cartridge type for 'MBC3Cartridge': '0x{:02X}'",
                    m_header.cartridgeType
                );
        }

        // - Validate ROM size.
        //   - MBC3 supports ROM sizes from 64 KB to 2 MB.
        if (
            m_header.romSize < 0x01 || // 64 KB
            m_header.romSize > 0x06    // 2 MB
        )
        {
            return error(
                "Invalid ROM size for 'MBC3Cartridge': '0x{:02X}'",
                m_header.romSize
            );
        }

        // - Validate RAM size.
        //   - `CART_MBC3` cartridges must have 0 KB RAM.
        //   - Otherwise, RAM size can be 0 KB, 8 KB or 32 KB.
        if (m_header.cartridgeType == CART_MBC3 && m_header.ramSize != 0x00)
        {
            return error(
                "Invalid RAM size for 'MBC3Cartridge' of type '0x{:02X}': '0x{:02X}'",
                m_header.cartridgeType,
                m_header.ramSize
            );
        }
        else if (
            m_header.ramSize != 0x00 && // No RAM
            m_header.ramSize != 0x02 && // 8 KB
            m_header.ramSize != 0x03    // 32 KB
        )
        {
            return error(
                "Invalid RAM size for 'MBC3Cartridge': '0x{:02X}'",
                m_header.ramSize
            );
        }

        return {};
    }

    auto MBC3Cartridge::finalize (
        std::fstream& file
    ) noexcept -> Result<void>
    {
        // - Allocate and load ROM data.
        const auto romSize = getROMSize();
        m_rom.resize(romSize);
        file.seekg(0);
        file.read(
            reinterpret_cast<char*>(m_rom.data()),
            static_cast<std::streamsize>(romSize)
        );
        if (file.good() == false)
        {
            return error("Failed to read ROM data from file.");
        }

        // - Allocate external RAM, if present.
        const auto ramSize = getRAMSize();
        if (ramSize > 0)
        {
            m_extram.resize(ramSize, 0x00);
        }

        // - Indicate whether the cartridge has a battery.
        m_hasBattery = (
            m_header.cartridgeType == CART_MBC3_RAM_BATTERY ||
            m_header.cartridgeType == CART_MBC3_TIMER_BATTERY ||
            m_header.cartridgeType == CART_MBC3_TIMER_RAM_BATTERY
        );

        // - Indicate whether the cartridge has a real-time clock.
        m_hasTimer = (
            m_header.cartridgeType == CART_MBC3_TIMER_BATTERY ||
            m_header.cartridgeType == CART_MBC3_TIMER_RAM_BATTERY
        );

        // - Initialize the RTC update timestamp.
        if (m_hasTimer)
        {
            m_lastUpdateTime = std::chrono::system_clock::now();
        }

        return {};
    }

}
