/**
 * @file    GB/SystemContext.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's primary system
 *          context class.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Cartridge/ICartridge.hpp>
#include <GB/Memory.hpp>

/* Public Classes *************************************************************/

namespace gb
{

    /**
     * @brief   Definss a class representing the main system context for the
     *          Game Boy Emulator.
     */
    class GB_API SystemContext final
    {
        friend class Executive;

    public:

        using BusReadCallback = std::function<void(std::uint16_t, std::uint8_t&)>;
        using BusWriteCallback = std::function<void(std::uint16_t,
            const std::uint8_t&, std::uint8_t&)>;

    public:

        /**
         * @brief   Constructs a new system context instance, initializing
         *          its components to their default, power-on states.
         */
        SystemContext ();
        
        /**
         * @brief   Initializes (or resets) the system context and its
         *          components to their default, power-on states.
         */
        auto initialize () -> void;

        /**
         * @brief   Attaches a cartridge to the system context.
         * 
         * Attaching (or detaching) a cartridge will reset the system context.
         * 
         * @param   cartridge   A shared pointer to the cartridge to attach.
         *                      Pass `nullptr` to detach any currently attached
         *                      cartridge.
         */
        auto attachCartridge (
            const std::shared_ptr<ICartridge>& cartridge
        ) -> void;

        /**
         * @brief   Indicates whether the system is currently operating in
         *          Game Boy Color (CGB) mode.
         * 
         * In the Game Boy Emulator, this is indicated by checking the
         * @a `cgbFlag` byte in the attached cartridge's header.
         * 
         * @return  If a cartridge is attached and its `cgbFlag` indicates CGB
         *          support, returns `true`;
         *          Otherwise, returns `false`.
         */
        auto isCGBMode () const noexcept -> bool;

        /**
         * @brief   Reads a byte from the system bus at the specified address,
         *          applying the provided access rules.
         * 
         * @param   address     The 16-bit address on the system bus from which
         *                      to read the byte.
         * @param   rules       The access rules to apply for this read access.
         *                      If no rules are provided, no access rules will be
         *                      applied.
         * 
         * @return  On success, returns the byte read from the specified address.
         *          If the address is unmapped, the memory at that address is
         *          entirely write-only, or if the read is blocked by the provided
         *          access rules, returns an open-bus value of `0xFF`.
         */
        auto readFromBus (
            std::uint16_t       address,
            const AccessRules&  rules = {}
        ) -> std::uint8_t;

        /**
         * @brief   Writes a byte to the system bus at the specified address,
         *          applying the provided access rules.
         * 
         * @param   address     The 16-bit address on the system bus to which
         *                      to write the byte.
         * @param   value       The byte value to write to the specified address.
         * @param   rules       The access rules to apply for this write access.
         *                      If no rules are provided, no access rules will be
         *                      applied.
         * 
         * @return  On success, returns the byte which was actually written to
         *          the specified address, which may differ from the provided value.
         *          If the address is unmapped, or if the write is blocked by the
         *          provided access rules, returns an open-bus value of `0xFF`.
         */
        auto writeToBus (
            std::uint16_t       address,
            std::uint8_t        value,
            const AccessRules&  rules = {}
        ) -> std::uint8_t;

        /**
         * @brief   Gets a reference to the system context's memory component.
         * 
         * @return  A reference to the system context's memory component.
         */
        inline auto getMemory () noexcept -> Memory&
            { return m_memory; }
        inline auto getMemory () const noexcept -> const Memory&
            { return m_memory; }

        /**
         * @brief   Sets the callback function to be invoked on bus read
         *          operations.
         * 
         * @param   callback    The callback function to set for bus read
         *                      operations.
         */
        inline auto setBusReadCallback (BusReadCallback callback) -> void
            { m_busReadCallback = callback; }

        /**
         * @brief   Sets the callback function to be invoked on bus write
         *          operations.
         * 
         * @param   callback    The callback function to set for bus write
         *                      operations.
         */
        inline auto setBusWriteCallback (BusWriteCallback callback) -> void
            { m_busWriteCallback = callback; }

    private:
        
        /**
         * @brief   Ticks the system context, causing it and its components
         *          to advance their state by one emulation step.
         * 
         * This method is intended to be called only by the Executive class.
         * 
         * @return  On success, returns `void`;
         *          On error, returns a string describing the error that occurred.
         */
        auto tick () -> Result<void>;

    private: /* Private Members ***********************************************/

        BusReadCallback     m_busReadCallback { nullptr };
        BusWriteCallback    m_busWriteCallback { nullptr };

    private: /* Private Members - System Components ***************************/

        std::shared_ptr<ICartridge>    m_cartridge { nullptr };
        Memory                         m_memory;

    };

}
