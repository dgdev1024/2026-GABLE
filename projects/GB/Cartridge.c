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
    uint8_t                     rtcLatchValue;          // MBC3 with Timer only
    time_t                      rtcLastUpdated;         // MBC3 with Timer only
    bool                        rtcHalted;              // MBC3 with Timer only
    uint16_t                    rtcDayCounter;          // MBC3 with Timer only
    bool                        rtcCarryBit;            // MBC3 with Timer only
};

/* Private Function Declarations - Helper Functions ***************************/

static void gbUpdateMBC3RTC (gbCartridge* cartridge);

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

/* Private Function Definitions - Helper Functions ****************************/

void gbUpdateMBC3RTC (gbCartridge* cartridge)
{
    gbAssert(cartridge != nullptr);

    // - Only update if the cartridge has a timer.
    if (!cartridge->hasTimer)
    {
        return;
    }

    // - Check if the RTC is halted (bit 6 of DH register).
    if (cartridge->rtcHalted)
    {
        return;
    }

    // - Calculate elapsed time since last update.
    time_t currentTime = time(nullptr);
    time_t elapsedSeconds = currentTime - cartridge->rtcLastUpdated;
    cartridge->rtcLastUpdated = currentTime;

    // - Update RTC registers.
    while (elapsedSeconds > 0)
    {
        // - Increment seconds.
        cartridge->rtcRegisters[0]++;
        if (cartridge->rtcRegisters[0] >= 60)
        {
            cartridge->rtcRegisters[0] = 0;

            // - Increment minutes.
            cartridge->rtcRegisters[1]++;
            if (cartridge->rtcRegisters[1] >= 60)
            {
                cartridge->rtcRegisters[1] = 0;

                // - Increment hours.
                cartridge->rtcRegisters[2]++;
                if (cartridge->rtcRegisters[2] >= 24)
                {
                    cartridge->rtcRegisters[2] = 0;

                    // - Increment day counter (9-bit: DL + bit 0 of DH).
                    cartridge->rtcDayCounter++;
                    if (cartridge->rtcDayCounter > 0x1FF)
                    {
                        // - Day counter overflow: set carry bit (bit 7 of DH).
                        cartridge->rtcDayCounter = 0;
                        cartridge->rtcCarryBit = true;
                    }

                    // - Update DL and DH registers from day counter.
                    cartridge->rtcRegisters[3] = 
                        (uint8_t) (cartridge->rtcDayCounter & 0xFF);
                    
                    // - Reconstruct DH: bit 0 = day counter bit 8, bit 6 = halt, 
                    //   bit 7 = carry.
                    cartridge->rtcRegisters[4] = 
                        ((cartridge->rtcDayCounter >> 8) & 0x01) |
                        (cartridge->rtcHalted ? 0x40 : 0x00) |
                        (cartridge->rtcCarryBit ? 0x80 : 0x00);
                }
            }
        }

        elapsedSeconds--;
    }
}

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

    // - Calculate the maximum bank number based on ROM size
    size_t maxBankNumber = (cartridge->romSize / GB_ROM_BANK_SIZE) - 1;
    size_t bankMask = maxBankNumber;
    
    // - `$0000 - $3FFF`: See below
    if (address < GB_ROM_BANK_SIZE)
    {
        // - If RAM banking mode is disabled, this area always maps to ROM bank 0.
        if (!cartridge->ramBankingEnabled)
        {
            *outValue = cartridge->romData[address];
        }

        // - If RAM banking mode is enabled (mode 1), this area maps to the ROM bank
        //   selected by the secondary 2-bit register (ramBankNumber), allowing
        //   access to banks $00/$20/$40/$60 (or $00/$10/$20/$30 for MBC1M).
        else
        {
            uint8_t bankNumber = (cartridge->ramBankNumber << 5) & bankMask;
            size_t bankOffset = bankNumber * GB_ROM_BANK_SIZE;
            *outValue = cartridge->romData[bankOffset + address];
        }
    }

    // - `$4000 - $7FFF`: Switchable ROM bank area.
    else if (address < (GB_ROM_BANK_SIZE * 2))
    {
        // - Combine the 5-bit ROM bank register with the 2-bit secondary register
        //   for large ROMs: Selected ROM Bank = (Secondary Bank << 5) + ROM Bank
        uint8_t bankNumber = (cartridge->ramBankNumber << 5) | 
            (cartridge->romBankNumber & 0x1F);
        
        if ((cartridge->romBankNumber & 0x1F) == 0x00)
        {
            // - ROM banks `0x00`, `0x20`, `0x40` and `0x60` are not allowed
            //   and thereby map to the next valid bank ($01/$21/$41/$61).
            //   This check uses only the lower 5 bits of the ROM bank register.
            bankNumber |= 0x01;
        }

        // - Mask the bank number to the actual number of banks available
        bankNumber &= bankMask;

        size_t bankOffset = bankNumber * GB_ROM_BANK_SIZE;
        size_t relativeAddress = address - GB_ROM_BANK_SIZE;
        *outValue = cartridge->romData[bankOffset + relativeAddress];
    }

    return true;
}

bool gbReadMBC2CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - `$0000 - $3FFF`: Fixed ROM bank 0
    if (address < GB_ROM_BANK_SIZE)
    {
        *outValue = cartridge->romData[address];
    }

    // - `$4000 - $7FFF`: Switchable ROM bank 0x01-0x0F (4-bit bank number).
    //   MBC2 supports only 16 ROM banks (max 256KB ROM).
    else if (address < (GB_ROM_BANK_SIZE * 2))
    {
        // - ROM bank number is stored in lower 4 bits of romBankNumber
        uint8_t bankNumber = cartridge->romBankNumber & 0x0F;
        
        // - Bank 0 is not allowed and maps to bank 1 (handled in write function)
        // - Mask to actual ROM size
        size_t maxBankNumber = (cartridge->romSize / GB_ROM_BANK_SIZE) - 1;
        if (maxBankNumber > 0x0F) maxBankNumber = 0x0F;  // MBC2 max is 15
        bankNumber &= maxBankNumber;

        size_t bankOffset = bankNumber * GB_ROM_BANK_SIZE;
        size_t relativeAddress = address - GB_ROM_BANK_SIZE;
        *outValue = cartridge->romData[bankOffset + relativeAddress];
    }

    return true;
}

bool gbReadMBC3CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - `$0000 - $3FFF`: Fixed ROM bank 0
    if (address < GB_ROM_BANK_SIZE)
    {
        *outValue = cartridge->romData[address];
    }

    // - `$4000 - $7FFF`: Switchable ROM bank area.
    else if (address < (GB_ROM_BANK_SIZE * 2))
    {
        uint8_t bankNumber = (cartridge->romBankNumber & 0x7F);
        size_t bankOffset = bankNumber * GB_ROM_BANK_SIZE;
        size_t relativeAddress = address - GB_ROM_BANK_SIZE;
        *outValue = cartridge->romData[bankOffset + relativeAddress];
    }

    return true;
}

bool gbReadMBC5CartridgeROM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - `$0000 - $3FFF`: Fixed ROM bank 0
    if (address < GB_ROM_BANK_SIZE)
    {
        *outValue = cartridge->romData[address];
    }

    // - `$4000 - $7FFF`: Switchable ROM bank 0x00-0x1FF (9-bit bank number).
    //   Note: MBC5 allows bank 0 to be selected (unlike MBC1/MBC3).
    else if (address < (GB_ROM_BANK_SIZE * 2))
    {
        // - Combine romBankNumber (lower 8 bits) with ramBankingEnabled flag
        //   (used to store the 9th bit for MBC5).
        // - For proper MBC5 support, romBankNumber should ideally be uint16_t,
        //   but we work with the existing structure.
        uint16_t bankNumber = cartridge->romBankNumber;
        if (cartridge->ramBankingEnabled)
        {
            bankNumber |= 0x100;  // Set bit 8 if flag is set
        }

        // - Mask to actual ROM size
        size_t maxBankNumber = (cartridge->romSize / GB_ROM_BANK_SIZE) - 1;
        bankNumber &= maxBankNumber;

        size_t bankOffset = bankNumber * GB_ROM_BANK_SIZE;
        size_t relativeAddress = address - GB_ROM_BANK_SIZE;
        *outValue = cartridge->romData[bankOffset + relativeAddress];
    }

    return true;
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

    // - If RAM is disabled, read returns 0xFF.
    if (!cartridge->ramEnabled)
    {
        *outValue = 0xFF;
        return true;
    }

    // - If there is no RAM, read returns 0xFF.
    if (cartridge->ramData == nullptr)
    {
        *outValue = 0xFF;
        return true;
    }

    // - If RAM banking mode is disabled (mode 0), this area always maps to RAM 
    //   bank 0.
    // - Also, RAM banking only works with 32KB RAM (4 banks). Smaller RAM sizes
    //   always use bank 0.
    if (!cartridge->ramBankingEnabled || cartridge->ramSize <= GB_EXTRAM_SIZE)
    {
        *outValue = cartridge->ramData[address];
    }

    // - If RAM banking mode is enabled (mode 1) and RAM is 32KB, this area maps
    //   to the RAM bank selected by the secondary 2-bit register (ramBankNumber).
    else
    {
        size_t maxRamBank = (cartridge->ramSize / GB_EXTRAM_SIZE) - 1;
        size_t bankNumber = cartridge->ramBankNumber & maxRamBank;
        size_t bankOffset = bankNumber * GB_EXTRAM_SIZE;
        *outValue = cartridge->ramData[bankOffset + address];
    }

    return true;
}

bool gbReadMBC2CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - If RAM is disabled, read returns 0xFF.
    if (!cartridge->ramEnabled)
    {
        *outValue = 0xFF;
        return true;
    }

    // - MBC2 has built-in RAM (512 × 4-bit values, 512 bytes total).
    // - Only the lower 9 bits of the address are used, so RAM repeats every 512 bytes.
    // - Address range A000-BFFF repeats 16 times (A000-A1FF, A200-A3FF, etc.)
    // - Only the lower 4 bits of each byte are used; upper 4 bits are undefined.
    if (cartridge->ramData == nullptr)
    {
        *outValue = 0xFF;
        return true;
    }

    size_t ramAddress = address & 0x01FF;  // Lower 9 bits (0-511)
    uint8_t value = cartridge->ramData[ramAddress];
    
    // - Only lower 4 bits are valid; upper 4 bits are undefined (we return them as 0xF)
    *outValue = (value & 0x0F) | 0xF0;

    return true;
}

bool gbReadMBC3CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - If RAM/RTC access is disabled, read returns 0xFF.
    if (!cartridge->ramEnabled)
    {
        *outValue = 0xFF;
        return true;
    }

    // - If an RTC register is selected, read from the latched RTC registers.
    if (cartridge->ramBankNumber >= 0x08 &&
        cartridge->ramBankNumber <= 0x0C)
    {
        // - Update the real RTC before reading (only affects non-latched registers).
        gbUpdateMBC3RTC((gbCartridge*)cartridge);
        
        uint8_t rtcIndex = cartridge->ramBankNumber - 0x08;
        *outValue = cartridge->rtcLatchedRegisters[rtcIndex];
        return true;
    }

    // - If a RAM bank is selected, read from the RAM.
    if (cartridge->ramBankNumber <= 0x03)
    {
        // - If there is no RAM, read returns 0xFF.
        if (cartridge->ramData == nullptr)
        {
            *outValue = 0xFF;
            return true;
        }

        size_t maxRamBank = (cartridge->ramSize / GB_EXTRAM_SIZE) - 1;
        size_t bankNumber = cartridge->ramBankNumber & maxRamBank;
        size_t bankOffset = bankNumber * GB_EXTRAM_SIZE;
        *outValue = cartridge->ramData[bankOffset + address];
        return true;
    }

    return true;
}

bool gbReadMBC5CartridgeRAM (const gbCartridge* cartridge,
    uint16_t address, uint8_t* outValue)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outValue != nullptr);

    // - If RAM is disabled, read returns 0xFF.
    if (!cartridge->ramEnabled)
    {
        *outValue = 0xFF;
        return true;
    }

    // - If there is no RAM, read returns 0xFF.
    if (cartridge->ramData == nullptr)
    {
        *outValue = 0xFF;
        return true;
    }

    // - MBC5 supports RAM banks 0x00-0x0F (4 bits).
    //   For rumble cartridges, bit 3 of ramBankNumber controls the rumble motor,
    //   so only bits 0-2 are used for RAM banking (0x00-0x07).
    size_t maxRamBank = (cartridge->ramSize / GB_EXTRAM_SIZE) - 1;
    size_t bankNumber = cartridge->ramBankNumber & 0x0F;  // 4-bit bank number
    
    // - For rumble cartridges, mask to 3 bits instead of 4
    if (cartridge->hasRumble)
    {
        bankNumber &= 0x07;
    }
    
    bankNumber &= maxRamBank;
    size_t bankOffset = bankNumber * GB_EXTRAM_SIZE;
    *outValue = cartridge->ramData[bankOffset + address];

    return true;
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
    
    *outActual = 0xFF;

    // - `$0000 - $1FFF`: RAM Enable
    if (address < 0x2000)
    {
        // - Write any value with `0xA` in the lower nibble to enable RAM.
        // - Write any other value to disable RAM.
        cartridge->ramEnabled = ((value & 0x0F) == 0x0A);
    }

    // - `$2000 - $3FFF`: ROM Bank Number (lower 5 bits)
    else if (address < 0x4000)
    {
        cartridge->romBankNumber =
            (cartridge->romBankNumber & 0b11100000) | 
            (value & 0b00011111);
    }

    // - `$4000 - $5FFF`: RAM Bank Number or Upper Bits of ROM Bank Number
    else if (address < 0x6000)
    {
        if (cartridge->ramBankingEnabled)
        {
            // - RAM Banking Mode: Set RAM bank number (2 bits).
            cartridge->ramBankNumber = value & 0b00000011;
        }
        else
        {
            // - ROM Banking Mode: Set upper bits of ROM bank number (2 bits).
            cartridge->romBankNumber =
                (cartridge->romBankNumber & 0b00011111) |
                ((value & 0b00000011) << 5);
        }
    }

    // - `$6000 - $7FFF`: Banking Mode Select
    else if (address < 0x8000)
    {
        // - Write `0` to select ROM Banking Mode.
        // - Write `1` to select RAM Banking Mode.
        cartridge->ramBankingEnabled = (value & 0x01) != 0;
    }

    return true;
}

bool gbWriteMBC2CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    *outActual = 0xFF;

    // - `$0000 - $3FFF`: RAM Enable or ROM Bank Number
    //   The function is determined by bit 8 of the address (LSB of upper byte).
    if (address < 0x4000)
    {
        // - Check bit 8 of the address (address & 0x0100)
        if ((address & 0x0100) == 0)
        {
            // - Bit 8 is CLEAR: RAM Enable
            //   Write 0xA in lower 4 bits to enable RAM, anything else to disable.
            //   Examples: $0000-00FF, $0200-02FF, $0400-04FF, ..., $3E00-3EFF
            cartridge->ramEnabled = ((value & 0x0F) == 0x0A);
        }
        else
        {
            // - Bit 8 is SET: ROM Bank Number (4 bits: 0x01-0x0F)
            //   Examples: $0100-01FF, $0300-03FF, $0500-05FF, ..., $3F00-3FFF
            cartridge->romBankNumber = value & 0x0F;
            
            // - Writing 0 selects bank 1 instead (same as MBC1/MBC3)
            if (cartridge->romBankNumber == 0x00)
            {
                cartridge->romBankNumber = 0x01;
            }
        }
    }

    return true;
}

bool gbWriteMBC3CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    *outActual = 0xFF;

    // - `$0000 - $1FFF`: RAM and RTC Enable
    if (address < 0x2000)
    {
        // - Write any value with `0xA` in the lower nibble to enable RAM/RTC.
        // - Write any other value to disable RAM/RTC.
        cartridge->ramEnabled = ((value & 0x0F) == 0x0A);
    }

    // - `$2000 - $3FFF`: ROM Bank Number (7 bits)
    else if (address < 0x4000)
    {
        cartridge->romBankNumber = value & 0x7F;
        if (cartridge->romBankNumber == 0x00)
        {
            // - ROM bank `0x00` is not allowed and thereby maps to bank `0x01`.
            cartridge->romBankNumber = 0x01;
        }
    }

    // - `$4000 - $5FFF`: RAM Bank Number or RTC Register Select
    else if (address < 0x6000)
    {
        cartridge->ramBankNumber = value;
    }

    // - `$6000 - $7FFF`: Latch Clock Data
    else if (address < 0x8000)
    {
        // - Writing `0x00` followed by `0x01` latches the RTC data into the
        //   latched registers.
        if (value == 0x01 && cartridge->rtcLatchPrimed)
        {
            // - Update the real RTC before latching.
            gbUpdateMBC3RTC(cartridge);
            
            for (size_t i = 0; i < 5; ++i)
            {
                cartridge->rtcLatchedRegisters[i] = cartridge->rtcRegisters[i];
            }
        }

        cartridge->rtcLatchPrimed = (value == 0x00);
    }

    return true;
}

bool gbWriteMBC5CartridgeROM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    *outActual = 0xFF;

    // - `$0000 - $1FFF`: RAM Enable
    if (address < 0x2000)
    {
        // - Write `0xA` in the lower nibble to enable RAM.
        // - Write any other value to disable RAM.
        cartridge->ramEnabled = ((value & 0x0F) == 0x0A);
    }

    // - `$2000 - $2FFF`: ROM Bank Number (lower 8 bits)
    //   Note: MBC5 allows writing 0x00, and bank 0 will actually be selected.
    else if (address < 0x3000)
    {
        cartridge->romBankNumber = value;
    }

    // - `$3000 - $3FFF`: ROM Bank Number (9th bit, bit 8)
    //   We store this in the ramBankingEnabled flag as a workaround since
    //   romBankNumber is only uint8_t.
    else if (address < 0x4000)
    {
        cartridge->ramBankingEnabled = (value & 0x01) != 0;
    }

    // - `$4000 - $5FFF`: RAM Bank Number (4 bits: 0x00-0x0F)
    //   For rumble cartridges, bit 3 controls the rumble motor.
    else if (address < 0x6000)
    {
        cartridge->ramBankNumber = value & 0x0F;
        
        // - If this is a rumble cartridge, bit 3 controls the rumble motor.
        //   (In a real implementation, this would activate hardware rumble)
        if (cartridge->hasRumble)
        {
            // bool rumbleActive = (value & 0x08) != 0;
            // TODO: Trigger rumble hardware if needed
        }
    }

    return true;
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

    *outActual = 0xFF;

    // - If RAM is disabled, writes are ignored.
    if (!cartridge->ramEnabled)
    {
        return true;
    }

    // - If there is no RAM, writes are ignored.
    if (cartridge->ramData == nullptr)
    {
        return true;
    }

    // - If RAM banking mode is disabled (mode 0), this area always maps to RAM bank 0.
    // - Also, RAM banking only works with 32KB RAM (4 banks). Smaller RAM sizes
    //   always use bank 0.
    if (!cartridge->ramBankingEnabled || cartridge->ramSize <= GB_EXTRAM_SIZE)
    {
        cartridge->ramData[address] = value;
        *outActual = value;
    }

    // - If RAM banking mode is enabled (mode 1) and RAM is 32KB, this area maps
    //   to the RAM bank selected by the secondary 2-bit register (ramBankNumber).
    else
    {
        size_t maxRamBank = (cartridge->ramSize / GB_EXTRAM_SIZE) - 1;
        size_t bankNumber = cartridge->ramBankNumber & maxRamBank;
        size_t bankOffset = bankNumber * GB_EXTRAM_SIZE;
        cartridge->ramData[bankOffset + address] = value;
        *outActual = value;
    }

    return true;
}

bool gbWriteMBC2CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    *outActual = 0xFF;

    // - If RAM is disabled, writes are ignored.
    if (!cartridge->ramEnabled)
    {
        return true;
    }

    // - MBC2 has built-in RAM (512 × 4-bit values, 512 bytes total).
    // - Only the lower 9 bits of the address are used, so RAM repeats every 512 bytes.
    // - Only the lower 4 bits of each byte are used; upper 4 bits should be ignored.
    if (cartridge->ramData == nullptr)
    {
        return true;
    }

    size_t ramAddress = address & 0x01FF;  // Lower 9 bits (0-511)
    
    // - Store only the lower 4 bits; upper 4 bits are ignored
    cartridge->ramData[ramAddress] = value & 0x0F;
    *outActual = value & 0x0F;

    return true;
}

bool gbWriteMBC3CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    *outActual = 0xFF;

    // - If RAM/RTC access is disabled, writes are ignored.
    if (!cartridge->ramEnabled)
    {
        return true;
    }

    // - If an RTC register is selected, write to the RTC registers.
    if (cartridge->ramBankNumber >= 0x08 &&
        cartridge->ramBankNumber <= 0x0C)
    {
        uint8_t rtcIndex = cartridge->ramBankNumber - 0x08;
        cartridge->rtcRegisters[rtcIndex] = value;
        *outActual = value;

        // - Special handling for DH register (index 4): extract flags and day counter bit 8.
        if (rtcIndex == 4)
        {
            // - Bit 0: Day counter bit 8.
            // - Bit 6: Halt flag (0=running, 1=stopped).
            // - Bit 7: Carry bit (day counter overflow).
            cartridge->rtcHalted = (value & 0x40) != 0;
            cartridge->rtcCarryBit = (value & 0x80) != 0;
            
            // - Update the full 9-bit day counter from DL and DH bit 0.
            uint16_t dayHigh = (uint16_t)(value & 0x01) << 8;
            uint16_t dayLow = cartridge->rtcRegisters[3];
            cartridge->rtcDayCounter = dayHigh | dayLow;
        }
        // - Update day counter when DL register (index 3) is written.
        else if (rtcIndex == 3)
        {
            uint16_t dayHigh = (uint16_t)(cartridge->rtcRegisters[4] & 0x01) << 8;
            cartridge->rtcDayCounter = dayHigh | value;
        }
    }

    // - If a RAM bank is selected, write to the RAM.
    if (cartridge->ramBankNumber <= 0x03)
    {
        // - If there is no RAM, writes are ignored.
        if (cartridge->ramData == nullptr)
        {
            return true;
        }

        size_t maxRamBank = (cartridge->ramSize / GB_EXTRAM_SIZE) - 1;
        size_t bankNumber = cartridge->ramBankNumber & maxRamBank;
        size_t bankOffset = bankNumber * GB_EXTRAM_SIZE;
        cartridge->ramData[bankOffset + address] = value;
        *outActual = value;
    }

    return true;
}

bool gbWriteMBC5CartridgeRAM (gbCartridge* cartridge,
    uint16_t address, uint8_t value, uint8_t* outActual)
{
    gbAssert(cartridge != nullptr);
    gbAssert(outActual != nullptr);

    *outActual = 0xFF;

    // - If RAM is disabled, writes are ignored.
    if (!cartridge->ramEnabled)
    {
        return true;
    }

    // - If there is no RAM, writes are ignored.
    if (cartridge->ramData == nullptr)
    {
        return true;
    }

    // - MBC5 supports RAM banks 0x00-0x0F (4 bits).
    //   For rumble cartridges, bit 3 of ramBankNumber controls the rumble motor,
    //   so only bits 0-2 are used for RAM banking (0x00-0x07).
    size_t maxRamBank = (cartridge->ramSize / GB_EXTRAM_SIZE) - 1;
    size_t bankNumber = cartridge->ramBankNumber & 0x0F;  // 4-bit bank number
    
    // - For rumble cartridges, mask to 3 bits instead of 4
    if (cartridge->hasRumble)
    {
        bankNumber &= 0x07;
    }
    
    bankNumber &= maxRamBank;
    size_t bankOffset = bankNumber * GB_EXTRAM_SIZE;
    cartridge->ramData[bankOffset + address] = value;
    *outActual = value;

    return true;
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
