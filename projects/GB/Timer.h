/**
 * @file    GB/Timer.h
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-19
 * 
 * @brief   Contains declarations for the Game Boy Emulator Core's timer
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Context.h>

/* Public Types and Forward Declarations **************************************/

/**
 * @brief   Defines a pointer to a function called by the Game Boy Emulator Core's
 *          timer component when its `TIMA` counter overflows.
 * 
 * @param   context     A pointer to the @a `gbContext` structure whose timer
 *                      component triggered the overflow.
 */
typedef void (*gbTimerOverflowCallback) (gbContext* context);

/* Public Constants and Enumerations ******************************************/

/**
 * @brief   Enumerates the four possible clock speeds for the Game Boy's timer
 *          component, as selectable by bits 0-1 of the `TAC` hardware register.
 */
typedef enum gbTimerClockSpeed
{
    GB_TCS_4096_HZ        = 0x00, /** @brief 4,096 Hz (256 M-cycles) */
    GB_TCS_262144_HZ      = 0x01, /** @brief 262,144 Hz (16 M-cycles) */
    GB_TCS_65536_HZ       = 0x02, /** @brief 65,536 Hz (64 M-cycles) */
    GB_TCS_16384_HZ       = 0x03  /** @brief 16,384 Hz (256 M-cycles) */
} gbTimerClockSpeed;

/* Public Unions and Structures ***********************************************/

/**
 * @brief   Defines a bitfield union representing the Game Boy timer's `TAC`
 *          hardware register, which controls the timer's operation - its clock
 *          speed and whether it is enabled or disabled.
 */
typedef union gbRegisterTAC
{
    struct
    {
        uint8_t clockSpeed  : 2;    /** @brief Bits 0-1: Clock speed; see @a `gbTimerClockSpeed`. */
        uint8_t enabled     : 1;    /** @brief Bit 2: Timer enable/disable flag. */
        uint8_t             : 5;
    };

    uint8_t raw;    /** @brief The raw, 8-bit value of the register. */
} gbRegisterTAC;

/* Public Function Declarations ***********************************************/

GB_API gbTimer* gbCreateTimer (gbContext* context);
GB_API bool gbDestroyTimer (gbTimer* timer);
GB_API bool gbInitializeTimer (gbTimer* timer);

/* Public Function Declarations - Helper Functions ****************************/

GB_API bool gbCheckTimerDividerFallingEdge (const gbTimer* timer,
    uint8_t dividerBit, bool* outFallingEdge);

/* Public Function Declarations - Callbacks ***********************************/

GB_API bool gbSetTimerOverflowCallback (gbTimer* timer,
    gbTimerOverflowCallback callback);

/* Public Function Declarations - Ticking *************************************/

GB_API bool gbTickTimer (gbTimer* timer);

/* Public Function Declarations - Hardware Register Access ********************/

GB_API bool gbReadDIV (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* rules);
GB_API bool gbReadTIMA (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* rules);
GB_API bool gbReadTMA (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* rules);
GB_API bool gbReadTAC (const gbTimer* timer, uint8_t* outValue, const gbCheckRules* rules);

GB_API bool gbWriteDIV (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);
GB_API bool gbWriteTIMA (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);
GB_API bool gbWriteTMA (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);
GB_API bool gbWriteTAC (gbTimer* timer, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);
