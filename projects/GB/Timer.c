/**
 * @file    GB/Timer.c
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-19
 * 
 * @brief   Contains definitions for the Game Boy Emulator Core's timer
 *          component.
 */

/* Private Includes ***********************************************************/

#include <GB/Timer.h>
#include <GB/Processor.h>

/* Private Unions and Structures **********************************************/

struct gbTimer
{
    // Parent Context
    gbContext*  parent;

    // Callbacks
    gbTimerOverflowCallback overflowCallback;

    // Hardware Registers
    uint16_t        div; 
    uint8_t         tima;
    uint8_t         tma; 
    gbRegisterTAC   tac; 

    // Internal State
    bool            isEngineMode;
    bool            isCGBMode;
    uint16_t        oldDivider;

};

/* Public Function Definitions ************************************************/

gbTimer* gbCreateTimer (gbContext* context)
{
    gbCheckv(context, nullptr, "Parent context pointer is null");

    gbTimer* timer = gbCreateZero(1, gbTimer);
    gbCheckpv(timer, nullptr, "Error allocating memory for 'gbTimer'");

    timer->parent = context;
    return timer;
}

bool gbDestroyTimer (gbTimer* timer)
{
    gbCheckqv(timer, false);
    gbDestroy(timer);
    return true;
}

bool gbInitializeTimer (gbTimer* timer)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");

    // - Check Engine and CGB Modes
    gbCheckCGBMode(timer->parent, &timer->isCGBMode);
    gbCheckEngineMode(timer->parent, &timer->isEngineMode);
    
    // - Initialize Hardware Registers
    timer->tima = 0x00;
    timer->tma = 0x00;
    timer->tac.raw = 0xF8;
    if (timer->isCGBMode == true)
    {
        timer->div = 0x0000;
    }
    else
    {
        timer->div = 0xAB00;
    }

    // - Initialize Internal State
    timer->oldDivider = timer->div;

    return true;
}

/* Public Function Definitions - Helper Functions *****************************/

bool gbCheckTimerDividerFallingEdge (const gbTimer* timer, uint8_t dividerBit,
    bool* outFallingEdge)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");
    gbCheckv(outFallingEdge != nullptr, false,
        "No valid output pointer provided for falling edge check.");

    uint8_t oldBit = (timer->oldDivider >> dividerBit) & 0x01;
    uint8_t newBit = (timer->div >> dividerBit) & 0x01;

    *outFallingEdge = (oldBit == 1 && newBit == 0);
    return true;
}

/* Public Function Definitions - Callbacks ************************************/

bool gbSetTimerOverflowCallback (gbTimer* timer,
    gbTimerOverflowCallback callback)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");

    timer->overflowCallback = callback;
    return true;
}

/* Public Function Definitions - Ticking **************************************/

bool gbTickTimer (gbTimer* timer)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");

    // - Check `STOP` and Speed Switch States.
    // - If either is active, early out.
    bool isStopped = false, isSwitchingSpeed = false;
    gbProcessor* processor = gbGetProcessor(timer->parent);
    gbCheckStopState(processor, &isStopped);
    gbCheckSpeedSwitchState(processor, &isSwitchingSpeed);
    if (isStopped == true || isSwitchingSpeed == true)
    {
        return true;
    }

    // - Increment Divider Register; Save Old Value.
    timer->oldDivider = timer->div++;

    // - Check Timer Enable State.
    if (timer->tac.enabled == false)
    {
        return true;
    }

    // - Determine Divider Bit to Monitor Based on Clock Speed.
    uint8_t dividerBit = 9; // Default to 4096 Hz
    switch (timer->tac.clockSpeed)
    {
        case GB_TCS_4096_HZ:    dividerBit = 9; break;
        case GB_TCS_262144_HZ:  dividerBit = 3; break;
        case GB_TCS_65536_HZ:   dividerBit = 5; break;
        case GB_TCS_16384_HZ:   dividerBit = 7; break;
    }

    // - Check for Double Speed Mode.
    bool isDoubleSpeed = false;
    gbCheckCurrentSpeedMode(processor, &isDoubleSpeed);
    if (isDoubleSpeed == true)
    {
        dividerBit--;
    }

    // - Check for Falling Edge on Selected Divider Bit.
    bool fallingEdge = false;
    gbCheckTimerDividerFallingEdge(timer, dividerBit, &fallingEdge);
    if (fallingEdge == false)
    {
        return true;
    }

    // - Increment TIMA Register and Handle Overflow.
    timer->tima++;
    if (timer->tima == 0x00)
    {
        // - TIMA Overflowed.
        timer->tima = timer->tma;

        // - Request Timer Interrupt.
        gbRequestInterrupt(processor, GB_INT_TIMER);

        // - Invoke Overflow Callback if Set.
        if (timer->overflowCallback != nullptr)
        {
            timer->overflowCallback(timer->parent);
        }
    }

    return true;
}

/* Private Function Definitions - Hardware Register Access ********************/

bool gbReadDIV (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");
    gbCheckv(outValue != nullptr, false,
        "No valid output pointer provided for DIV register read.");

    // - All 8 bits of `DIV` are readable.
    //   - These 8 bits are the upper byte of the internal 16-bit divider counter.
    *outValue = ((timer->div >> 8) & 0xFF);

    return true;
}

bool gbReadTIMA (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");
    gbCheckv(outValue != nullptr, false,
        "No valid output pointer provided for TIMA register read.");

    // - All 8 bits of `TIMA` are readable.
    *outValue = timer->tima;

    return true;
}

bool gbReadTMA (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");
    gbCheckv(outValue != nullptr, false,
        "No valid output pointer provided for TMA register read.");

    // - All 8 bits of `TMA` are readable.
    *outValue = timer->tma;

    return true;
}

bool gbReadTAC (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");
    gbCheckv(outValue != nullptr, false,
        "No valid output pointer provided for TAC register read.");

    // - Bits 3-7 are unused; read as `1`.
    // - Bits 0-2 are readable.
    *outValue =
        0b11111000 |                            // Bits 3-7 unused, read as `1`
        (timer->tac.raw & 0b00000111);          // Bits 0-2 readable

    return true;
}

bool gbWriteDIV (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");

    // - Input value ignored. Any write to `DIV` resets the internal divider
    //   counter to `0x0000`.
    timer->div = 0x0000;

    if (outActual != nullptr)
    {
        *outActual = 0x00;
    }

    return true;
}

bool gbWriteTIMA (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");

    // - All 8 bits of `TIMA` are writable.
    timer->tima = value;

    if (outActual != nullptr)
    {
        *outActual = timer->tima;
    }

    return true;
}

bool gbWriteTMA (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");

    // - All 8 bits of `TMA` are writable.
    timer->tma = value;

    if (outActual != nullptr)
    {
        *outActual = timer->tma;
    }

    return true;
}

bool gbWriteTAC (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* checkRules)
{
    gbFallback(timer, gbGetTimer(nullptr));
    gbCheckv(timer != nullptr, false,
        "No valid 'gbTimer' provided, and no current context is set.");

    // - Bits 3-7 are unused; write as `1`.
    // - Bits 0-2 are writable.
    timer->tac.raw =
        0b11111000 |                            // Bits 3-7 unused, write as `1`
        (value & 0b00000111);                   // Bits 0-2 writable

    if (outActual != nullptr)
    {
        *outActual = timer->tac.raw;
    }

    return true;
}
