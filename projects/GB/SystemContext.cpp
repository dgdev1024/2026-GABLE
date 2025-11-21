/**
 * @file    GB/SystemContext.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains implementations for the Game Boy Emulator's primary system
 *          context class.
 */

/* Private Includes ***********************************************************/

#include <GB/Cartridge/MBC3.hpp>
#include <GB/SystemContext.hpp>

/* Public Methods *************************************************************/

namespace gb
{

    SystemContext::SystemContext () :
        m_memory { *this }
    {
        initialize();
    }

    auto SystemContext::initialize () -> void
    {

    }

    auto SystemContext::attachCartridge (
        const std::shared_ptr<ICartridge>& cartridge
    ) -> void
    {
        m_cartridge = cartridge;
        initialize();
    }

    auto SystemContext::isCGBMode () const noexcept -> bool
    {
        if (m_cartridge == nullptr)
            { return false; }

        return m_cartridge->supportsCGB();
    }

    auto SystemContext::readFromBus (
        std::uint16_t       address,
        const AccessRules&  rules
    ) -> std::uint8_t
    {
        if (rules.updateRTC == true && m_cartridge != nullptr)
        {
            if (auto mbc3 = std::static_pointer_cast<MBC3Cartridge>(m_cartridge))
                { mbc3->updateRTC();}
        }

        std::uint8_t result = 0xFF;

        if (m_cartridge != nullptr && address <= ROMX_END)
        {
            result = m_cartridge->readROM(address);
        }
        else if (m_cartridge != nullptr && address >= EXTRAM_START && 
            address <= EXTRAM_END)
        {
            result = m_cartridge->readExternalRAM(address - EXTRAM_START);
        }
        else if (address >= WRAM0_START && address <= WRAMX_END)
        {
            result = m_memory.readWRAM(address - WRAM0_START);
        }
        else if (address >= HRAM_START && address <= HRAM_END)
        {
            result = m_memory.readHRAM(address - HRAM_START);
        }
        else
        {
            switch (address)
            {
                case PortRegister::PR_SVBK:     
                    result = m_memory.readSVBK();   
                    break;
                default: break;
            }
        }

        if (m_busReadCallback != nullptr)
        {
            m_busReadCallback(address, result);
        }

        return result;
    }

    auto SystemContext::writeToBus (
        std::uint16_t       address,
        std::uint8_t        value,
        const AccessRules&  rules
    ) -> std::uint8_t
    {
        if (rules.updateRTC == true && m_cartridge != nullptr)
        {
            if (auto mbc3 = std::static_pointer_cast<MBC3Cartridge>(m_cartridge))
                { mbc3->updateRTC();}
        }

        std::uint8_t result = 0xFF;

        if (m_cartridge != nullptr && address <= ROMX_END)
        {
            result = m_cartridge->writeROM(address, value);
        }
        else if (m_cartridge != nullptr && address >= EXTRAM_START && 
            address <= EXTRAM_END)
        {
            result = m_cartridge->writeExternalRAM(
                address - EXTRAM_START,
                value
            );
        }
        else if (address >= WRAM0_START && address <= WRAMX_END)
        {
            result = m_memory.writeWRAM(address - WRAM0_START, value);
        }
        else if (address >= HRAM_START && address <= HRAM_END)
        {
            result = m_memory.writeHRAM(address - HRAM_START, value);
        }
        else
        {
            switch (address)
            {
                case PortRegister::PR_SVBK:
                    result = m_memory.writeSVBK(value);
                    break;

                default: break;
            }
        }

        if (m_busWriteCallback != nullptr)
        {
            m_busWriteCallback(address, value, result);
        }

        return result;
    }

}

/* Private Methods ************************************************************/

namespace gb
{

    auto SystemContext::tick () -> Result<void>
    {
        return {};
    }

}
