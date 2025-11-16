/**
 * @file    GB/Memory.h
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-16
 * 
 * @brief   Contains declarations for the Game Boy Emulator Core's internal,
 *          random-access memory (RAM) component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Context.h>

/* Public Unions and Structures ***********************************************/

/**
 * @brief   Defines a bitfield union representing the Game Boy Emulator Core's
 *          `SVBK` hardware register, which selects the active WRAM bank in
 *          CGB mode.
 */
typedef union gbRegisterSVBK
{
    struct
    {
        uint8_t wramBank    : 3;    /** @brief Bits 0-2: WRAM bank select (0 - 7). If `0`, then `1` instead. */
        uint8_t             : 5;
    };

    uint8_t raw;    /** @brief The raw, 8-bit value of the register. */
} gbRegisterSVBK;

/* Public Function Declarations ***********************************************/

/**
 * @brief   Allocates and creates a new random-access memory (RAM) component for
 *          the given Game Boy Emulator Core context.
 * 
 * @param   parentContext   A pointer to the @a `gbContext` structure which will
 *                          own this memory component. Must not be `nullptr`.
 * 
 * @return  If successful, a pointer to the newly created @a `gbMemory` structure.
 *          If allocation fails or if invalid parameters are provided, returns
 *          `nullptr`.
 */
GB_API gbMemory* gbCreateMemory (gbContext* parentContext);

/**
 * @brief   Destroys and deallocates a random-access memory (RAM) component.
 * 
 * @param   memory      A pointer to the @a `gbMemory` structure to be destroyed.
 * 
 * @return  If successful, returns `true`.
 *          If no memory component is provided (i.e., `nullptr`), returns `false`.
 */
GB_API bool gbDestroyMemory (gbMemory* memory);

/**
 * @brief   Initializes (or resets) a random-access memory (RAM) component.
 * 
 * This function initializes (or resets) the provided @a `gbMemory` structure,
 * clearing all memory areas (WRAM, HRAM, etc.) and setting hardware registers
 * to their default power-on values.
 * 
 * This function is called internally when a context is initialized or reset, but
 * can also be called separately to re-initialize just the memory component.
 * 
 * @param   memory      A pointer to the @a `gbMemory` structure to be initialized
 * 
 * @return  If successful, returns `true`.
 *          If no memory component is provided (i.e., `nullptr`), returns `false`.
 */
GB_API bool gbInitializeMemory (gbMemory* memory);

/* Public Function Declarations - Memory Access *******************************/

/**
 * @brief   Reads a byte from the specified address within the Work RAM (WRAM)
 *          area of the given memory component.
 * 
 * @param   memory          A pointer to the @a `gbMemory` structure from which
 *                          to read data. Must not be `nullptr`.
 * @param   relativeAddress The 16-bit address from which to read the byte,
 *                          relative to the start of the WRAM area. Must be within
 *                          the range `0x0000` to `0x1FFF`.
 * @param   outValue        A pointer to a byte variable where the value read
 *                          from memory will be stored. Must not be `nullptr`.
 * @param   rules           A pointer to a @a `gbCheckRules` structure defining
 *                          the access rules to enforce during this read operation.
 * 
 * @return  If successful, returns `true`.
 *          If reading fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 * 
 * @note    If access rules are provided, they will be enforced during the read
 *          operation, potentially causing the read to be blocked, in which case
 *          an open-bus value (`0xFF`) will be returned.
 */
GB_API bool gbReadWorkRAM (const gbMemory* memory, uint16_t relativeAddress, 
    uint8_t* outValue, const gbCheckRules* rules);

/**
 * @brief   Reads a byte from the specified address within the High RAM (HRAM)
 *          area of the given memory component.
 * 
 * @param   memory          A pointer to the @a `gbMemory` structure from which
 *                          to read data. Must not be `nullptr`.
 * @param   relativeAddress The 16-bit address from which to read the byte,
 *                          relative to the start of the HRAM area. Must be within
 *                          the range `0x0000` to `0x007E`.
 * @param   outValue        A pointer to a byte variable where the value read
 *                          from memory will be stored. Must not be `nullptr`.
 * @param   rules           A pointer to a @a `gbCheckRules` structure defining
 *                          the access rules to enforce during this read operation.
 * 
 * @return  If successful, returns `true`.
 *          If reading fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 * 
 * @note    If access rules are provided, they will be enforced during the read
 *          operation, potentially causing the read to be blocked, in which case
 *          an open-bus value (`0xFF`) will be returned.
 */
GB_API bool gbReadHighRAM (const gbMemory* memory, uint16_t relativeAddress, 
    uint8_t* outValue, const gbCheckRules* rules);

/**
 * @brief   Writes a byte to the specified address within the Work RAM (WRAM)
 *          area of the given memory component.
 * 
 * @param   memory          A pointer to the @a `gbMemory` structure to which
 *                          to write data. Must not be `nullptr`.
 * @param   relativeAddress The 16-bit address to which to write the byte,
 *                          relative to the start of the WRAM area. Must be within
 *                          the range `0x0000` to `0x1FFF`.
 * @param   value           The byte value to write to the specified address.
 * @param   outActual       A pointer to a byte variable where the actual value
 *                          written to memory will be stored. Must not be `nullptr`.
 * @param   rules           A pointer to a @a `gbCheckRules` structure defining
 *                          the access rules to enforce during this write operation.
 * 
 * @return  If successful, returns `true`.
 *          If writing fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 * 
 * @note    If access rules are provided, they will be enforced during the write
 *          operation, potentially causing the write to be blocked, in which case
 *          @a `outActual` will be set to an open-bus value (`0xFF`).
 */
GB_API bool gbWriteWorkRAM (gbMemory* memory, uint16_t relativeAddress, 
    uint8_t value, uint8_t* outActual, const gbCheckRules* rules);

/**
 * @brief   Writes a byte to the specified address within the High RAM (HRAM)
 *          area of the given memory component.
 * 
 * @param   memory          A pointer to the @a `gbMemory` structure to which
 *                          to write data. Must not be `nullptr`.
 * @param   relativeAddress The 16-bit address to which to write the byte,
 *                          relative to the start of the HRAM area. Must be within
 *                          the range `0x0000` to `0x007E`.
 * @param   value           The byte value to write to the specified address.
 * @param   outActual       A pointer to a byte variable where the actual value
 *                          written to memory will be stored. Must not be `nullptr`.
 * @param   rules           A pointer to a @a `gbCheckRules` structure defining
 *                          the access rules to enforce during this write operation.
 * 
 * @return  If successful, returns `true`.
 *          If writing fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 *
 * @note    If access rules are provided, they will be enforced during the write
 *          operation, potentially causing the write to be blocked, in which case
 *          @a `outActual` will be set to an open-bus value (`0xFF`).
 */
GB_API bool gbWriteHighRAM (gbMemory* memory, uint16_t relativeAddress, 
    uint8_t value, uint8_t* outActual, const gbCheckRules* rules);

/* Public Function Declarations - Hardware Register Access ********************/

/**
 * @brief   Reads the value of the `SVBK` hardware register from the given
 *          memory component.
 * 
 * @param   memory      A pointer to the @a `gbMemory` structure from which to
 *                      read the register. Must not be `nullptr`.
 * @param   outValue    A pointer to a byte variable where the register value
 *                      will be stored. Must not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining the
 *                      access rules to enforce during this read operation.
 * 
 * @return  If successful, returns `true`.
 *          If reading fails (e.g., null pointers, etc.), returns `false`.
 */
GB_API bool gbReadSVBK (const gbMemory* memory, uint8_t* outValue, 
    const gbCheckRules* rules);

/**
 * @brief   Writes a value to the `SVBK` hardware register of the given
 *          memory component.
 * 
 * @param   memory      A pointer to the @a `gbMemory` structure to which to
 *                      write the register. Must not be `nullptr`.
 * @param   value       The byte value to write to the register.
 * @param   outActual   A pointer to a byte variable where the actual value
 *                      written to the register will be stored. Must not be
 *                      `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining the
 *                      access rules to enforce during this write operation.
 *
 * @return  If successful, returns `true`.
 *          If writing fails (e.g., null pointers, etc.), returns `false`.
 */
GB_API bool gbWriteSVBK (gbMemory* memory, uint8_t value, uint8_t* outActual, 
    const gbCheckRules* rules);
    