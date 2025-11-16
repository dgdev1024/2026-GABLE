/**
 * @file    GB/Memory.c
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-16
 * 
 * @brief   Contains implementations for the Game Boy Emulator Core's internal,
 *          random-access memory (RAM) component.
 */

/* Private Includes ***********************************************************/

#include <GB/Memory.h>

/* Private Constants and Enumerations *****************************************/

/**
 * @brief   Defines the number of work RAM (WRAM) banks available in Engine Mode.
 */
#define GB_WRAM_BANK_COUNT 0xFF

/**
 * @brief   Defines the total size of work RAM (WRAM) in Engine Mode, in bytes.
 */
#define GB_WRAM_TOTAL_SIZE (GB_WRAM_BANK_SIZE * GB_WRAM_BANK_COUNT)

/* Private Unions and Structures **********************************************/

struct gbMemory
{
    // Parent Context
    gbContext*  parent;

    // Memory
    uint8_t    wram[GB_WRAM_TOTAL_SIZE];
    uint8_t    hram[GB_HRAM_SIZE];

    // Hardware Registers
    gbRegisterSVBK  svbk;

};

/* Public Function Definitions ************************************************/

gbMemory* gbCreateMemory (gbContext* parentContext)
{
    gbCheckv(parentContext != nullptr, nullptr, "Parent context pointer is null");

    gbMemory* memory = gbCreateZero(1, gbMemory);
    gbCheckpv(memory != nullptr, nullptr, "Error allocating memory for 'gbMemory'");

    memory->parent = parentContext;
    return memory;
}

bool gbDestroyMemory (gbMemory* memory)
{
    gbCheckqv(memory, false);
    gbDestroy(memory);
    return true;
}

bool gbInitializeMemory (gbMemory* memory)
{
    gbCheckqv(memory, false);

    // Initialize Memory Buffers
    memset(memory->wram, 0, sizeof(memory->wram));
    memset(memory->hram, 0, sizeof(memory->hram));

    // Initialize Hardware Registers
    memory->svbk.raw = 1;

    return true;
}

/* Public Function Definitions - Memory Access ********************************/

bool gbReadWorkRAM (const gbMemory* memory, uint16_t relativeAddress,
    uint8_t* outValue, const gbCheckRules* rules)
{
    gbCheckv(memory != nullptr, false, "Memory pointer is null");
    gbCheckv(memory->parent != nullptr, false,
        "Parent context pointer is null");
    gbCheckv(outValue != nullptr, false,
        "Output value pointer is null");
        
    // - Check for CGB Mode and Engine Mode
    bool isCGBMode = false, isEngineMode = false;
    gbCheckCGBMode(memory->parent, &isCGBMode);
    gbCheckEngineMode(memory->parent, &isEngineMode);

    // - Determine active WRAM bank.
    //   - In Engine Mode, use all 8 bits of the SVBK register, mapping `0` to `1`.
    //   - In CGB Mode, use only bits 0-2 of the SVBK register, mapping `0` to `1`.
    //   - In non-CGB Mode, always use bank `1`.
    uint8_t wramBank = 1;
    if (isEngineMode)
        { wramBank = memory->svbk.raw; }
    else if (isCGBMode)
        { wramBank = memory->svbk.wramBank; }
    if (wramBank == 0) { wramBank = 1; }

    // - Calculate absolute address within WRAM.
    size_t absoluteAddress = (wramBank * GB_WRAM_BANK_SIZE) +
        (relativeAddress % GB_WRAM_BANK_SIZE);

    // - Read value from WRAM.
    *outValue = memory->wram[absoluteAddress];
    return true;
}

bool gbReadHighRAM (const gbMemory* memory, uint16_t relativeAddress,
    uint8_t* outValue, const gbCheckRules* rules)
{
    gbCheckv(memory != nullptr, false, "Memory pointer is null");
    gbCheckv(memory->parent != nullptr, false,
        "Parent context pointer is null");
    gbCheckv(outValue != nullptr, false,
        "Output value pointer is null");

    // - Read value from HRAM.
    *outValue = memory->hram[relativeAddress % GB_HRAM_SIZE];
    return true;
}

bool gbWriteWorkRAM (gbMemory* memory, uint16_t relativeAddress,
    uint8_t value, uint8_t* outActual, const gbCheckRules* rules)
{
    gbCheckv(memory != nullptr, false, "Memory pointer is null");
    gbCheckv(memory->parent != nullptr, false,
        "Parent context pointer is null");
    gbCheckv(outActual != nullptr, false,
        "Output actual value pointer is null");

    // - Check for CGB Mode and Engine Mode
    bool isCGBMode = false, isEngineMode = false;
    gbCheckCGBMode(memory->parent, &isCGBMode);
    gbCheckEngineMode(memory->parent, &isEngineMode);
    
    // - Determine active WRAM bank.
    //   - In Engine Mode, use all 8 bits of the SVBK register, mapping `0` to `1`.
    //   - In CGB Mode, use only bits 0-2 of the SVBK register, mapping `0` to `1`.
    //   - In non-CGB Mode, always use bank `1`.
    uint8_t wramBank = 1;
    if (isEngineMode)
        { wramBank = memory->svbk.raw; }
    else if (isCGBMode)
        { wramBank = memory->svbk.wramBank; }
    if (wramBank == 0) { wramBank = 1; }

    // - Calculate absolute address within WRAM.
    size_t absoluteAddress = (wramBank * GB_WRAM_BANK_SIZE) +
        (relativeAddress % GB_WRAM_BANK_SIZE);

    // - Write value to WRAM.
    memory->wram[absoluteAddress] = value;
    *outActual = value;
    return true;
}

bool gbWriteHighRAM (gbMemory* memory, uint16_t relativeAddress,
    uint8_t value, uint8_t* outActual, const gbCheckRules* rules)
{
    gbCheckv(memory != nullptr, false, "Memory pointer is null");
    gbCheckv(memory->parent != nullptr, false,
        "Parent context pointer is null");
    gbCheckv(outActual != nullptr, false,
        "Output actual value pointer is null");

    // - Write value to HRAM.
    memory->hram[relativeAddress % GB_HRAM_SIZE] = value;
    *outActual = value;
    return true;
}

/* Public Function Definitions - Hardware Register Access *********************/

bool gbReadSVBK (const gbMemory* memory, uint8_t* outValue,
    const gbCheckRules* rules)
{
    gbCheckv(memory != nullptr, false, "Memory pointer is null");
    gbCheckv(memory->parent != nullptr, false,
        "Parent context pointer is null");
    gbCheckv(outValue != nullptr, false,
        "Output value pointer is null");
        
    // - Check for CGB Mode and Engine Mode
    bool isCGBMode = false, isEngineMode = false;
    gbCheckCGBMode(memory->parent, &isCGBMode);
    gbCheckEngineMode(memory->parent, &isEngineMode);

    // - In Engine Mode:
    //   - All bits of `SVBK` are readable.
    // - In CGB Mode:
    //   - Bits 3-7 are unused and read as `1`.
    //   - Bits 0-2 are readable.
    // - In non-CGB Mode:
    //   - This register is not available and reads open-bus.
    if (isEngineMode)
    {
        *outValue = memory->svbk.raw;
    }
    else if (isCGBMode)
    {
        *outValue =
            (0b11111000) |                      // Bits 3-7 are unused; read as `1`
            (memory->svbk.raw & 0b00000111);    // Bits 0-2 are readable as-is
    }
    else
    {
        *outValue = 0xFF;
    }

    return true;
}

bool gbWriteSVBK (gbMemory* memory, uint8_t value, uint8_t* outActual, 
    const gbCheckRules* rules)
{
    gbCheckv(memory != nullptr, false, "Memory pointer is null");
    gbCheckv(memory->parent != nullptr, false,
        "Parent context pointer is null");
    gbCheckv(outActual != nullptr, false,
        "Output actual value pointer is null");
        
    // - Check for CGB Mode and Engine Mode
    bool isCGBMode = false, isEngineMode = false;
    gbCheckCGBMode(memory->parent, &isCGBMode);
    gbCheckEngineMode(memory->parent, &isEngineMode);

    // - In Engine Mode:
    //   - All bits of `SVBK` are writable.
    // - In CGB Mode:
    //   - Bits 3-7 are unused and should be written as `1`.
    //   - Bits 0-2 are writable.
    // - In non-CGB Mode:
    //   - This register is not available and writes are ignored.
    if (isEngineMode)
    {
        memory->svbk.raw = value;
        *outActual = value;
    }
    else if (isCGBMode)
    {
        memory->svbk.raw =
            (0b11111000) |                      // Bits 3-7 are unused; write as `1`
            (value & 0b00000111);               // Bits 0-2 are writable as-is
        *outActual = memory->svbk.raw;
    }
    else
    {
        *outActual = 0xFF;
    }

    return true;
}
