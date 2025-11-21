/**
 * @file    GB/Memory.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains implementations for the Game Boy Emulator's random-access
 *          memory (RAM) component class.
 */

/* Private Includes ***********************************************************/

#include <GB/SystemContext.hpp>
#include <GB/Memory.hpp>

/* Public Methods *************************************************************/

namespace gb
{

    Memory::Memory (
        SystemContext& parentContext
    ) noexcept :
        m_parentContext { parentContext }
    {}

    auto Memory::initialize () -> void
    {
        // - Initialize Buffers
        std::fill(m_wram.begin(), m_wram.end(), 0x00);
        std::fill(m_hram.begin(), m_hram.end(), 0x00);

        // - Initialize Hardware Registers
        if (m_parentContext.isCGBMode() == true)
        {
            m_registerSVBK.raw = 0xFE;
        }
        else
        {
            m_registerSVBK.raw = 0x00;
        }
    }

}

/* Public Methods - Memory Access *********************************************/

namespace gb
{

    auto Memory::readWRAM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {

        // - Return open-bus if relative address is out of bounds.
        if (relativeAddress > WRAM0_SIZE)
        {
            return 0xFF;
        }

        // - If address is in WRAM0, read directly.
        if (relativeAddress < WRAM0_SIZE)
        {
            return m_wram[relativeAddress];
        }

        // - Address is in switchable WRAM bank.
        //   - If not CGB mode, always use bank 1.
        std::uint8_t bank = 1;
        if (m_parentContext.isCGBMode() == true)
        {
            bank = m_registerSVBK.wramBank;
            if (bank == 0) { bank = 1; }
        }

        std::size_t offset = 
            static_cast<std::size_t>(bank) * WRAM0_SIZE + 
            (relativeAddress - WRAM0_SIZE);
        return m_wram[offset];
    }

    auto Memory::writeWRAM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        // - Return open-bus if relative address is out of bounds.
        if (relativeAddress > WRAM0_SIZE)
        {
            return 0xFF;
        }

        // - If address is in WRAM0, write directly.
        if (relativeAddress < WRAM0_SIZE)
        {
            m_wram[relativeAddress] = value;
            return value;
        }

        // - Address is in switchable WRAM bank.
        //   - If not CGB mode, always use bank 1.
        std::uint8_t bank = 1;
        if (m_parentContext.isCGBMode() == true)
        {
            bank = m_registerSVBK.wramBank;
            if (bank == 0) { bank = 1; }
        }

        std::size_t offset = 
            static_cast<std::size_t>(bank) * WRAM0_SIZE + 
            (relativeAddress - WRAM0_SIZE);
        m_wram[offset] = value;
        return value;
    }

    auto Memory::readHRAM (
        const std::uint16_t relativeAddress
    ) noexcept -> std::uint8_t
    {
        // - Return open-bus if relative address is out of bounds.
        if (relativeAddress > HRAM_END)
        {
            return 0xFF;
        }

        return m_hram[relativeAddress];
    }

    auto Memory::writeHRAM (
        const std::uint16_t relativeAddress,
        const std::uint8_t  value
    ) noexcept -> std::uint8_t
    {
        // - Return open-bus if relative address is out of bounds.
        if (relativeAddress > HRAM_END)
        {
            return 0xFF;
        }

        m_hram[relativeAddress] = value;
        return value;
    }

}

/* Public Methods - Hardware Register Access **********************************/

namespace gb
{

    auto Memory::readSVBK () noexcept -> std::uint8_t
    {
        // - Requires CGB mode.
        if (m_parentContext.isCGBMode() == false)
        {
            return 0xFF;
        }

        // - Read Register
        return
            0b11111000 |                        // Bits 3-7 unused; read `1`. 
            (m_registerSVBK.raw & 0b00000111);  // Bits 0-2 readable.
    }

    auto Memory::writeSVBK (
        const std::uint8_t value
    ) noexcept -> std::uint8_t
    {
        // - Requires CGB mode.
        if (m_parentContext.isCGBMode() == false)
        {
            return 0xFF;
        }

        // - Write Register
        m_registerSVBK.raw =
            0b11111000 |                        // Bits 3-7 unused; write `1`. 
            (value & 0b00000111);               // Bits 0-2 writable.

        return m_registerSVBK.raw;
    }

}
