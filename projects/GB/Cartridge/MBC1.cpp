/**
 * @file    GB/Cartridge/MBC1.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's MBC1 cartridge
 *          type.
 */

/* Public Includes ************************************************************/

#include <GB/Cartridge/MBC1.hpp>

/* Public Methods *************************************************************/

namespace gb
{

    auto MBC1Cartridge::readROM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {

        // - Get the size of this ROM. Are we working with a large ROM, larger
        //   than 512 KB?
        const bool isLargeROM = getROMSize() > 512 * 1024;

        // - Determine the ROM bank to read from, based on the provided relative
        //   address, the banking mode, and the current bank registers.
        std::uint8_t bank = 0;
        if (relativeAddress < ROM0_SIZE)
        {
            if (m_mode == 0 || isLargeROM == false)
                { bank = 0; }
            else
                { bank = m_ramBank << 5; }
        }
        else
        {
            bank = m_romBank;
            if (isLargeROM == true)
                { bank |= m_ramBank << 5; }
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

    auto MBC1Cartridge::writeROM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        // - Handle a write to one of the MBC1 control registers, based on the
        //   address range specified.
        //
        // `$0000` - `$1FFF`: RAM Enable
        if (relativeAddress < 0x2000)
        {
            // - Writing any value with lower nibble `0x0A` enables RAM;
            //   writing any other value disables RAM.
            m_ramEnable = ((value & 0x0F) == 0x0A);
        }

        // `$2000` - `$3FFF`: ROM Bank Number (5 bits)
        else if (relativeAddress < ROM0_SIZE)
        {
            // - Only the lower 5 bits are used for the ROM bank number.
            // - If the resultant ROM bank number is `0`, it is `1` instead.
            m_romBank = value & 0x1F;
            if (m_romBank == 0)
            {
                m_romBank = 1;
            }
        }

        // `$4000` - `$5FFF`: 
        // - If in ROM banking mode: ROM Bank Number bits 5-6
        // - If in RAM banking mode: RAM Bank Number (2 bits)
        else if (relativeAddress < 0x6000)
        {
            // - If in ROM banking mode, the upper two bits of the ROM bank
            //   number are set in the RAM bank register.
            // - If in RAM banking mode, the RAM bank number is set.
            m_ramBank = value & 0x03;
        }

        // `$6000` - `$7FFF`: Banking Mode Select
        else if (relativeAddress < 0x8000)
        {
            // - If `0`, then select ROM banking mode.
            // - If `1`, then select RAM banking mode.
            m_mode = value & 0x01;
        }

        // - Nothing is actually written to ROM, so return an open-bus value.
        return 0xFF;
    }

    auto MBC1Cartridge::readExternalRAM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {
        // - Only read from RAM if it is enabled.
        if (m_ramEnable == false)
        {
            return 0xFF;
        }

        // - Check the ROM's size. Are we working with a large ROM, larger
        //   than 512 KB?
        // - If we are, then ROM banking mode is always assumed. Limit to RAM
        //   bank `0`.
        const bool isLargeROM = getROMSize() > 512 * 1024;
        std::uint8_t ramBank = (m_mode == 0 || isLargeROM == true) ? 
            0 : m_ramBank;

        // - Calculate the actual offset into the external RAM data.
        //   - If the offset is out of bounds, return an open-bus value.
        std::size_t offset = 
            static_cast<std::size_t>(ramBank) * 0x2000 + (relativeAddress & 0x1FFF);
        if (offset >= m_extram.size())
        {
            return 0xFF;
        }

        return m_extram[offset];
    }

    auto MBC1Cartridge::writeExternalRAM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        // - Only write to RAM if it is enabled.
        if (m_ramEnable == false)
        {
            return 0xFF;
        }

        // - Check the ROM's size. Are we working with a large ROM, larger
        //   than 512 KB?
        // - If we are, then ROM banking mode is always assumed. Limit to RAM
        //   bank `0`.
        const bool isLargeROM = getROMSize() > 512 * 1024;
        std::uint8_t ramBank = (m_mode == 0 || isLargeROM == true) ? 
            0 : m_ramBank;

        // - Calculate the actual offset into the external RAM data.
        //   - If the offset is out of bounds, return an open-bus value.
        std::size_t offset = 
            static_cast<std::size_t>(ramBank) * 0x2000 + (relativeAddress & 0x1FFF);
        if (offset >= m_extram.size())
        {
            return 0xFF;
        }

        m_extram[offset] = value;
        return value;
    }

}

/* Private Methods ************************************************************/

namespace gb
{

    auto MBC1Cartridge::validateByType () const noexcept -> Result<void>
    {
        // - Validate cartridge type.
        switch (m_header.cartridgeType)
        {
            case CART_MBC1:
            case CART_MBC1_RAM:
            case CART_MBC1_RAM_BATTERY:
                break;

            default:
                return error(
                    "Invalid cartridge type for 'MBC1Cartridge': '0x{:02X}'",
                    m_header.cartridgeType
                );
        }

        // - Validate ROM size.
        //   - Must be between 32 KB and 2 MB for MBC1 cartridges.
        if (m_header.romSize > 0x06) // 2 MB
        {
            return error(
                "Invalid ROM size for 'MBC1Cartridge': '0x{:02X}'",
                m_header.romSize
            );
        }

        // - Validate RAM size.
        //   - For `CART_MBC1` cartridges, must be 0 KB.
        //   - For `CART_MBC1_RAM` and `CART_MBC1_RAM_BATTERY` cartridges:
        //     - If ROM <= 512 KB, can be 0 KB, 8 KB or 32 KB.
        //     - If ROM > 512 KB, must be 0 KB or 8 KB.
        if (m_header.cartridgeType == CART_MBC1 && m_header.ramSize != 0x00)
        {
            return error(
                "Invalid RAM size for 'MBC1Cartridge' of type '0x{:02X}': '0x{:02X}'",
                m_header.cartridgeType,
                m_header.ramSize
            );
        }
        else if (
            m_header.cartridgeType == CART_MBC1_RAM ||
            m_header.cartridgeType == CART_MBC1_RAM_BATTERY
        )
        {
            if (m_header.romSize <= 0x04 && m_header.ramSize > 0x03)
            {
                return error(
                    "Invalid RAM size for 'MBC1Cartridge' of type '0x{:02X}': '0x{:02X}'",
                    m_header.cartridgeType,
                    m_header.ramSize
                );
            }
            else if (m_header.romSize > 0x04 && 
                     m_header.ramSize != 0x00 && 
                     m_header.ramSize != 0x02)
            {
                return error(
                    "Invalid RAM size for 'MBC1Cartridge' of type '0x{:02X}': '0x{:02X}'",
                    m_header.cartridgeType,
                    m_header.ramSize
                );
            }
        }

        return {};
    }

    auto MBC1Cartridge::finalize (
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
            m_header.cartridgeType == CART_MBC1_RAM_BATTERY
        );

        return {};
    }

}
