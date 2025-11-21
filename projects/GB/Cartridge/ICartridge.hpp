/**
 * @file    GB/Cartridge/ICartridge.hpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains definitions for the Game Boy Emulator's base cartridge
 *          interface.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Common.hpp>

/* Public Constants and Enumerations ******************************************/

namespace gb
{

    /**
     * @brief   Enumerates validate cartridge type bytes which can be found in
     *          the cartridge header. These bytes identify the type of cartridge
     *          hardware used.
     */
    enum CartridgeType : std::uint8_t
    {
        CART_BASIC                          = 0x00,
        CART_MBC1                           = 0x01,
        CART_MBC1_RAM                       = 0x02,
        CART_MBC1_RAM_BATTERY               = 0x03,
        CART_MBC2                           = 0x05,
        CART_MBC2_BATTERY                   = 0x06,
        CART_BASIC_RAM                      = 0x08,
        CART_BASIC_RAM_BATTERY              = 0x09,
        CART_MMM01                          = 0x0B,
        CART_MMM01_RAM                      = 0x0C,
        CART_MMM01_RAM_BATTERY              = 0x0D,
        CART_MBC3_TIMER_BATTERY             = 0x0F,
        CART_MBC3_TIMER_RAM_BATTERY         = 0x10,
        CART_MBC3                           = 0x11,
        CART_MBC3_RAM                       = 0x12,
        CART_MBC3_RAM_BATTERY               = 0x13,
        CART_MBC5                           = 0x19,
        CART_MBC5_RAM                       = 0x1A,
        CART_MBC5_RAM_BATTERY               = 0x1B,
        CART_MBC5_RUMBLE                    = 0x1C,
        CART_MBC5_RUMBLE_RAM                = 0x1D,
        CART_MBC5_RUMBLE_RAM_BATTERY        = 0x1E,
        CART_MBC6                           = 0x20,
        CART_MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
        CART_POCKET_CAMERA                  = 0xFC,
        CART_BANDAI_TAMA5                   = 0xFD,
        CART_HUDSON_HUC3                    = 0xFE,
        CART_HUDSON_HUC1_RAM_BATTERY        = 0xFF
    };

}

/* Public Unions and Structures ***********************************************/

namespace gb
{

    /**
     * @brief   Defines a structure representing the header information of a
     *          Game Boy cartridge.
     */
    struct GB_API CartridgeHeader final
    {
        std::uint8_t    entryPoint[4];            /** @brief Cartridge entry point. */
        std::uint8_t    nintendoLogo[48];         /** @brief Nintendo logo data. */
        char            title[15];                /** @brief Cartridge title (padded with 0x00). */
        std::uint8_t    cgbFlag;                  /** @brief Game Boy Color support flag. */
        std::uint8_t    newLicenseeCode[2];       /** @brief New licensee code. */
        std::uint8_t    sgbFlag;                  /** @brief Super Game Boy support flag. */
        std::uint8_t    cartridgeType;            /** @brief Cartridge type. */
        std::uint8_t    romSize;                  /** @brief ROM size. */
        std::uint8_t    ramSize;                  /** @brief External RAM size. */
        std::uint8_t    destinationCode;          /** @brief Destination code. */
        std::uint8_t    oldLicenseeCode;          /** @brief Old licensee code. */
        std::uint8_t    maskROMVersion;           /** @brief Mask ROM version number. */
        std::uint8_t    headerChecksum;           /** @brief Header checksum. */
        std::uint16_t   globalChecksum;           /** @brief Global checksum. */
    };

}

/* Public Classes *************************************************************/

namespace gb
{

    /**
     * @brief   Defines a class representing the base interface for all Game Boy
     *          cartridges.
     */
    class GB_API ICartridge
    {
    public: /* Public Methods *************************************************/

        /**
         * @brief   Virtual destructor.
         */
        virtual ~ICartridge () noexcept = default;

        /**
         * @brief   Factory method to create an instance of a cartridge from a
         *          ROM file at the specified path.
         * 
         * @param   path    The path to the ROM file.
         * 
         * @return  On success, returns a shared pointer to the created
         *          `ICartridge` instance.
         *          On failure, returns an error message.
         */
        static auto make (
            const std::filesystem::path& path
        ) noexcept -> Result<std::shared_ptr<ICartridge>>;

        /**
         * @brief   Reads a byte from the cartridge's read-only memory (ROM)
         *          at the specified address.
         * 
         * @param   relativeAddress The address from which to read the byte. Must
         *                          be between `0x0000` and `0x7FFF`.
         * 
         * @return  The byte read from the specified address.
         */
        virtual auto readROM (
            const std::uint16_t relativeAddress
        ) noexcept -> std::uint8_t = 0;

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
        virtual auto writeROM (
            const std::uint16_t relativeAddress,
            const std::uint8_t  value
        ) noexcept -> std::uint8_t = 0;

        /**
         * @brief   Reads a byte from the cartridge's external random-access
         *          memory (RAM) at the specified address.
         * 
         * @param   relativeAddress The address from which to read the byte. Must
         *                          be between `0x0000` and `0x1FFF`.
         * 
         * @return  The byte read from the specified address.
         */
        virtual auto readExternalRAM (
            const std::uint16_t relativeAddress
        ) noexcept -> std::uint8_t = 0;

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
        virtual auto writeExternalRAM (
            const std::uint16_t relativeAddress,
            const std::uint8_t  value
        ) noexcept -> std::uint8_t = 0;

        /**
         * @brief   Loads the cartridge's external random-access memory (RAM) data
         *          from a file at the specified path.
         * 
         * @param   path                The path to the RAM data file to load.
         * @param   requireBattery      Indicates whether to require the cartridge
         *                              to have a battery in order to load the
         *                              RAM data.
         * 
         * @return  On success, returns `true`.
         *          If no RAM data is loaded (no RAM, or file not found), returns `false`.
         *          On failure, returns an error message.
         */
        auto loadExternalRAM (
            const std::filesystem::path&    path,
            bool                            requireBattery = true
        ) noexcept -> Result<bool>;

        /**
         * @brief   Saves the cartridge's external random-access memory (RAM) data
         *          to a file at the specified path.
         * 
         * @param   path                The path to the RAM data file to save.
         * @param   requireBattery      Indicates whether to require the cartridge
         *                              to have a battery in order to save the
         *                              RAM data.
         * 
         * @return  On success, returns `true`.
         *          If no RAM data is saved (no RAM), returns `false`.
         *          On failure, returns an error message.
         */
        auto saveExternalRAM (
            const std::filesystem::path&    path,
            bool                            requireBattery = true
        ) noexcept -> Result<bool>;

        /**
         * @brief   Gets this cartridge's header information.
         * 
         * @return  A handle to the cartridge's header information structure.
         */
        inline auto getHeader () const noexcept -> const CartridgeHeader&
            { return m_header; }

        /**
         * @brief   Gets the size of the cartridge's read-only memory (ROM) in
         *          bytes.
         *
         * @return  If the cartridge header's ROM size byte is valid, returns
         *          the size of the ROM in bytes. Otherwise, returns zero.
         */
        inline constexpr auto getROMSize () const noexcept -> std::size_t
        {
            switch (m_header.romSize)
            {
                case 0x00: return 1024 * 16 * 2;    // 32 KB
                case 0x01: return 1024 * 16 * 4;    // 64 KB
                case 0x02: return 1024 * 16 * 8;    // 128 KB
                case 0x03: return 1024 * 16 * 16;   // 256 KB
                case 0x04: return 1024 * 16 * 32;   // 512 KB
                case 0x05: return 1024 * 16 * 64;   // 1 MB
                case 0x06: return 1024 * 16 * 128;  // 2 MB
                case 0x07: return 1024 * 16 * 256;  // 4 MB
                case 0x08: return 1024 * 16 * 512;  // 8 MB
                case 0x52: return 1024 * 16 * 72;   // 1.1 MB
                case 0x53: return 1024 * 16 * 80;   // 1.2 MB
                case 0x54: return 1024 * 16 * 96;   // 1.5 MB
                default: return 0;
            }
        }

        /**
         * @brief   Gets the size of the cartridge's external random-access
         *          memory (RAM) in bytes.
         *
         * @return  If the cartridge header's RAM size byte is valid, returns
         *          the size of the external RAM in bytes. If not, or if the
         *          cartridge has no external RAM, returns zero.
         */
        inline constexpr auto getRAMSize () const noexcept -> std::size_t
        {
            switch (m_header.ramSize)
            {
                case 0x00: return 0;                 // No RAM
                case 0x01: return 1024 * 2;          // 2 KB
                case 0x02: return 1024 * 8;          // 8 KB
                case 0x03: return 1024 * 32;         // 32 KB (4 banks of 8 KB each)
                case 0x04: return 1024 * 128;        // 128 KB (16 banks of 8 KB each)
                case 0x05: return 1024 * 64;         // 64 KB (8 banks of 8 KB each)
                default: return 0;
            }
        }

        /**
         * @brief   Determines if the cartridge supports the enhanced features
         *          of the Game Boy Color (CGB).
         * 
         * @return  If the cartridge supports CGB features, returns `true`.
         *          Otherwise, returns `false`.
         */
        inline virtual constexpr auto supportsCGB () const noexcept -> bool
            { return (m_header.cgbFlag & 0x80) != 0; }

        /**
         * @brief   Determines if the cartridge requires the enhanced features
         *          of the Game Boy Color (CGB).
         *
         * @return  If the cartridge requires CGB features, returns `true`.
         *          Otherwise, returns `false`.
         */
        inline virtual constexpr auto requiresCGB () const noexcept -> bool
            { return (m_header.cgbFlag & 0xC0) == 0xC0; }

    protected: /* Protected Methods *******************************************/

        explicit ICartridge (
            const CartridgeHeader& header
        ) noexcept :
            m_header    { header }
        {}

        /**
         * @brief   Performs basic, non-type-specific validation of the cartridge's
         *          header data.
         * 
         * @return  On success, returns `void`. 
         *          On failure, returns an error message.
         */
        auto validate () const noexcept -> Result<void>;

        /**
         * @brief   Performs type-specific validation of the cartridge's header
         *          data.
         * 
         * @return  On success, returns `void`. 
         *          On failure, returns an error message.
         */
        virtual auto validateByType () const noexcept -> Result<void> = 0;

        /**
         * @brief   Finalizes the cartridge by allocating necessary memory and
         *          loading the full ROM data from the provided file stream.
         * 
         * @param   file        The file stream from which to read the ROM data.
         * 
         * @return  On success, returns `void`. 
         *          On failure, returns an error message.
         */
        virtual auto finalize (
            std::fstream& file
        ) noexcept -> Result<void> = 0;

    protected: /* Protected Members *******************************************/

        /**
         * @brief   The cartridge header information.
         */
        CartridgeHeader m_header;

        /**
         * @brief   Contains the cartridge's read-only memory (ROM) data.
         */
        std::vector<std::uint8_t> m_rom;

        /**
         * @brief   Contains the cartridge's external random-access memory
         *          (RAM) data.
         */
        std::vector<std::uint8_t> m_extram;

        /**
         * @brief   Indicates whether the cartridge has a battery for persisting
         *          the cartridge's external RAM data, and possibly other
         *          external hardware state.
         */
        bool m_hasBattery { false };

    };

}
