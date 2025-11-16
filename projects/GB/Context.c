/**
 * @file    GB/Context.c
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains definitions for the Game Boy Emulator Core context.
 */

/* Private Includes ***********************************************************/

#include <GB/Cartridge.h>
#include <GB/Memory.h>
#include <GB/Context.h>

/* Private Unions and Structures **********************************************/

struct gbContext
{
    // Userdata
    void*               userdata;

    // Callbacks
    gbBusReadCallback   busReadCallback;
    gbBusWriteCallback  busWriteCallback;

    // Components
    gbCartridge*        cartridge;
    gbMemory*           memory;

    // Internal State
    bool                engineMode;
};

/* Private Static Variables ***************************************************/

/**
 * @brief   A pointer to the Game Boy Emulator Core context designated as the
 *          "current context" for operations that do not explicitly receive a
 *          context parameter.
 */
static gbContext* s_currentContext = nullptr;

/* Public Function Definitions ************************************************/

gbContext* gbCreateContext (bool engineMode)
{
    gbContext* context = gbCreateZero(1, gbContext);
    gbCheckpv(context, nullptr, "Error allocating memory for 'gbContext'");

    if (
        (context->memory = gbCreateMemory(context)) == nullptr
    )
    {
        gbDestroyContext(context);
        return nullptr;
    }

    context->engineMode = engineMode;
    gbInitializeContext(context);
    return context;
}

bool gbDestroyContext (gbContext* context)
{
    gbCheckqv(context, false);

    // - Un-set userdata pointer to avoid dangling references.
    context->userdata = nullptr;

    gbDestroy(context);
    return true;
}

bool gbInitializeContext (gbContext* context)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");

    return
        gbInitializeMemory(context->memory);
}

/* Public Functions - Cartridge ***********************************************/

bool gbAttachCartridge (gbContext* context, gbCartridge* cartridge)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");

    // - Attach the cartridge to the context, then re-initialize the context.
    context->cartridge = cartridge;
    return gbInitializeContext(context);
}

/* Public Functions - Context Operation Mode **********************************/

bool gbCheckCGBMode (const gbContext* context, bool* outIsCGBMode)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");
    gbCheckv(outIsCGBMode != nullptr, false,
        "No valid output pointer provided for CGB mode check.");

    // - If in engine mode, then CGB mode is forced.
    if (context->engineMode)
    {
        *outIsCGBMode = true;
        return true;
    }

    // - If no cartridge is attached, then assume non-CGB mode.
    if (context->cartridge == nullptr)
    {
        *outIsCGBMode = false;
        return true;
    }

    // - Check the attached cartridge's header for CGB support/requirement.
    const gbCartridgeHeader* header = gbGetCartridgeHeader(context->cartridge);
    gbCheckCartridgeCGBSupport(header, outIsCGBMode, nullptr);

    return true;
}

bool gbCheckEngineMode (const gbContext* context, bool* outIsEngineMode)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");
    gbCheckv(outIsEngineMode != nullptr, false,
        "No valid output pointer provided for engine mode check.");

    *outIsEngineMode = context->engineMode;
    return true;
}

/* Public Functions - Current Context *****************************************/

bool gbMakeContextCurrent (gbContext* context)
{
    s_currentContext = context;
    return (s_currentContext != nullptr);
}

gbContext* gbGetCurrentContext ()
{
    return s_currentContext;
}

/* Public Functions - Userdata ************************************************/

bool gbSetUserdata (gbContext* context, void* userdata)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");

    context->userdata = userdata;
    return true;
}

void* gbGetUserdata (const gbContext* context)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, nullptr,
        "No valid 'gbContext' provided, and no current context is set.");

    return context->userdata;
}

/* Public Functions - Callbacks ***********************************************/

bool gbSetBusReadCallback (gbContext* context, gbBusReadCallback callback)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");

    context->busReadCallback = callback;
    return true;
}

bool gbSetBusWriteCallback (gbContext* context, gbBusWriteCallback callback)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");

    context->busWriteCallback = callback;
    return true;
}

/* Public Functions - Address Bus *********************************************/

bool gbReadByte (const gbContext* context, uint16_t address, uint8_t* outValue, 
    const gbCheckRules* rules)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");
    gbCheckv(outValue != nullptr, false,
        "No valid output pointer provided for read value.");

    // - Store the read value and result here.
    uint8_t value = 0xFF;
    bool result = false;

    // - Check address range and read from appropriate component.
    //
    // - `$0000` - `$7FFF`: Attached catridge device ROM
    if (address <= GB_ROMX_END)
    {
        if (context->cartridge != nullptr)
        {
            result = gbReadCartridgeROM(context->cartridge, address, &value);
        }
    }

    // - `$8000` - `$9FFF`: Video RAM
    else if (address >= GB_VRAM_START && address <= GB_VRAM_END)
    {

    }

    // - `$A000` - `$BFFF`: Attached cartridge device RAM
    else if (address >= GB_EXTRAM_START && address <= GB_EXTRAM_END)
    {
        if (context->cartridge != nullptr)
        {
            result = gbReadCartridgeRAM(context->cartridge, 
                address - GB_EXTRAM_START, &value);
        }
    }

    // - `$C000` - `$DFFF`: Work RAM
    else if (address >= GB_WRAM0_START && address <= GB_WRAMX_END)
    {
        result = gbReadWorkRAM(context->memory, 
            address - GB_WRAM0_START, &value, rules);
    }

    // - `$E000` - `$FDFF`: Echo RAM (mirror of `$C000` - `$DDFF`)
    else if (address >= GB_ECHO_START && address <= GB_ECHO_END)
    {
        if (context->engineMode == true)
        {
            // - In Engine Mode, something else will be mapped to this space.
            //   For now, return open-bus.
            result = true;
        }
        else
        {
            result = gbReadWorkRAM(context->memory, 
                address - GB_ECHO_START, &value, rules);
        }
    }

    // - `$FE00` - `$FE9F`: Object Attribute Memory (OAM)
    else if (address >= GB_OAM_START && address <= GB_OAM_END)
    {

    }

    // - `$FEA0` - `$FEFF`: Unusable memory area
    else if (address >= GB_UNUSED_START && address <= GB_UNUSED_END)
    {
        if (context->engineMode == true)
        {
            // - In Engine Mode, something else will be mapped to this space.
            //   For now, return open-bus.
            result = true;
        }
        else
        {
            bool isCGBMode = false;
            gbCheckCGBMode(context, &isCGBMode);

            if (isCGBMode == true)
            {
                // - In CGB mode, revision E is assumed. Reads from this area
                //   return a byte value in which both the upper and lower nibbles
                //   are equal to the second-to-last nibble of the read address.
                uint8_t nibble = (address >> 4) & 0x0F;
                value = (nibble << 4) | nibble;
                result = true;
            }
            else
            {
                // - In DMG mode, if a read from this area is attempted while
                //   the PPU is undergoing an OAM DMA transfer, the OAM corruption
                //   bug occurs. Otherwise, reads return open-bus.
                // - For now, since the PPU is not in place, yet, assume no
                //   OAM DMA transfer is occurring and return open-bus.
                result = true;
            }
        }
    }

    // - `$FF80` - `$FFFE`: High RAM (HRAM)
    else if (address >= GB_HRAM_START && address <= GB_HRAM_END)
    {
        result = gbReadHighRAM(context->memory, 
            address - GB_HRAM_START, &value, rules);
    }

    // - Port I/O Registers
    switch (address)
    {
        case GB_PR_P1:      break;
        case GB_PR_SB:      break;
        case GB_PR_SC:      break;
        case GB_PR_DIV:     break;
        case GB_PR_TIMA:    break;
        case GB_PR_TMA:     break;
        case GB_PR_TAC:     break;
        case GB_PR_IF:      break;
        case GB_PR_NR10:    break;
        case GB_PR_NR11:    break;
        case GB_PR_NR12:    break;
        case GB_PR_NR13:    break;
        case GB_PR_NR14:    break;
        case GB_PR_NR21:    break;
        case GB_PR_NR22:    break;
        case GB_PR_NR23:    break;
        case GB_PR_NR24:    break;
        case GB_PR_NR30:    break;
        case GB_PR_NR31:    break;
        case GB_PR_NR32:    break;
        case GB_PR_NR33:    break;
        case GB_PR_NR34:    break;
        case GB_PR_NR41:    break;
        case GB_PR_NR42:    break;
        case GB_PR_NR43:    break;
        case GB_PR_NR44:    break;
        case GB_PR_NR50:    break;
        case GB_PR_NR51:    break;
        case GB_PR_NR52:    break;
        case GB_PR_LCDC:    break;
        case GB_PR_STAT:    break;
        case GB_PR_SCY:     break;
        case GB_PR_SCX:     break;
        case GB_PR_LY:      break;
        case GB_PR_LYC:     break;
        case GB_PR_DMA:     break;
        case GB_PR_BGP:     break;
        case GB_PR_OBP0:    break;
        case GB_PR_OBP1:    break;
        case GB_PR_WY:      break;
        case GB_PR_WX:      break;
        case GB_PR_KEY0:    break;
        case GB_PR_KEY1:    break;
        case GB_PR_VBK:     break;
        case GB_PR_BANK:    break;
        case GB_PR_HDMA1:   break;
        case GB_PR_HDMA2:   break;
        case GB_PR_HDMA3:   break;
        case GB_PR_HDMA4:   break;
        case GB_PR_HDMA5:   break;
        case GB_PR_RP:      break;
        case GB_PR_BCPS:    break;
        case GB_PR_BCPD:    break;
        case GB_PR_OCPS:    break;
        case GB_PR_OCPD:    break;
        case GB_PR_OPRI:    break;
        case GB_PR_SVBK:    result = gbReadSVBK(context->memory, &value, rules); break;
        case GB_PR_PCM12:   break;
        case GB_PR_PCM34:   break;
        case GB_PR_IE:      break;
        default:            break;
    }

    // - If a bus read callback is set, invoke it.
    if (context->busReadCallback != nullptr)
    {
        context->busReadCallback(context, address, value);
    }

    *outValue = value;
    return result;
}

bool gbWriteByte (gbContext* context, uint16_t address, uint8_t value, 
    const gbCheckRules* rules, uint8_t* outActual)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");

    // - Store the actual written value and result here.
    uint8_t actual = 0xFF;
    bool result = false;
    
    // - Check address range and write to appropriate component.
    //
    // - `$0000` - `$7FFF`: Attached catridge device ROM
    if (address <= GB_ROMX_END)
    {
        if (context->cartridge != nullptr)
        {
            result = gbWriteCartridgeROM(context->cartridge, address, value, 
                &actual);
        }
    }

    // - `$8000` - `$9FFF`: Video RAM
    else if (address >= GB_VRAM_START && address <= GB_VRAM_END)
    {

    }

    // - `$A000` - `$BFFF`: Attached cartridge device RAM
    else if (address >= GB_EXTRAM_START && address <= GB_EXTRAM_END)
    {
        if (context->cartridge != nullptr)
        {
            result = gbWriteCartridgeRAM(context->cartridge, 
                address - GB_EXTRAM_START, value, &actual);
        }
    }

    // - `$C000` - `$DFFF`: Work RAM
    else if (address >= GB_WRAM0_START && address <= GB_WRAMX_END)
    {
        result = gbWriteWorkRAM(context->memory, 
            address - GB_WRAM0_START, value, &actual, rules);
    }

    // - `$E000` - `$FDFF`: Echo RAM (mirror of `$C000` - `$DDFF`)
    else if (address >= GB_ECHO_START && address <= GB_ECHO_END)
    {
        if (context->engineMode == true)
        {
            // - In Engine Mode, something else will be mapped to this space.
            //   For now, do nothing.
            result = true;
        }
        else
        {
            result = gbWriteWorkRAM(context->memory, 
                address - GB_ECHO_START, value, &actual, rules);
        }
    }

    // - `$FE00` - `$FE9F`: Object Attribute Memory (OAM)
    else if (address >= GB_OAM_START && address <= GB_OAM_END)
    {

    }

    // - `$FEA0` - `$FEFF`: Unusable memory area
    else if (address >= GB_UNUSED_START && address <= GB_UNUSED_END)
    {
        if (context->engineMode == true)
        {
            // - In Engine Mode, something else will be mapped to this space.
            //   For now, do nothing.
            result = true;
        }
        else
        {
            // - Otherwise, writes are ignored.
            result = true;
        }
    }

    // - `$FF80` - `$FFFE`: High RAM (HRAM)
    else if (address >= GB_HRAM_START && address <= GB_HRAM_END)
    {
        result = gbWriteHighRAM(context->memory, 
            address - GB_HRAM_START, value, &actual, rules);
    }

    // - Port I/O Registers
    switch (address)
    {
        case GB_PR_P1:      break;
        case GB_PR_SB:      break;
        case GB_PR_SC:      break;
        case GB_PR_DIV:     break;
        case GB_PR_TIMA:    break;
        case GB_PR_TMA:     break;
        case GB_PR_TAC:     break;
        case GB_PR_IF:      break;
        case GB_PR_NR10:    break;
        case GB_PR_NR11:    break;
        case GB_PR_NR12:    break;
        case GB_PR_NR13:    break;
        case GB_PR_NR14:    break;
        case GB_PR_NR21:    break;
        case GB_PR_NR22:    break;
        case GB_PR_NR23:    break;
        case GB_PR_NR24:    break;
        case GB_PR_NR30:    break;
        case GB_PR_NR31:    break;
        case GB_PR_NR32:    break;
        case GB_PR_NR33:    break;
        case GB_PR_NR34:    break;
        case GB_PR_NR41:    break;
        case GB_PR_NR42:    break;
        case GB_PR_NR43:    break;
        case GB_PR_NR44:    break;
        case GB_PR_NR50:    break;
        case GB_PR_NR51:    break;
        case GB_PR_NR52:    break;
        case GB_PR_LCDC:    break;
        case GB_PR_STAT:    break;
        case GB_PR_SCY:     break;
        case GB_PR_SCX:     break;
        case GB_PR_LY:      break;
        case GB_PR_LYC:     break;
        case GB_PR_DMA:     break;
        case GB_PR_BGP:     break;
        case GB_PR_OBP0:    break;
        case GB_PR_OBP1:    break;
        case GB_PR_WY:      break;
        case GB_PR_WX:      break;
        case GB_PR_KEY0:    break;
        case GB_PR_KEY1:    break;
        case GB_PR_VBK:     break;
        case GB_PR_BANK:    break;
        case GB_PR_HDMA1:   break;
        case GB_PR_HDMA2:   break;
        case GB_PR_HDMA3:   break;
        case GB_PR_HDMA4:   break;
        case GB_PR_HDMA5:   break;
        case GB_PR_RP:      break;
        case GB_PR_BCPS:    break;
        case GB_PR_BCPD:    break;
        case GB_PR_OCPS:    break;
        case GB_PR_OCPD:    break;
        case GB_PR_OPRI:    break;
        case GB_PR_SVBK:    result = gbReadSVBK(context->memory, &actual, rules); break;
        case GB_PR_PCM12:   break;
        case GB_PR_PCM34:   break;
        case GB_PR_IE:      break;
        default:            break;
    }

    // - If a bus write callback is set, invoke it.
    if (context->busWriteCallback != nullptr)
    {
        context->busWriteCallback(context, address, value, actual);
    }

    if (outActual != nullptr)
    {
        *outActual = actual;
    }

    return result;
}
