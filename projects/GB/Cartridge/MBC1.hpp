/**
 * @file    GB/Cartridge/MBC1.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's MBC1 cartridge
 *          type.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Cartridge/ICartridge.hpp>

/* Public Classes *************************************************************/

namespace gb
{

    /**
     * @brief   Defines a class representing an MBC1 Game Boy cartridge with
     *          memory bank controller for ROM and RAM banking.
     */
    class GB_API MBC1Cartridge final : public ICartridge
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs an `MBC1Cartridge` instance with the specified
         *          cartridge header.
         * 
         * @param   header  The cartridge header information.
         */
        explicit MBC1Cartridge (
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
         * @brief   RAM enable flag.
         */
        bool m_ramEnable { false };

        /**
         * @brief   ROM bank register (5 bits).
         */
        std::uint8_t m_romBank { 1 };

        /**
         * @brief   RAM bank or upper ROM bank register (2 bits).
         */
        std::uint8_t m_ramBank { 0 };

        /**
         * @brief   Banking mode (1 bit).
         */
        std::uint8_t m_mode { 0 };

    };

}
