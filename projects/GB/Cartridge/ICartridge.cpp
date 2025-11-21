/**
 * @file    GB/Cartridge/ICartridge.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-21
 * 
 * @brief   Contains implementations for the Game Boy Emulator's base cartridge
 *          interface.
 */

/* Public Includes ************************************************************/

#include <GB/Cartridge/Basic.hpp>
#include <GB/Cartridge/MBC1.hpp>
#include <GB/Cartridge/MBC3.hpp>

/* Private Constants and Enumerations *****************************************/

namespace gb
{
    static constexpr std::size_t MIN_ROM_SIZE = 32 * 1024; // 32 KB

    static constexpr std::uint8_t NINTENDO_LOGO[48] =
    {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
        0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
        0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
        0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
}

/* Public Methods *************************************************************/

namespace gb
{

    auto ICartridge::make (
        const std::filesystem::path& path
    ) noexcept -> Result<std::shared_ptr<ICartridge>>
    {
        // - Make sure the file exists.
        if (std::filesystem::exists(path) == false)
        {
            return error(
                "Cartridge ROM file does not exist: '{}'.",
                path.string()
            );
        }

        // - Make sure the ROM file is large enough to contain a valid ROM
        //   image (32 KB minimum).
        const auto fileSize = std::filesystem::file_size(path);
        if (fileSize < MIN_ROM_SIZE)
        {
            return error(
                "Cartridge ROM file is too small ({} bytes): '{}'. "
                "Minimum size is {} bytes.",
                fileSize,
                path.string(),
                MIN_ROM_SIZE
            );
        }

        // - Open the ROM file for reading.
        std::fstream file(
            path,
            std::ios::in | std::ios::binary
        );
        if (file.is_open() == false)
        {
            return error(
                "Failed to open cartridge ROM file for reading: '{}'.",
                path.string()
            );
        }

        // - Read the cartridge header.
        CartridgeHeader header;
        file.seekg(0x0100);
        file.read(
            reinterpret_cast<char*>(&header),
            sizeof(CartridgeHeader)
        );
        if (file.good() == false)
        {
            return error(
                "Failed to read cartridge header from ROM file: '{}'.",
                path.string()
            );
        }

        // - Determine the cartridge type and create the appropriate instance.
        std::shared_ptr<ICartridge> cartridge = nullptr;
        switch (header.cartridgeType)
        {
            case CART_BASIC:
            case CART_BASIC_RAM:
            case CART_BASIC_RAM_BATTERY:
                cartridge = std::make_shared<BasicCartridge>(header);
                break;

            case CART_MBC1:
            case CART_MBC1_RAM:
            case CART_MBC1_RAM_BATTERY:
                cartridge = std::make_shared<MBC1Cartridge>(header);
                break;

            case CART_MBC3:
            case CART_MBC3_RAM:
            case CART_MBC3_RAM_BATTERY:
            case CART_MBC3_TIMER_BATTERY:
            case CART_MBC3_TIMER_RAM_BATTERY:
                cartridge = std::make_shared<MBC3Cartridge>(header);
                break;

            default:
                return error(
                    "Unsupported cartridge type: '0x{:02X}'.",
                    header.cartridgeType
                );
        }

        // - Validate, then finalize the cartridge.
        auto validate = cartridge->validate();
        if (validate.has_value() == false)
        {
            return error("Failed to validate ROM file '{}': {}",
                path.string(),
                validate.error()
            );
        }

        auto finalize = cartridge->finalize(file);
        if (finalize.has_value() == false)
        {
            return error("Failed to finalize cartridge from ROM file '{}': {}",
                path.string(),
                finalize.error()
            );
        }

        // - Close the ROM file and return the created cartridge.
        file.close();
        return cartridge;
    }

    auto ICartridge::loadExternalRAM (
        const std::filesystem::path&    path,
        const bool                      requireBattery
    ) noexcept -> Result<bool>
    {

        // - Make sure a path was provided.
        if (path.empty())
        {
            return error("No path specified for loading external RAM data.");
        }

        // - If no external RAM is present, or if the file does not exist, return
        //   `false`.
        const auto ramSize = getRAMSize();
        if (ramSize == 0 || std::filesystem::exists(path) == false)
        {
            return false;
        }

        // - Determine if the cartridge has a battery (if required).
        if (requireBattery == true && m_hasBattery == false)
        {
            return false;
        }

        // - Get the size of the RAM file. It must match the expected size.
        const auto fileSize = std::filesystem::file_size(path);
        if (fileSize != ramSize)
        {
            return error(
                "External RAM file size mismatch: expected {} bytes, "
                "found {} bytes.",
                ramSize,
                fileSize
            );
        }

        // - Open the RAM file for reading.
        std::fstream file(
            path,
            std::ios::in | std::ios::binary
        );
        if (file.is_open() == false)
        {
            return error(
                "Failed to open external RAM file for reading: '{}'.",
                path.string()
            );
        }

        // - Read the RAM data from the file.
        file.read(
            reinterpret_cast<char*>(m_extram.data()),
            static_cast<std::streamsize>(ramSize)
        );
        if (file.good() == false)
        {
            return error(
                "Failed to read external RAM data from file: '{}'.",
                path.string()
            );
        }

        // - Close the file and return success.
        file.close();
        return true;
    }

    auto ICartridge::saveExternalRAM (
        const std::filesystem::path&    path,
        const bool                      requireBattery
    ) noexcept -> Result<bool>
    {
        // - Make sure a path was provided.
        if (path.empty())
        {
            return error("No path specified for saving external RAM data.");
        }

        // - If no external RAM is present, return `false`.
        const auto ramSize = getRAMSize();
        if (ramSize == 0)
        {
            return false;
        }

        // - Determine if the cartridge has a battery (if required).
        if (requireBattery == true && m_hasBattery == false)
        {
            return false;
        }

        // - Open the RAM file for writing.
        std::fstream file(
            path,
            std::ios::out | std::ios::binary | std::ios::trunc
        );
        if (file.is_open() == false)
        {
            return error(
                "Failed to open external RAM file for writing: '{}'.",
                path.string()
            );
        }

        // - Write the RAM data to the file.
        file.write(
            reinterpret_cast<const char*>(m_extram.data()),
            static_cast<std::streamsize>(ramSize)
        );
        if (file.good() == false)
        {
            return error(
                "Failed to write external RAM data to file: '{}'.",
                path.string()
            );
        }

        // - Close the file and return success.
        file.close();
        return true;
    }

}

/* Protected Methods **********************************************************/

namespace gb
{

    auto ICartridge::validate () const noexcept -> Result<void>
    {
        // Validate the Nintendo logo.
        for (std::size_t i = 0; i < sizeof(NINTENDO_LOGO); ++i)
        {
            if (m_header.nintendoLogo[i] != NINTENDO_LOGO[i])
            {
                return error(
                    "Nintendo logo byte mismatch at index #{}", i
                );
            }
        }

        // Validate cartridge type.
        switch (m_header.cartridgeType)
        {
            case CART_BASIC:
            case CART_MBC1:
            case CART_MBC1_RAM:
            case CART_MBC1_RAM_BATTERY:
            case CART_MBC2:
            case CART_MBC2_BATTERY:
            case CART_BASIC_RAM:
            case CART_BASIC_RAM_BATTERY:
            case CART_MMM01:
            case CART_MMM01_RAM:
            case CART_MMM01_RAM_BATTERY:
            case CART_MBC3_TIMER_BATTERY:
            case CART_MBC3_TIMER_RAM_BATTERY:
            case CART_MBC3:
            case CART_MBC3_RAM:
            case CART_MBC3_RAM_BATTERY:
            case CART_MBC5:
            case CART_MBC5_RAM:
            case CART_MBC5_RAM_BATTERY:
            case CART_MBC5_RUMBLE:
            case CART_MBC5_RUMBLE_RAM:
            case CART_MBC5_RUMBLE_RAM_BATTERY:
            case CART_MBC6:
            case CART_MBC7_SENSOR_RUMBLE_RAM_BATTERY:
            case CART_POCKET_CAMERA:
            case CART_BANDAI_TAMA5:
            case CART_HUDSON_HUC3:
            case CART_HUDSON_HUC1_RAM_BATTERY:
                break;

            default:
                return error(
                    "Invalid cartridge type byte: '0x{:02X}'",
                    m_header.cartridgeType
                );
        }

        // - Validate ROM and RAM size.
        if (getROMSize() == 0)
        {
            return error("Invalid ROM size byte: '0x{:02X}'", m_header.romSize);
        }
        else if (m_header.ramSize != 0x00 && getRAMSize() == 0)
        {
            return error("Invalid RAM size byte: '0x{:02X}'", m_header.ramSize);
        }

        // - Validate header checksum.
        std::uint8_t checksum = 0;
        const std::uint8_t* headerBytes = 
            reinterpret_cast<const std::uint8_t*>(&m_header);
        for (std::size_t i = 0x34; i <= 0x4C; ++i)
        {
            checksum = checksum - headerBytes[i] - 1;
        }

        if (checksum != m_header.headerChecksum)
        {
            return error(
                "Header checksum mismatch: calculated '0x{:02X}', "
                "expected '0x{:02X}'",
                checksum,
                m_header.headerChecksum
            );
        }

        // - Additional, type-specific validations may be needed.
        return validateByType();
    }

}
