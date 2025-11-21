/**
 * @file    GB/Cartridge/Basic.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's basic cartridge
 *          type.
 */

/* Public Includes ************************************************************/

#include <GB/Cartridge/Basic.hpp>

/* Public Methods *************************************************************/

namespace gb
{

    auto BasicCartridge::readROM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {
        if (relativeAddress >= m_rom.size())
        {
            return 0xFF;
        }

        return m_rom[relativeAddress];
    }

    auto BasicCartridge::writeROM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        // Basic cartridges do not support writing to ROM.
        return 0xFF;
    }

    auto BasicCartridge::readExternalRAM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {
        if (relativeAddress >= m_extram.size())
        {
            return 0xFF;
        }

        return m_extram[relativeAddress];
    }

    auto BasicCartridge::writeExternalRAM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        if (relativeAddress >= m_extram.size())
        {
            return 0xFF;
        }

        m_extram[relativeAddress] = value;
        return value;
    }

}

/* Private Methods ************************************************************/

namespace gb
{

    auto BasicCartridge::validateByType () const noexcept -> Result<void>
    {
        // - Validate cartridge type.
        switch (m_header.cartridgeType)
        {
            case CART_BASIC:
            case CART_BASIC_RAM:
            case CART_BASIC_RAM_BATTERY:
                break;

            default:
                return error(
                    "Invalid cartridge type for 'BasicCartridge': '0x{:02X}'",
                    m_header.cartridgeType
                );
        }

        // - Validate ROM size.
        //   - Must be exactly 32 KB for basic cartridges.
        if (m_header.romSize != 0x00) // 32 KB
        {
            return error(
                "Invalid ROM size for 'BasicCartridge': '0x{:02X}'",
                m_header.romSize
            );
        }

        // - Validate RAM size.
        //   - `CART_BASIC` cartridges must have 0 KB RAM.
        //   - `CART_BASIC_RAM` and `CART_BASIC_RAM_BATTERY` cartridges must have
        //     0 or 8 KB RAM.
        if (
            (
                m_header.cartridgeType == CART_BASIC &&
                m_header.ramSize != 0x00 // No RAM
            ) ||
            (
                m_header.ramSize != 0x00 && // No RAM
                m_header.ramSize != 0x02    // 8 KB
            )
        )
        {
            return error(
                "Invalid RAM size for 'BasicCartridge': '0x{:02X}'",
                m_header.ramSize
            );
        }

        return {};
    }

    auto BasicCartridge::finalize (
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
            m_header.cartridgeType == CART_BASIC_RAM_BATTERY
        );

        return {};
    }

}
