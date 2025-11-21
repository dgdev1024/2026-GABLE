/**
 * @file    GB/Memory.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains declarations for the Game Boy Emulator's random-access
 *          memory (RAM) component class.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace gb
{
    /**
     * @brief   Defines the total size of the Game Boy's working RAM (WRAM) in
     *          bytes.
     */
    constexpr std::size_t WRAM_TOTAL_SIZE = 0x8000;

    /**
     * @brief   Defines the size of the Game Boy's high RAM (HRAM) in bytes.
     */
    constexpr std::size_t HRAM_TOTAL_SIZE = 0x7F;
}

/* Public Unions and Structures ***********************************************/

namespace gb
{

    /**
     * @brief   Defines a union representing the Game Boy Color's `SVBK` hardware
     *          register, which is used to select the working RAM bank.
     */
    union RegisterSVBK final
    {
        struct
        {
            std::uint8_t wramBank   : 3;    /** @brief Bits 0-2: WRAM Bank */
            std::uint8_t            : 5;
        };

        std::uint8_t raw { 0x01 };
    };

}

/* Public Classes *************************************************************/

namespace gb
{

    /**
     * @brief   Forward declaration of the @a `SystemContext` class.
     */
    class SystemContext;

    /**
     * @brief   Defines a class representing the Game Boy Emulator's internal,
     *          general-purpose random-access memory (RAM) component.
     * 
     * This component is responsible for managing the Game Boy's working RAM
     * (WRAM) and high RAM (HRAM) areas, and the `SVBK` register for WRAM bank
     * selection.
     */
    class GB_API Memory final
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Constructs a new `Memory` instance associated with the
         *          specified system context.
         * 
         * @param   parentContext   A reference to the parent system context.
         */
        explicit Memory (
            SystemContext& parentContext
        ) noexcept;

        /**
         * @brief   Initializes (or resets) the memory component, clearing its
         *          RAM contents and resetting hardware registers to their
         *          default, power-on states.
         */
        auto initialize () -> void;

    public: /* Public Methods - Memory Access *********************************/

        /**
         * @brief   Reads a byte from working RAM (WRAM) at the specified
         *          address, applying the provided access rules.
         * 
         * @param   relativeAddress The address from which to read the byte. Must
         *                          be between `0x0000` and `0x1FFF`.
         * 
         * @return  If successful, returns the byte read from the specified
         *          address.
         *          If the address is out of bounds, returns an open-bus value of
         *          `0xFF`.
         */
        auto readWRAM (
            const std::uint16_t relativeAddress
        ) noexcept -> std::uint8_t;

        /**
         * @brief   Writes a byte to working RAM (WRAM) at the specified
         *          address, applying the provided access rules.
         * 
         * @param   relativeAddress The address to which to write the byte. Must
         *                          be between `0x0000` and `0x1FFF`.
         * @param   value           The byte value to write.
         * 
         * @return  If successful, returns the actual byte written to the specified
         *          address, which may differ from the input value.
         *          If the address is out of bounds, returns an open-bus value of
         *          the address is out of bounds, returns an open-bus value of
         *          `0xFF`.
         */
        auto writeWRAM (
            const std::uint16_t relativeAddress,
            const std::uint8_t  value
        ) noexcept -> std::uint8_t;

        /**
         * @brief   Reads a byte from high RAM (HRAM) at the specified address,
         *          applying the provided access rules.
         * 
         * @param   relativeAddress The address from which to read the byte. Must
         *                          be between `0x0000` and `0x007E`.
         * 
         * @return  If successful, returns the byte read from the specified
         *          address.
         *          If the address is out of bounds, returns an open-bus value of
         *          `0xFF`.
         */
        auto readHRAM (
            const std::uint16_t relativeAddress
        ) noexcept -> std::uint8_t;

        /**
         * @brief   Writes a byte to high RAM (HRAM) at the specified address,
         *          applying the provided access rules.
         * 
         * @param   relativeAddress The address to which to write the byte. Must
         *                          be between `0x0000` and `0x007E`.
         * @param   value           The byte value to write.
         * 
         * @return  If successful, returns the actual byte written to the specified
         *          address, which may differ from the input value.
         *          If the address is out of bounds, returns an open-bus value 
         *          of `0xFF`.
         */
        auto writeHRAM (
            const std::uint16_t relativeAddress,
            const std::uint8_t  value
        ) noexcept -> std::uint8_t;

    public: /* Public Methods - Hardware Register Access **********************/
        
        /**
         * @brief   Reads the value of the `SVBK` hardware register, which
         *          controls the working RAM (WRAM) bank selection.
         * 
         * @return  The current value of the `SVBK` register.
         */
        auto readSVBK () noexcept -> std::uint8_t;

        /**
         * @brief   Writes a value to the `SVBK` hardware register, which
         *          controls the working RAM (WRAM) bank selection.
         * 
         * @param   value   The byte value to write to the `SVBK` register.
         * 
         * @return  The actual byte written to the `SVBK` register, which may
         *          differ from the input value.
         */
        auto writeSVBK (
            const std::uint8_t value
        ) noexcept -> std::uint8_t;

    private: /* Private Members ***********************************************/

        /**
         * @brief   A handle to the memory component's parent system context.
         */
        SystemContext& m_parentContext;

    private: /* Private Members - Memory **************************************/

        /**
         * @brief   Working RAM (WRAM) storage.
         */
        std::array<std::uint8_t, WRAM_TOTAL_SIZE> m_wram {};

        /**
         * @brief   High RAM (HRAM) storage.
         */
        std::array<std::uint8_t, HRAM_TOTAL_SIZE> m_hram {};

    private: /* Private Members - Hardware Registers **************************/

        /**
         * @brief   SVBK hardware register for WRAM bank selection.
         */
        RegisterSVBK m_registerSVBK {};

    };

}
