/**
 * @file    GB/Context.c
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains definitions for the Game Boy Emulator Core context.
 */

/* Private Includes ***********************************************************/

#include <GB/Context.h>

/* Private Unions and Structures **********************************************/

struct gbContext
{
    // Userdata
    void*                userdata;

    // Callbacks
    gbBusReadCallback    busReadCallback;
    gbBusWriteCallback   busWriteCallback;
};

/* Private Static Variables ***************************************************/

/**
 * @brief   A pointer to the Game Boy Emulator Core context designated as the
 *          "current context" for operations that do not explicitly receive a
 *          context parameter.
 */
static gbContext* s_currentContext = nullptr;

/* Public Function Definitions ************************************************/

gbContext* gbCreateContext ()
{
    gbContext* context = gbCreateZero(1, gbContext);
    gbCheckpv(context, nullptr, "Error allocating memory for 'gbContext'");

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

    // - Store the read value here.
    uint8_t value = 0xFF;

    // - Check address range and read from appropriate component.
    //
    // - `$0000` - `$7FFF`: Attached catridge device ROM
    if (address <= GB_ROMX_END)
    {

    }

    // - `$8000` - `$9FFF`: Video RAM
    else if (address >= GB_VRAM_START && address <= GB_VRAM_END)
    {

    }

    // - `$A000` - `$BFFF`: Attached cartridge device RAM
    else if (address >= GB_EXTRAM_START && address <= GB_EXTRAM_END)
    {

    }

    // - `$C000` - `$DFFF`: Work RAM
    else if (address >= GB_WRAM0_START && address <= GB_WRAMX_END)
    {

    }

    // - `$E000` - `$FDFF`: Echo RAM (mirror of `$C000` - `$DDFF`)
    else if (address >= GB_ECHO_START && address <= GB_ECHO_END)
    {

    }

    // - `$FE00` - `$FE9F`: Object Attribute Memory (OAM)
    else if (address >= GB_OAM_START && address <= GB_OAM_END)
    {

    }

    // - `$FEA0` - `$FEFF`: Unusable memory area
    else if (address >= GB_UNUSED_START && address <= GB_UNUSED_END)
    {

    }

    // - `$FF80` - `$FFFE`: High RAM (HRAM)
    else if (address >= GB_HRAM_START && address <= GB_HRAM_END)
    {

    }

    // - Port I/O Registers
    switch (address)
    {
        case GB_PR_P1:
        case GB_PR_SB:
        case GB_PR_SC:
        case GB_PR_DIV:
        case GB_PR_TIMA:
        case GB_PR_TMA:
        case GB_PR_TAC:
        case GB_PR_IF:
        case GB_PR_NR10:
        case GB_PR_NR11:
        case GB_PR_NR12:
        case GB_PR_NR13:
        case GB_PR_NR14:
        case GB_PR_NR21:
        case GB_PR_NR22:
        case GB_PR_NR23:
        case GB_PR_NR24:
        case GB_PR_NR30:
        case GB_PR_NR31:
        case GB_PR_NR32:
        case GB_PR_NR33:
        case GB_PR_NR34:
        case GB_PR_NR41:
        case GB_PR_NR42:
        case GB_PR_NR43:
        case GB_PR_NR44:
        case GB_PR_NR50:
        case GB_PR_NR51:
        case GB_PR_NR52:
        case GB_PR_LCDC:
        case GB_PR_STAT:
        case GB_PR_SCY:
        case GB_PR_SCX:
        case GB_PR_LY:
        case GB_PR_LYC:
        case GB_PR_DMA:
        case GB_PR_BGP:
        case GB_PR_OBP0:
        case GB_PR_OBP1:
        case GB_PR_WY:
        case GB_PR_WX:
        case GB_PR_KEY0:
        case GB_PR_KEY1:
        case GB_PR_VBK:
        case GB_PR_BANK:
        case GB_PR_HDMA1:
        case GB_PR_HDMA2:
        case GB_PR_HDMA3:
        case GB_PR_HDMA4:
        case GB_PR_HDMA5:
        case GB_PR_RP:
        case GB_PR_BCPS:
        case GB_PR_BCPD:
        case GB_PR_OCPS:
        case GB_PR_OCPD:
        case GB_PR_OPRI:
        case GB_PR_SVBK:
        case GB_PR_PCM12:
        case GB_PR_PCM34:
        case GB_PR_IE:
        default:            break;
    }

    // - If a bus read callback is set, invoke it.
    if (context->busReadCallback != nullptr)
    {
        context->busReadCallback(context, address, value);
    }

    *outValue = value;
    return true;
}

bool gbWriteByte (gbContext* context, uint16_t address, uint8_t value, 
    const gbCheckRules* rules, uint8_t* outActual)
{
    gbFallback(context, gbGetCurrentContext());
    gbCheckv(context != nullptr, false,
        "No valid 'gbContext' provided, and no current context is set.");

    // - Store the actual written value here.
    uint8_t actual = 0xFF;
    
    // - Check address range and write to appropriate component.
    //
    // - `$0000` - `$7FFF`: Attached catridge device ROM
    if (address <= GB_ROMX_END)
    {

    }

    // - `$8000` - `$9FFF`: Video RAM
    else if (address >= GB_VRAM_START && address <= GB_VRAM_END)
    {

    }

    // - `$A000` - `$BFFF`: Attached cartridge device RAM
    else if (address >= GB_EXTRAM_START && address <= GB_EXTRAM_END)
    {

    }

    // - `$C000` - `$DFFF`: Work RAM
    else if (address >= GB_WRAM0_START && address <= GB_WRAMX_END)
    {

    }

    // - `$E000` - `$FDFF`: Echo RAM (mirror of `$C000` - `$DDFF`)
    else if (address >= GB_ECHO_START && address <= GB_ECHO_END)
    {

    }

    // - `$FE00` - `$FE9F`: Object Attribute Memory (OAM)
    else if (address >= GB_OAM_START && address <= GB_OAM_END)
    {

    }

    // - `$FEA0` - `$FEFF`: Unusable memory area
    else if (address >= GB_UNUSED_START && address <= GB_UNUSED_END)
    {

    }

    // - `$FF80` - `$FFFE`: High RAM (HRAM)
    else if (address >= GB_HRAM_START && address <= GB_HRAM_END)
    {

    }

    // - Port I/O Registers
    switch (address)
    {
        case GB_PR_P1:
        case GB_PR_SB:
        case GB_PR_SC:
        case GB_PR_DIV:
        case GB_PR_TIMA:
        case GB_PR_TMA:
        case GB_PR_TAC:
        case GB_PR_IF:
        case GB_PR_NR10:
        case GB_PR_NR11:
        case GB_PR_NR12:
        case GB_PR_NR13:
        case GB_PR_NR14:
        case GB_PR_NR21:
        case GB_PR_NR22:
        case GB_PR_NR23:
        case GB_PR_NR24:
        case GB_PR_NR30:
        case GB_PR_NR31:
        case GB_PR_NR32:
        case GB_PR_NR33:
        case GB_PR_NR34:
        case GB_PR_NR41:
        case GB_PR_NR42:
        case GB_PR_NR43:
        case GB_PR_NR44:
        case GB_PR_NR50:
        case GB_PR_NR51:
        case GB_PR_NR52:
        case GB_PR_LCDC:
        case GB_PR_STAT:
        case GB_PR_SCY:
        case GB_PR_SCX:
        case GB_PR_LY:
        case GB_PR_LYC:
        case GB_PR_DMA:
        case GB_PR_BGP:
        case GB_PR_OBP0:
        case GB_PR_OBP1:
        case GB_PR_WY:
        case GB_PR_WX:
        case GB_PR_KEY0:
        case GB_PR_KEY1:
        case GB_PR_VBK:
        case GB_PR_BANK:
        case GB_PR_HDMA1:
        case GB_PR_HDMA2:
        case GB_PR_HDMA3:
        case GB_PR_HDMA4:
        case GB_PR_HDMA5:
        case GB_PR_RP:
        case GB_PR_BCPS:
        case GB_PR_BCPD:
        case GB_PR_OCPS:
        case GB_PR_OCPD:
        case GB_PR_OPRI:
        case GB_PR_SVBK:
        case GB_PR_PCM12:
        case GB_PR_PCM34:
        case GB_PR_IE:
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

    return true;
}
