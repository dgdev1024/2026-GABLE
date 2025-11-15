/**
 * @file    GB/Cartridge.c
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains definitions for the Game Boy Emulator's cartridge
 *          component.
 */

/* Private Includes ***********************************************************/

#include <GB/Cartridge.h>

/* Private Constants and Enumerations *****************************************/

/**
 * @brief   Defines an array of bytes which make up a bitmapped image of the
 *          Nintendo logo. This data is expected to be present in a valid Game
 *          Boy cartridge's header, starting at address `$0104`.
 */
static const uint8_t NINTENDO_LOGO[48] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

/**
 * @brief   Defines an array of strings representing human-readable names
 *          for each valid cartridge type defined in the @a `gbCartridgeType`
 *          enumeration.
 */
static const char* GB_CARTRIDGE_TYPE_STRINGS[] = {
    [GB_CT_BASIC]                           = "ROM Only",
    [GB_CT_BASIC_RAM]                       = "ROM with RAM",
    [GB_CT_BASIC_RAM_BATTERY]               = "ROM with RAM, Battery",
    [GB_CT_MBC1]                            = "MBC1, ROM Only",
    [GB_CT_MBC1_RAM]                        = "MBC1 with RAM",
    [GB_CT_MBC1_RAM_BATTERY]                = "MBC1 with RAM, Battery",
    [GB_CT_MBC2]                            = "MBC2, ROM with built-in RAM",
    [GB_CT_MBC2_BATTERY]                    = "MBC2 with Battery",
    [GB_CT_MBC3]                            = "MBC3, ROM Only",
    [GB_CT_MBC3_RAM]                        = "MBC3 with RAM",
    [GB_CT_MBC3_RAM_BATTERY]                = "MBC3 with RAM, Battery",
    [GB_CT_MBC3_TIMER_BATTERY]              = "MBC3 with Timer, Battery",
    [GB_CT_MBC3_TIMER_RAM_BATTERY]          = "MBC3 with Timer, RAM, Battery",
    [GB_CT_MBC5]                            = "MBC5, ROM Only",
    [GB_CT_MBC5_RAM]                        = "MBC5 with RAM",
    [GB_CT_MBC5_RAM_BATTERY]                = "MBC5 with RAM, Battery",
    [GB_CT_MBC5_RUMBLE]                     = "MBC5 with Rumble",
    [GB_CT_MBC5_RUMBLE_RAM]                 = "MBC5 with Rumble, RAM",
    [GB_CT_MBC5_RUMBLE_RAM_BATTERY]         = "MBC5 with Rumble, RAM, Battery",

    [0xFF]                                  = nullptr // Invalid type bytes map to nullptr
};

/**
 * @brief   Defines an array mapping ROM size indicator bytes from a Game Boy
 *          cartridge's header to their corresponding sizes in bytes.
 */
static const size_t GB_CARTRIDGE_ROM_SIZES[] = {
    [0x00] = 1024 * 16 * 2,    // 32KB
    [0x01] = 1024 * 16 * 4,    // 64KB
    [0x02] = 1024 * 16 * 8,    // 128KB
    [0x03] = 1024 * 16 * 16,   // 256KB
    [0x04] = 1024 * 16 * 32,   // 512KB
    [0x05] = 1024 * 16 * 64,   // 1MB
    [0x06] = 1024 * 16 * 128,  // 2MB
    [0x07] = 1024 * 16 * 256,  // 4MB
    [0x52] = 1024 * 16 * 72,   // 1.1MB (Unofficial)
    [0x53] = 1024 * 16 * 80,   // 1.2MB (Unofficial)
    [0x54] = 1024 * 16 * 96,   // 1.5MB (Unofficial)
    [0xFF] = 0                 // Invalid size bytes map to 0
};

/**
 * @brief   Defines an array mapping RAM size indicator bytes from a Game Boy
 *          cartridge's header to their corresponding sizes in bytes.
 */
static const size_t GB_CARTRIDGE_RAM_SIZES[] = {
    [0x00] = 0,                // No RAM
    [0x01] = 1024 * 2,         // 2KB (Unofficial)
    [0x02] = 1024 * 8,         // 8KB
    [0x03] = 1024 * 32,        // 32KB
    [0x04] = 1024 * 128,       // 128KB
    [0x05] = 1024 * 64,        // 64KB
    [0xFF] = 0                 // Invalid size bytes map to 0
};

/**
 * @brief   Defines the minimum valid size for a Game Boy cartridge ROM image
 *          file, in bytes.
 */
static const size_t GB_CARTRIDGE_MINIMUM_SIZE = 1024 * 16 * 2; // 32KB

/* Private Unions and Structures **********************************************/

struct gbCartridge
{
    // Common Attributes
    const gbCartridgeHeader*    header;
    uint8_t*                    romData;
    uint8_t*                    ramData;
    size_t                      romSize;
    size_t                      ramSize;
    
    // Type-Specific Attributes
    bool                        hasBattery;
    bool                        hasTimer;
    bool                        hasRumble;
    bool                        ramEnabled;
    bool                        ramBankingEnabled;      // MBC1 only
    uint8_t                     romBankNumber;
    uint8_t                     ramBankNumber;
    uint8_t                     rtcRegisters[5];        // MBC3 with Timer only
    uint8_t                     rtcLatchedRegisters[5]; // MBC3 with Timer only
    bool                        rtcLatchPrimed;         // MBC3 with Timer only
    time_t                      rtcLastUpdated;         // MBC3 with Timer only
};

/* Private Function Declarations - Read ROM ***********************************/

static bool gbReadBasicCartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC1CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC2CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC3CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC5CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);

/* Private Function Declarations - Read RAM ***********************************/

static bool gbReadBasicCartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC1CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC2CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC3CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);
static bool gbReadMBC5CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue);

/* Private Function Declarations - Write ROM **********************************/

static bool gbWriteBasicCartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC1CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC2CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC3CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC5CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);

/* Private Function Declarations - Write RAM **********************************/

static bool gbWriteBasicCartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC1CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC2CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC3CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);
static bool gbWriteMBC5CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual);

/* Private Function Declarations - Validation *********************************/

static bool gbValidateBasic (gbCartridge* cartridge, 
    const gbCartridgeHeader* header);
static bool gbValidateMBC1 (gbCartridge* cartridge, 
    const gbCartridgeHeader* header);
static bool gbValidateMBC2 (gbCartridge* cartridge, 
    const gbCartridgeHeader* header);
static bool gbValidateMBC3 (gbCartridge* cartridge, 
    const gbCartridgeHeader* header);
static bool gbValidateMBC5 (gbCartridge* cartridge, 
    const gbCartridgeHeader* header);
static bool gbValidateCartridgeHeader (gbCartridge* cartridge, 
    const gbCartridgeHeader* header);

/* Private Function Definitions - Read ROM ************************************/

bool gbReadBasicCartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - Basic cartridges have no banking hardware. Just read directly from ROM.
    *outValue = cartridge->romData[address];

    return true;
}

bool gbReadMBC1CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

bool gbReadMBC2CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

bool gbReadMBC3CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

bool gbReadMBC5CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

/* Private Function Definitions - Read RAM ************************************/

bool gbReadBasicCartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - Basic cartridges have no banking hardware. Just read directly from RAM,
    //   if there is any.
    *outValue = (cartridge->ramData != nullptr) ?
        cartridge->ramData[address] : 0xFF;

    return true;
}

bool gbReadMBC1CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

bool gbReadMBC2CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

bool gbReadMBC3CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

bool gbReadMBC5CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    return false;
}

/* Private Function Definitions - Write ROM ***********************************/

bool gbWriteBasicCartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    // - Basic cartridges have no external hardware with registers to write to.
    //   Do nothing.

    *outActual = 0xFF;
    return true;
}

bool gbWriteMBC1CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

bool gbWriteMBC2CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

bool gbWriteMBC3CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

bool gbWriteMBC5CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

/* Private Function Definitions - Write RAM ***********************************/

bool gbWriteBasicCartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    // - Basic cartridges have no banking hardware. Just write directly to RAM,
    //   if there is any.
    if (cartridge->ramData != nullptr)
    {
        cartridge->ramData[address] = value;
        *outActual = value;
    }
    else
    {
        *outActual = 0xFF;
    }

    return false;
}

bool gbWriteMBC1CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

bool gbWriteMBC2CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

bool gbWriteMBC3CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

bool gbWriteMBC5CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    return false;
}

/* Private Function Definitions - Validation **********************************/

bool gbValidateBasic (gbCartridge* cartridge, const gbCartridgeHeader* header)
{
    // - Requires exactly 32KB of ROM data.
    if (header->romSizeByte != 0x00)
    {
        gbLogError("Validation error: Basic cartridge must have 32KB of ROM.");
        return false;
    }

    // - Requires either 0KB or 8KB of RAM.
    if (header->ramSizeByte != 0x00 && header->ramSizeByte != 0x02)
    {
        gbLogError("Validation error: Basic cartridge must have 0KB or 8KB of RAM.");
        return false;
    }

    // - May have a battery only if it has RAM.
    cartridge->hasBattery = 
        (header->cartridgeType == GB_CT_BASIC_RAM_BATTERY) &&
        (header->ramSizeByte != 0x00);

    return true;
}

bool gbValidateMBC1 (gbCartridge* cartridge, const gbCartridgeHeader* header)
{
    // - Requires up to 2MB of ROM.
    if (header->romSizeByte > 0x06)
    {
        gbLogError("Validation error: MBC1 cartridge supports up to 2MB of ROM.");
        return false;
    }

    // - Requires up to 32KB of RAM if ROM <= 512KB.
    // - Requires up to 8KB of RAM if ROM > 512KB.
    if (header->romSizeByte <= 0x04)
    {
        if (header->ramSizeByte > 0x03)
        {
            gbLogError("Validation error: MBC1 cartridge with ROM size up to 512KB "
                "supports up to 32KB of RAM.");
            return false;
        }
    }
    else
    {
        if (header->ramSizeByte > 0x02)
        {
            gbLogError("Validation error: MBC1 cartridge with ROM size above 512KB "
                "supports up to 8KB of RAM.");
            return false;
        }
    }

    // - May have a battery only if it has RAM.
    cartridge->hasBattery = 
        (header->cartridgeType == GB_CT_MBC1_RAM_BATTERY) &&
        (header->ramSizeByte != 0x00);

    return true;
}

bool gbValidateMBC2 (gbCartridge* cartridge, const gbCartridgeHeader* header)
{
    // - Requires up to 256KB of ROM.
    if (header->romSizeByte > 0x03)
    {
        gbLogError("Validation error: MBC2 cartridge supports up to 256KB of ROM.");
        return false;
    }

    // - MBC2 cartridges have 512 x 4 bits (256 bytes) of RAM built in. It is
    //   not considered external RAM, so the RAM size byte must be 0x00.
    if (header->ramSizeByte != 0x00)
    {
        gbLogError("Validation error: MBC2 cartridge must have no external RAM.");
        return false;
    }

    // - May have a battery.
    cartridge->hasBattery = 
        (header->cartridgeType == GB_CT_MBC2_BATTERY);

    return true;
}

bool gbValidateMBC3 (gbCartridge* cartridge, const gbCartridgeHeader* header)
{
    // - Requires up to 2MB of ROM.
    if (header->romSizeByte > 0x06)
    {
        gbLogError("Validation error: MBC3 cartridge supports up to 2MB of ROM.");
        return false;
    }

    // - Requires up to 32KB of RAM.
    if (header->ramSizeByte > 0x03)
    {
        gbLogError("Validation error: MBC3 cartridge supports up to 32KB of RAM.");
        return false;
    }

    // - May have a battery.
    cartridge->hasBattery = 
        (header->cartridgeType == GB_CT_MBC3_RAM_BATTERY) ||
        (header->cartridgeType == GB_CT_MBC3_TIMER_BATTERY) ||
        (header->cartridgeType == GB_CT_MBC3_TIMER_RAM_BATTERY);
        
    // - May have a timer only if it has a battery.
    cartridge->hasTimer = 
        ((header->cartridgeType == GB_CT_MBC3_TIMER_BATTERY) ||
         (header->cartridgeType == GB_CT_MBC3_TIMER_RAM_BATTERY)) &&
        cartridge->hasBattery;

    // - If it has a timer, update RTC last updated time.
    if (cartridge->hasTimer)
    {
        cartridge->rtcLastUpdated = time(nullptr);
    }

    return true;
}

bool gbValidateMBC5 (gbCartridge* cartridge, const gbCartridgeHeader* header)
{
    // - Requires up to 8MB of ROM.
    if (header->romSizeByte > 0x07)
    {
        gbLogError("Validation error: MBC5 cartridge supports up to 8MB of ROM.");
        return false;
    }

    // - Requires 0KB, 8KB, 32KB or 128KB of RAM.
    if (header->ramSizeByte != 0x00 &&
        header->ramSizeByte != 0x02 &&
        header->ramSizeByte != 0x03 &&
        header->ramSizeByte != 0x04)
    {
        gbLogError("Validation error: MBC5 cartridge supports 0KB, 8KB, 32KB or 128KB of RAM.");
        return false;
    }

    // - May have a battery.
    cartridge->hasBattery =
        (header->cartridgeType == GB_CT_MBC5_RAM_BATTERY) ||
        (header->cartridgeType == GB_CT_MBC5_RUMBLE_RAM_BATTERY);

    // - May have rumble.
    cartridge->hasRumble =
        (header->cartridgeType == GB_CT_MBC5_RUMBLE) ||
        (header->cartridgeType == GB_CT_MBC5_RUMBLE_RAM) ||
        (header->cartridgeType == GB_CT_MBC5_RUMBLE_RAM_BATTERY);

    return true;
}

bool gbValidateCartridgeHeader (gbCartridge* cartridge, 
    const gbCartridgeHeader* header)
{
    // - Validate the Nintendo logo.
    if (memcmp(header->nintendoLogo, NINTENDO_LOGO, sizeof(NINTENDO_LOGO)) != 0)
    {
        gbLogError("Validation error: Nintendo logo is missing or invalid.");
        return false;
    }

    // - Validate header checksum.
    uint8_t checksum = 0;
    const uint8_t* ptr = (const uint8_t*) header;
    for (size_t i = 0x34; i <= 0x4C; ++i)
    {
        checksum = (uint8_t) (checksum - ptr[i] - 1);
    }

    if (checksum != header->headerChecksum)
    {
        gbLogError("Validation error: Cartridge header checksum is invalid.");
        return false;
    }

    // - Perform type-specific validation.
    switch (header->cartridgeType)
    {
        case GB_CT_BASIC:
        case GB_CT_BASIC_RAM:
        case GB_CT_BASIC_RAM_BATTERY:
            return gbValidateBasic(cartridge, header);

        case GB_CT_MBC1:
        case GB_CT_MBC1_RAM:
        case GB_CT_MBC1_RAM_BATTERY:
            return gbValidateMBC1(cartridge, header);

        case GB_CT_MBC2:
        case GB_CT_MBC2_BATTERY:
            return gbValidateMBC2(cartridge, header);

        case GB_CT_MBC3:
        case GB_CT_MBC3_RAM:
        case GB_CT_MBC3_RAM_BATTERY:
        case GB_CT_MBC3_TIMER_BATTERY:
        case GB_CT_MBC3_TIMER_RAM_BATTERY:
            return gbValidateMBC3(cartridge, header);

        case GB_CT_MBC5:
        case GB_CT_MBC5_RAM:
        case GB_CT_MBC5_RAM_BATTERY:
        case GB_CT_MBC5_RUMBLE:
        case GB_CT_MBC5_RUMBLE_RAM:
        case GB_CT_MBC5_RUMBLE_RAM_BATTERY:
            return gbValidateMBC5(cartridge, header);

        default:
            gbLogError("Validation error: Unknown or unsupported cartridge type 0x%02X.",
                header->cartridgeType);
            return false;
    }
}

/* Public Function Definitions ************************************************/

gbCartridge* gbCreateCartridge (const char* filepath)
{
    gbCheckv(filepath != nullptr, nullptr, "File path string is null.");
    gbCheckv(filepath[0] != '\0', nullptr, "File path string is blank.");

    // - Attempt to open the specified file.
    FILE* fp = fopen(filepath, "rb");
    gbCheckpv(fp != nullptr, nullptr, "Failed to open ROM file '%s' for reading", 
        filepath);

    // - Determine the size of the file.
    // - Make sure it meets the minimum size requirement.
    fseek(fp, 0, SEEK_END);
    long result = ftell(fp);
    if (result < 0)
    {
        gbLogErrno("Failed to determine size of ROM file '%s'", filepath);
        fclose(fp);
        return nullptr;
    }

    // - Create the cartridge structure.
    gbCartridge* cartridge = gbCreateZero(1, gbCartridge);
    if (cartridge == nullptr)
    {
        gbLogErrno("Error allocating memory for 'gbCartridge'");
        fclose(fp);
        return nullptr;
    }

    // - Allocate the ROM data buffer.
    cartridge->romSize = (size_t) result;
    if (cartridge->romSize < GB_CARTRIDGE_MINIMUM_SIZE)
    {
        gbLogError("ROM file '%s' is too small to be a valid Game Boy cartridge",
            filepath);
        gbDestroyCartridge(cartridge);
        fclose(fp);
        return nullptr;
    }

    cartridge->romData = gbCreateZero(cartridge->romSize, uint8_t);
    if (cartridge->romData == nullptr)
    {
        gbLogErrno("Error allocating memory for cartridge ROM data");
        gbDestroyCartridge(cartridge);
        fclose(fp);
        return nullptr;
    }

    // - Read the ROM data from the file.
    fseek(fp, 0, SEEK_SET);
    size_t bytesRead = fread(cartridge->romData, 1, cartridge->romSize, fp);
    if (bytesRead != cartridge->romSize)
    {
        gbLogErrno("Error reading ROM data from file '%s'", filepath);
        gbDestroyCartridge(cartridge);
        fclose(fp);
        return nullptr;
    }

    // - Point to the header within the ROM data and validate it.
    cartridge->header =
        (const gbCartridgeHeader*) (cartridge->romData + 0x0100);
    if (!gbValidateCartridgeHeader(cartridge, cartridge->header))
    {
        gbLogError("Invalid or corrupted cartridge header in file '%s'",
            filepath);
        gbDestroyCartridge(cartridge);
        fclose(fp);
        return nullptr;
    }

    // - Close the file, as we no longer need it.
    fclose(fp);

    // - Validate the ROM size.
    const size_t expectedRomSize = gbGetCartridgeROMSize(cartridge->header);
    if (cartridge->romSize != expectedRomSize)
    {
        gbLogError("ROM size mismatch in file '%s': expected %zu bytes, got %zu bytes",
            filepath, expectedRomSize, cartridge->romSize);
        gbDestroyCartridge(cartridge);
        return nullptr;
    }

    // - Allocate the RAM data buffer, if applicable.
    if (cartridge->header->ramSizeByte != 0x00)
    {
        cartridge->ramSize = gbGetCartridgeRAMSize(cartridge->header);
        cartridge->ramData = gbCreateZero(cartridge->ramSize, uint8_t);
        if (cartridge->ramData == nullptr)
        {
            gbLogErrno("Error allocating memory for cartridge RAM data");
            gbDestroyCartridge(cartridge);
            return nullptr;
        }
    }

    return cartridge;
}

bool gbDestroyCartridge (gbCartridge* cartridge)
{
    gbCheckqv(cartridge, false);
    gbDestroy(cartridge->romData);
    gbDestroy(cartridge->ramData);
    gbDestroy(cartridge);
    return true;
}

/* Public Function Definitions - Cartridge Header *****************************/

const gbCartridgeHeader* gbGetCartridgeHeader (
    const gbCartridge* cartridge)
{
    gbCheckv(cartridge != nullptr, nullptr, "No valid 'gbCartridge' provided.");
    gbCheckv(cartridge->header != nullptr, nullptr,
        "The provided 'gbCartridge' has no valid header.");

    return cartridge->header;
}

const char* gbGetCartridgeTitle (const gbCartridgeHeader* header)
{
    gbCheckv(header != nullptr, nullptr, "No valid 'gbCartridgeHeader' provided.");
    return header->title;
}

bool gbCheckCartridgeCGBSupport (const gbCartridgeHeader* header,
    bool* outSupportsCGB, bool* outRequiresCGB)
{
    gbCheckv(header != nullptr, false, "No valid 'gbCartridgeHeader' provided.");
    gbCheckv(outSupportsCGB != nullptr, false,
        "No valid output pointer provided for CGB support.");

    // - Determine CGB support and requirement based on the CGB flag.
    const uint8_t cgbFlag = header->cgbFlag;
    *outSupportsCGB = gbGetBit(cgbFlag, 7);
    if (outRequiresCGB != nullptr)
    {
        *outRequiresCGB = gbGetMaskAll(cgbFlag, 0b11000000);
    }

    return true;
}

const char* gbStringifyCartridgeType (const gbCartridgeHeader* header)
{
    gbCheckv(header != nullptr, nullptr, "No valid 'gbCartridgeHeader' provided.");

    return GB_CARTRIDGE_TYPE_STRINGS[header->cartridgeType];
}

size_t gbGetCartridgeROMSize (const gbCartridgeHeader* header)
{
    gbCheckv(header != nullptr, 0, "No valid 'gbCartridgeHeader' provided.");

    return GB_CARTRIDGE_ROM_SIZES[header->romSizeByte];
}

size_t gbGetCartridgeRAMSize (const gbCartridgeHeader* header)
{
    gbCheckv(header != nullptr, 0, "No valid 'gbCartridgeHeader' provided.");

    return GB_CARTRIDGE_RAM_SIZES[header->ramSizeByte];
}

/* Public Function Definitions - Battery-Backed RAM ***************************/

bool gbLoadCartridgeRAM (gbCartridge* cartridge, const char* filepath,
    bool evenIfNoBattery)
{
    gbCheckv(cartridge != nullptr, false, "No valid 'gbCartridge' provided.");
    gbCheckv(filepath != nullptr, false, "File path string is null.");
    gbCheckv(filepath[0] != '\0', false, "File path string is blank.");

    // - If the cartridge has no RAM, nothing to load.
    if (cartridge->ramSize == 0 || cartridge->ramData == nullptr)
    {
        return true;
    }

    // - If the cartridge has no battery and we're not loading anyway, skip.
    if (!cartridge->hasBattery && !evenIfNoBattery)
    {
        return true;
    }

    // - Attempt to open the specified file.
    FILE* fp = fopen(filepath, "rb");
    gbCheckpv(fp != nullptr, true,
        "Failed to open RAM file '%s' for reading", filepath);

    // - Get the size of the file.
    // - It must match the expected RAM size.
    fseek(fp, 0, SEEK_END);
    long result = ftell(fp);
    if (result < 0)
    {
        gbLogErrno("Failed to determine size of RAM file '%s'", filepath);
        fclose(fp);
        return false;
    }

    const size_t fileSize = (size_t) result;
    if (fileSize != cartridge->ramSize)
    {
        gbLogError("RAM size mismatch in file '%s': expected %zu bytes, got %zu bytes",
            filepath, cartridge->ramSize, fileSize);
        fclose(fp);
        return false;   
    }

    // - Read the RAM data from the file.
    fseek(fp, 0, SEEK_SET);
    size_t bytesRead = fread(cartridge->ramData, 1, cartridge->ramSize, fp);
    if (bytesRead != cartridge->ramSize)
    {
        gbLogErrno("Error reading RAM data from file '%s'", filepath);
        fclose(fp);
        return false;
    }

    // - Close the file.
    fclose(fp);
    return true;
}

bool gbSaveCartridgeRAM (const gbCartridge* cartridge,
    const char* filepath, bool evenIfNoBattery)
{
    gbCheckv(cartridge != nullptr, false, "No valid 'gbCartridge' provided.");
    gbCheckv(filepath != nullptr, false, "File path string is null.");
    gbCheckv(filepath[0] != '\0', false, "File path string is blank.");

    // - If the cartridge has no RAM, nothing to save.
    if (cartridge->ramSize == 0 || cartridge->ramData == nullptr)
    {
        return true;
    }

    // - If the cartridge has no battery and we're not saving anyway, skip.
    if (!cartridge->hasBattery && !evenIfNoBattery)
    {
        return true;
    }

    // - Attempt to open the specified file.
    FILE* fp = fopen(filepath, "wb");
    gbCheckpv(fp != nullptr, false,
        "Failed to open RAM file '%s' for writing", filepath);

    // - Write the RAM data to the file.
    size_t bytesWritten = fwrite(cartridge->ramData, 1, cartridge->ramSize, fp);
    if (bytesWritten != cartridge->ramSize)
    {
        gbLogErrno("Error writing RAM data to file '%s'", filepath);
        fclose(fp);
        return false;
    }

    // - Close the file.
    fclose(fp);
    return true;
}

/* Public Function Definitions - Memory Access ********************************/

bool gbReadCartridgeROM (const gbCartridge* cartridge, uint16_t address,
    uint8_t* outValue)
{
    gbCheckv(cartridge != nullptr, false, "No valid 'gbCartridge' provided.");
    gbCheckv(cartridge->header != nullptr, false,
        "The provided 'gbCartridge' has no valid header.");
    gbCheckv(cartridge->romData != nullptr && cartridge->romSize > 0, false,
        "The provided 'gbCartridge' has no valid ROM data.");
    gbCheckv(outValue != nullptr, false, "No valid output pointer provided.");
    gbCheckv(address < GB_ROM_SIZE, false,
        "ROM relative read address '$%04X' is out of bounds.", address);

    // - Perform type-specific ROM read.
    switch (cartridge->header->cartridgeType)
    {
        case GB_CT_BASIC:
        case GB_CT_BASIC_RAM:
        case GB_CT_BASIC_RAM_BATTERY:
            return gbReadBasicCartridgeROM(cartridge, address, outValue);

        case GB_CT_MBC1:
        case GB_CT_MBC1_RAM:
        case GB_CT_MBC1_RAM_BATTERY:
            return gbReadMBC1CartridgeROM(cartridge, address, outValue);

        case GB_CT_MBC2:
        case GB_CT_MBC2_BATTERY:
            return gbReadMBC2CartridgeROM(cartridge, address, outValue);

        case GB_CT_MBC3:
        case GB_CT_MBC3_RAM:
        case GB_CT_MBC3_RAM_BATTERY:
        case GB_CT_MBC3_TIMER_BATTERY:
        case GB_CT_MBC3_TIMER_RAM_BATTERY:
            return gbReadMBC3CartridgeROM(cartridge, address, outValue);

        case GB_CT_MBC5:
        case GB_CT_MBC5_RAM:
        case GB_CT_MBC5_RAM_BATTERY:
        case GB_CT_MBC5_RUMBLE:
        case GB_CT_MBC5_RUMBLE_RAM:
        case GB_CT_MBC5_RUMBLE_RAM_BATTERY:
            return gbReadMBC5CartridgeROM(cartridge, address, outValue);

        default:
            gbLogError("Unknown or unsupported cartridge type 0x%02X for ROM read.",
                cartridge->header->cartridgeType);
            return false;
    }
}

bool gbReadCartridgeRAM (const gbCartridge* cartridge, uint16_t address,
    uint8_t* outValue)
{
    gbCheckv(cartridge != nullptr, false, "No valid 'gbCartridge' provided.");
    gbCheckv(cartridge->header != nullptr, false,
        "The provided 'gbCartridge' has no valid header.");
    gbCheckv(outValue != nullptr, false, "No valid output pointer provided.");
    gbCheckv(address < GB_EXTRAM_SIZE, false,
        "RAM relative read address '$%04X' is out of bounds.", address);

    // - If no RAM, read open-bus.
    if (cartridge->ramSize == 0 || cartridge->ramData == nullptr)
    {
        *outValue = 0xFF; // Open-bus value
        return true;
    }

    // - Perform type-specific RAM read.
    switch (cartridge->header->cartridgeType)
    {
        case GB_CT_BASIC:
        case GB_CT_BASIC_RAM:
        case GB_CT_BASIC_RAM_BATTERY:
            return gbReadBasicCartridgeRAM(cartridge, address, outValue);

        case GB_CT_MBC1:
        case GB_CT_MBC1_RAM:
        case GB_CT_MBC1_RAM_BATTERY:
            return gbReadMBC1CartridgeRAM(cartridge, address, outValue);

        case GB_CT_MBC2:
        case GB_CT_MBC2_BATTERY:
            return gbReadMBC2CartridgeRAM(cartridge, address, outValue);

        case GB_CT_MBC3:
        case GB_CT_MBC3_RAM:
        case GB_CT_MBC3_RAM_BATTERY:
        case GB_CT_MBC3_TIMER_BATTERY:
        case GB_CT_MBC3_TIMER_RAM_BATTERY:
            return gbReadMBC3CartridgeRAM(cartridge, address, outValue);

        case GB_CT_MBC5:
        case GB_CT_MBC5_RAM:
        case GB_CT_MBC5_RAM_BATTERY:
        case GB_CT_MBC5_RUMBLE:
        case GB_CT_MBC5_RUMBLE_RAM:
        case GB_CT_MBC5_RUMBLE_RAM_BATTERY:
            return gbReadMBC5CartridgeRAM(cartridge, address, outValue);

        default:
            gbLogError("Unknown or unsupported cartridge type 0x%02X for RAM read.",
                cartridge->header->cartridgeType);
            return false;
    }
}

bool gbWriteCartridgeROM (gbCartridge* cartridge, uint16_t address,
    uint8_t value, uint8_t* outActual)
{
    gbCheckv(outActual != nullptr, false, "No valid output pointer provided.");
    gbCheckv(cartridge != nullptr, false, "No valid 'gbCartridge' provided.");
    gbCheckv(cartridge->header != nullptr, false,
        "The provided 'gbCartridge' has no valid header.");
    gbCheckv(cartridge->romData != nullptr && cartridge->romSize > 0, false,
        "The provided 'gbCartridge' has no valid ROM data.");
    gbCheckv(address < GB_ROM_SIZE, false,
        "ROM relative write address '$%04X' is out of bounds.", address);

    // - Perform type-specific ROM write.
    switch (cartridge->header->cartridgeType)
    {
        case GB_CT_BASIC:
        case GB_CT_BASIC_RAM:
        case GB_CT_BASIC_RAM_BATTERY:
            return gbWriteBasicCartridgeROM(cartridge, address, value, outActual);

        case GB_CT_MBC1:
        case GB_CT_MBC1_RAM:
        case GB_CT_MBC1_RAM_BATTERY:
            return gbWriteMBC1CartridgeROM(cartridge, address, value, outActual);

        case GB_CT_MBC2:
        case GB_CT_MBC2_BATTERY:
            return gbWriteMBC2CartridgeROM(cartridge, address, value, outActual);

        case GB_CT_MBC3:
        case GB_CT_MBC3_RAM:
        case GB_CT_MBC3_RAM_BATTERY:
        case GB_CT_MBC3_TIMER_BATTERY:
        case GB_CT_MBC3_TIMER_RAM_BATTERY:
            return gbWriteMBC3CartridgeROM(cartridge, address, value, outActual);

        case GB_CT_MBC5:
        case GB_CT_MBC5_RAM:
        case GB_CT_MBC5_RAM_BATTERY:
        case GB_CT_MBC5_RUMBLE:
        case GB_CT_MBC5_RUMBLE_RAM:
        case GB_CT_MBC5_RUMBLE_RAM_BATTERY:
            return gbWriteMBC5CartridgeROM(cartridge, address, value, outActual);

        default:
            gbLogError("Unknown or unsupported cartridge type 0x%02X for ROM write.",
                cartridge->header->cartridgeType);
            return false;        
    }
}

bool gbWriteCartridgeRAM (gbCartridge* cartridge, uint16_t address,
    uint8_t value, uint8_t* outActual)
{
    gbCheckv(outActual != nullptr, false, "No valid output pointer provided.");
    gbCheckv(cartridge != nullptr, false, "No valid 'gbCartridge' provided.");
    gbCheckv(cartridge->header != nullptr, false,
        "The provided 'gbCartridge' has no valid header.");
    gbCheckv(address < GB_EXTRAM_SIZE, false,
        "RAM relative write address '$%04X' is out of bounds.", address);

    // - If no RAM, ignore write.
    if (cartridge->ramSize == 0 || cartridge->ramData == nullptr)
    {
        return true;
    }

    // - Perform type-specific RAM write.
    switch (cartridge->header->cartridgeType)
    {
        case GB_CT_BASIC:
        case GB_CT_BASIC_RAM:
        case GB_CT_BASIC_RAM_BATTERY:
            return gbWriteBasicCartridgeRAM(cartridge, address, value, outActual);

        case GB_CT_MBC1:
        case GB_CT_MBC1_RAM:
        case GB_CT_MBC1_RAM_BATTERY:
            return gbWriteMBC1CartridgeRAM(cartridge, address, value, outActual);

        case GB_CT_MBC2:
        case GB_CT_MBC2_BATTERY:
            return gbWriteMBC2CartridgeRAM(cartridge, address, value, outActual);

        case GB_CT_MBC3:
        case GB_CT_MBC3_RAM:
        case GB_CT_MBC3_RAM_BATTERY:
        case GB_CT_MBC3_TIMER_BATTERY:
        case GB_CT_MBC3_TIMER_RAM_BATTERY:
            return gbWriteMBC3CartridgeRAM(cartridge, address, value, outActual);

        case GB_CT_MBC5:
        case GB_CT_MBC5_RAM:
        case GB_CT_MBC5_RAM_BATTERY:
        case GB_CT_MBC5_RUMBLE:
        case GB_CT_MBC5_RUMBLE_RAM:
        case GB_CT_MBC5_RUMBLE_RAM_BATTERY:
            return gbWriteMBC5CartridgeRAM(cartridge, address, value, outActual);

        default:
            gbLogError("Unknown or unsupported cartridge type 0x%02X for RAM write.",
                cartridge->header->cartridgeType);
            return false;        
    }
}
