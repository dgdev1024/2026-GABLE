/**
 * @file    GB/Instructions.h
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-18
 * 
 * @brief   Contains definitions for the Game Boy Emulator CPU's instruction
 *          execution functions.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Processor.h>

/* Public Function Declarations - CPU Control Instructions ********************/

/**
 * @brief   Executes a `NOP` instruction, which does nothing but consume a
 *          machine cycle.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `NOP` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x00 NOP`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteNOP (gbProcessor* processor);

/**
 * @brief   Executes the `STOP`, which places the given CPU context into an
 *          ultra-low power `STOP` state, during which all internal components
 *          are halted until awakened by an external event, such as a button press
 *          or hardware reset.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `STOP` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x10 STOP`
 * @note    Instruction Length:     2 Bytes (Opcode + Padding Byte)
 * @note    Instruction Timing:     1 M-Cycle (minimum; actual timing will vary)
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteSTOP (gbProcessor* processor);

/**
 * @brief   Executes the `HALT` instruction, which places the given CPU context
 *          into a low-power `HALT` state, during which instruction execution is
 *          suspended until an interrupt is triggered.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `HALT` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x76 HALT`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle (minimum; actual timing will vary)
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteHALT (gbProcessor* processor);

/**
 * @brief   Executes the `DI` instruction, which disables interrupts by clearing
 *          the given CPU context's `IME` interrupt master enable flag.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `DI` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xF3 DI`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteDI (gbProcessor* processor);

/**
 * @brief   Executes the `EI` instruction, which enables interrupts by scheduling
 *          the given CPU context's `IME` interrupt master enable flag to be set
 *          after the next instruction is executed.   
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `EI` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xFB EI`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteEI (gbProcessor* processor);

/* Public Function Declarations - Branching Instructions **********************/

/**
 * @brief   Executes a `JR [cc], s8` instruction, which moves the given CPU
 *          context's `PC` program counter register by a signed 8-bit offset,
 *          optionally predicated on an execution condition.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   condition   The @a `gbCondition` condition to evaluate before
 *                      executing the instruction. Pass `GB_CT_NONE` to specify
 *                      an unconditional branch.
 * @param   offset      The signed 8-bit offset by which to adjust the `PC`
 *                      program counter register if the branch is taken.
 * @param   outTaken    A pointer to a @a `bool` which will indicate whether
 *                      the branch was taken (`true`) or not (`false`).
 *                      Pass `nullptr` if this information is not needed.
 * 
 * @return  `true` if the `JR [cc], s8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or condition enumeration
 *          value was provided.
 * 
 * @note    Instruction Opcodes:    `0x18 JR s8`, `0x20 JR NZ,s8`, `0x28 JR Z,s8`,
 *                                  `0x30 JR NC,s8`, `0x38 JR C,s8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     3 M-Cycles if Branch Taken;
 *                                  2 M-Cycles if Branch Not Taken
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteJR_S8 (gbProcessor* processor, gbCondition condition, 
    int8_t offset, bool* outTaken);

/**
 * @brief   Executes a `RET [cc]` instruction, which returns from a subroutine by
 *          popping a word from the stack and loading it into the given CPU
 *          context's `PC` program counter register, optionally predicated on an
 *          execution condition.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   condition   The @a `gbCondition` condition to evaluate before
 *                      executing the instruction. Pass `GB_CT_NONE` to specify
 *                      an unconditional return.
 * @param   outTaken    A pointer to a @a `bool` which will indicate whether
 *                      the return was taken (`true`) or not (`false`).
 *                      Pass `nullptr` if this information is not needed.
 * 
 * @return  `true` if the `RET [cc]` instruction was executed successfully;
 *          `false` if an invalid processor pointer or condition enumeration
 *          value was provided.
 * 
 * @note    Instruction Opcodes:    `0xC0 RET NZ`, `0xC8 RET Z`, `0xC9 RET`,
 *                                  `0xD0 RET NC`, `0xD8 RET C`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     4 M-Cycles if Return Taken Unconditionally;
 *                                  5 M-Cycles if Return Taken Conditionally;
 *                                  2 M-Cycles if Return Not Taken
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteRET (gbProcessor* processor, gbCondition condition, 
    bool* outTaken);

/**
 * @brief   Executes a `JP [cc], a16` instruction, which jumps to a given 16-bit
 *          address by loading it into the given CPU context's `PC` program
 *          counter register, optionally predicated on an execution condition.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   condition   The @a `gbCondition` condition to evaluate before
 *                      executing the instruction. Pass `GB_CT_NONE` to specify
 *                      an unconditional jump.
 * @param   address     The 16-bit address to which to jump if the branch is taken.
 * @param   outTaken    A pointer to a @a `bool` which will indicate whether
 *                      the jump was taken (`true`) or not (`false`).
 *                      Pass `nullptr` if this information is not needed.
 *
 * @return  `true` if the `JP [cc], a16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or condition enumeration
 *          value was provided.
 * 
 * @note    Instruction Opcodes:    `0xC2 JP NZ,a16`, `0xC3 JP a16`, `0xCA JP Z,a16`,
 *                                  `0xD2 JP NC,a16`, `0xDA JP C,a16`
 * @note    Instruction Length:     3 Bytes (Opcode + Immediate Word)
 * @note    Instruction Timing:     4 M-Cycles if Jump Taken;
 *                                  3 M-Cycles if Jump Not Taken
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteJP_A16 (gbProcessor* processor, gbCondition condition, 
    uint16_t address, bool* outTaken);

/**
 * @brief   Executes a `CALL [cc], a16` instruction, which calls a subroutine at
 *          a given 16-bit address by pushing the current `PC` program counter
 *          register onto the stack and loading the given address into `PC`,
 *          optionally predicated on an execution condition.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   condition   The @a `gbCondition` condition to evaluate before
 *                      executing the instruction. Pass `GB_CT_NONE` to specify
 *                      an unconditional call.
 * @param   address     The 16-bit address of the subroutine to call if the
 *                      branch is taken.
 * @param   outTaken    A pointer to a @a `bool` which will indicate whether
 *                      the call was taken (`true`) or not (`false`).
 *                      Pass `nullptr` if this information is not needed.
 *
 * @return  `true` if the `CALL [cc], a16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or condition enumeration
 *          value was provided.
 * 
 * @note    Instruction Opcodes:    `0xC4 CALL NZ,a16`, `0xCC CALL Z,a16`, `0xCD CALL a16`,
 *                                  `0xD4 CALL NC,a16`, `0xDC CALL C,a16`
 * @note    Instruction Length:     3 Bytes (Opcode + Immediate Word)
 * @note    Instruction Timing:     6 M-Cycles if Call Taken;
 *                                  3 M-Cycles if Call Not Taken
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteCALL_A16 (gbProcessor* processor, gbCondition condition,
    uint16_t address, bool* outTaken);

/**
 * @brief   Executes one of the `RST` instructions, which calls a subroutine at a
 *          fixed restart vector address by pushing the current `PC` program
 *          counter register onto the stack and loading the restart vector address
 *          into `PC`.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   vector      The @a `gbRestartVector` restart vector at which to
 *                      execute the instruction. A valid enumeration will contain
 *                      one of the eight possible restart vector addresses.
 * @param   outTaken    A pointer to a @a `bool` which will indicate whether
 *                      the restart was taken (always `true` for this instruction).
 *                      Pass `nullptr` if this information is not needed.
 * 
 * @return  `true` if the `RST` instruction was executed successfully;
 *          `false` if an invalid processor pointer or restart vector
 *          enumeration value was provided.
 * 
 * @note    Instruction Opcodes:    `0xC7 RST 00H`, `0xCF RST 10H`, `0xD7 RST 20H`,
 *                                  `0xDF RST 30H`, `0xE7 RST 08H`, `0xEF RST 18H`,
 *                                  `0xF7 RST 28H`, `0xFF RST 38H`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteRST (gbProcessor* processor, gbRestartVector vector,
    bool* outTaken);

/**
 * @brief   Executes a `RETI` instruction, which returns from an interrupt service
 *          routine by popping a word from the stack and loading it into the given
 *          CPU context's `PC` program counter register, then enabling interrupts
 *          by setting the `IME` interrupt master enable flag.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   outTaken    A pointer to a @a `bool` which will indicate whether
 *                      the return was taken (always `true` for this instruction).
 *                      Pass `nullptr` if this information is not needed.
 * 
 * @return  `true` if the `RETI` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xD9 RETI`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteRETI (gbProcessor* processor, bool* outTaken);

/**
 * @brief   Executes a `JP HL` instruction, which jumps to the address contained
 *          in the given CPU context's `HL` register pair by loading it into the
 *          `PC` program counter register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   outAddress  A pointer to a @a `uint16_t` which will contain the
 *                      value of the `HL` register pair, which contains the
 *                      address to which to jump. Pass `nullptr` if this information
 *                      is not needed.
 * @param   outTaken    A pointer to a @a `bool` which will indicate whether
 *                      the jump was taken (always `true` for this instruction).
 *                      Pass `nullptr` if this information is not needed.
 * 
 * @return  `true` if the `JP HL` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xE9 JP HL`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteJP_HL (gbProcessor* processor, uint16_t* outAddress,
    bool* outTaken);

/* Public Function Declarations - 8-Bit Load Instructions *********************/

/**
 * @brief   Executes an `LD (R16), R8` instruction, which stores the value of an
 *          8-bit register into the memory address contained in a 16-bit register pair,
 *          then optionally increments or decrements that register pair.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the destination memory address into which to store the value
 *                      of the `A` accumulator register. Must be one of the
 *                      16-bit register pairs: `BC`, `DE`, or `HL`; or the
 *                      special-purpose `SP` register.
 * @param   src         The @a `gbRegisterType` 8-bit register from which to
 *                      read the value to store into memory. Must be the
 *                      `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * @param   adjust      If negative, decrement the register pair's value by 1
 *                      after the store; if positive, increment the register pair's
 *                      value by 1 after the store; if zero, leave the register pair's
 *                      value unchanged.
 * 
 * @return  `true` if the `LD (R16), R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x02 LD (BC),A`,   `0x12 LD (DE),A`, `0x22 LD (HL+),A`,
 *                                  `0x32 LD (HL-),A`,  `0x70 LD (HL),B`, `0x71 LD (HL),C`,
 *                                  `0x72 LD (HL),D`,   `0x73 LD (HL),E`, `0x74 LD (HL),H`,
 *                                  `0x75 LD (HL),L`,   `0x77 LD (HL),A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_pR16_R8 (gbProcessor* processor,
    gbRegisterType dest, gbRegisterType src, int8_t adjust);

/**
 * @brief   Executes an `LD R8, D8` instruction, which loads an immediate
 *          8-bit value into the given CPU context's specified 8-bit register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register into which to load
 *                      the immediate 8-bit value. Must be the `A` accumulator 
 *                      register, or one of the general-purpose registers: `B`, `C`,
 *                      `D`, `E`, `H`, or `L`.
 * @param   src         The immediate 8-bit value to load into the specified
 *                      destination register.
 * 
 * @return  `true` if the `LD R8, D8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 *
 * @note    Instruction Opcodes:    `0x06 LD B,D8`,  `0x0E LD C,D8`,  `0x16 LD D,D8`,
 *                                  `0x1E LD E,D8`,  `0x26 LD H,D8`,  `0x2E LD L,D8`,
 *                                  `0x3E LD A,D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_R8_D8 (gbProcessor* processor,
    gbRegisterType dest, uint8_t src);

/**
 * @brief   Executes an `LD R8, (R16)` instruction, which loads an 8-bit value
 *          from the memory address contained in a 16-bit register pair into
 *          the given CPU context's specified 8-bit register, then optionally
 *          increments or decrements that register pair.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register into which to
 *                      load the value read from memory. Must be the `A` accumulator
 *                      register, or one of the general-purpose registers: `B`, `C`,
 *                      `D`, `E`, `H`, or `L`.
 * @param   src         The @a `gbRegisterType` 16-bit register pair which contains
 *                      the source memory address from which to read the 8-bit value.
 *                      Must be one of the 16-bit register pairs: `BC`, `DE`, or `HL`;
 *                      or the special-purpose `SP` register.
 * @param   adjust      If negative, decrement the register pair's value by 1
 *                      after the load; if positive, increment the register pair's
 *                      value by 1 after the load; if zero, leave the register pair's
 *                      value unchanged.
 * 
 * @return  `true` if the `LD R8, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x0A LD A,(BC)`,   `0x1A LD A,(DE)`, `0x2A LD A,(HL+)`,
 *                                  `0x3A LD A,(HL-)`,  `0x46 LD B,(HL)`, `0x4E LD C,(HL)`,
 *                                  `0x56 LD D,(HL)`,   `0x5E LD E,(HL)`, `0x66 LD H,(HL)`,
 *                                  `0x6E LD L,(HL)`,   `0x7E LD A,(HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_R8_pR16 (gbProcessor* processor,
    gbRegisterType dest, gbRegisterType src, int8_t adjust);

/**
 * @brief   Executes an `LD (R16), D8` instruction, which stores an immediate
 *          8-bit value into the memory address contained in a 16-bit register pair.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the destination memory address into which to store the
 *                      immediate 8-bit value. Must be one of the 16-bit register
 *                      pairs: `BC`, `DE`, or `HL`; or the special-purpose `SP` 
 *                      register.
 * @param   src         The immediate 8-bit value to store into the specified 
 *                      memory address.
 * 
 * @return  `true` if the `LD (R16), D8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x36 LD (HL),D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_pR16_D8 (gbProcessor* processor,
    gbRegisterType dest, uint8_t src);

/**
 * @brief   Executes an `LD R8, R8` instruction, which loads the value from
 *          one 8-bit register into another 8-bit register within the given
 *          CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register into which to
 *                      load the value. Must be the `A` accumulator register, or
 *                      one of the general-purpose registers: `B`, `C`, `D`, `E`,
 *                      `H`, or `L`.
 * @param   src         The @a `gbRegisterType` 8-bit register from which to
 *                      read the value. Must be the `A` accumulator register, or
 *                      one of the general-purpose registers: `B`, `C`, `D`, `E`,
 *                      `H`, or `L`.
 * 
 * @return  `true` if the `LD R8, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    Many, from `0x40 LD B,B` through `0x7F LD A,A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_R8_R8 (gbProcessor* processor,
    gbRegisterType dest, gbRegisterType src);

/**
 * @brief   Executes an `LDH (a8), R8` instruction, which stores the value of
 *          an 8-bit register into a relative memory address within the Game
 *          Boy's zero-page HRAM area (`$FF00` to `$FFFF`), specified by the
 *          given 8-bit offset.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context
 * @param   offset      The 8-bit offset from the base HRAM address `$FF00`. The
 *                      effective address is `$FF00` + `offset`.
 * @param   src         The @a `gbRegisterType` 8-bit register from which to
 *                      read the value to store into memory. Must be the
 *                      `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `LDH (a8), R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xE0 LDH (a8),A`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLDH_pA8_R8 (gbProcessor* processor,
    uint8_t offset, gbRegisterType src);

/**
 * @brief   Executes an `LDH (C), R8` instruction, which stores the value of
 *          an 8-bit register into a relative memory address within the Game
 *          Boy's zero-page HRAM area (`$FF00` to `$FFFF`), specified by the
 *          value in the `C` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The @a `gbRegisterType` 8-bit register from which to
 *                      read the value to store into memory. Must be the
 *                      `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `LDH (C), R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xE2 LDH (C),A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLDH_pC_R8 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes an `LD (a16), R8` instruction, which stores the value of
 *          an 8-bit register into the specified 16-bit memory address.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   address     The 16-bit memory address into which to store the value
 *                      of the specified 8-bit register.
 * @param   src         The @a `gbRegisterType` 8-bit register from which to
 *                      read the value to store into memory. Must be the
 *                      `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `LD (a16), R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 *
 * @note    Instruction Opcodes:    `0xEA LD (a16),A`
 * @note    Instruction Length:     3 Bytes (Opcode + Immediate Word)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_pA16_R8 (gbProcessor* processor,
    uint16_t address, gbRegisterType src);

/**
 * @brief   Executes an `LDH R8, (a8)` instruction, which loads an 8-bit value
 *          from a relative memory address within the Game Boy's zero-page
 *          HRAM area (`$FF00` to `$FFFF`) into the given CPU context's specified
 *          8-bit register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register into which to
 *                      load the value read from memory. Must be the `A` accumulator
 *                      register, or one of the general-purpose registers: `B`, `C`,
 *                      `D`, `E`, `H`, or `L`.
 * @param   offset      The 8-bit offset from the base HRAM address `$FF00`. The
 *                      effective address is `$FF00` + `offset`.
 * 
 * @return  `true` if the `LDH A, (a8)` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xF0 LDH A,(a8)`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLDH_R8_pA8 (gbProcessor* processor,
    gbRegisterType dest, uint8_t offset);

/**
 * @brief   Executes an `LDH R8, (C)` instruction, which loads an 8-bit value
 *          from a relative memory address within the Game Boy's zero-page
 *          HRAM area (`$FF00` to `$FFFF`), specified by the value in the `C`
 *          register, into the given CPU context's specified 8-bit register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context
 * @param   dest        The @a `gbRegisterType` 8-bit register into which to
 *                      load the value read from memory. Must be the `A` accumulator
 *                      register, or one of the general-purpose registers: `B`, `C`,
 *                      `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `LDH R8, (C)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 *
 * @note    Instruction Opcodes:    `0xF2 LDH A,(C)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLDH_R8_pC (gbProcessor* processor,
    gbRegisterType dest);

/**
 * @brief   Executes an `LD R8, (a16)` instruction, which loads an 8-bit value
 *          from the specified 16-bit memory address into the given CPU context's
 *          specified 8-bit register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @param   dest        The @a `gbRegisterType` 8-bit register into which to
 *                      load the value read from memory. Must be the `A` accumulator
 *                      register, or one of the general-purpose registers: `B`, `C`,
 *                      `D`, `E`, `H`, or `L`.
 * @param   address     The 16-bit memory address from which to read the 8-bit value.
 * 
 * @return  `true` if the `LD R8, (a16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xFA LD A,(a16)`
 * @note    Instruction Length:     3 Bytes (Opcode + Immediate Word)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_R8_pA16 (gbProcessor* processor,
    gbRegisterType dest, uint16_t address);

/* Public Functions - 16-Bit Load Instructions ********************************/

/**
 * @brief   Executes an `LD R16, D16` instruction, which loads an immediate
 *          16-bit value into the given CPU context's specified 16-bit register pair.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair into which
 *                      to load the immediate value. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, `HL`; or the `SP` stack
 *                      pointer register.
 * @param   src         The immediate 16-bit value to load into the specified
 *                      destination register pair.
 * 
 * @return  `true` if the `LD R16, D16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x01 LD BC,D16`,  `0x11 LD DE,D16`,
 *                                  `0x21 LD HL,D16`,  `0x31 LD SP,D16`
 * @note    Instruction Length:     3 Bytes (Opcode + Immediate Word)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_R16_D16 (gbProcessor* processor,
    gbRegisterType dest, uint16_t src);

/**
 * @brief   Executes an `LD (a16), SP` instruction, which stores the value of the
 *          given CPU context's `SP` stack pointer register into the specified
 *          16-bit memory address.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   address     The 16-bit memory address into which to store the value
 *                      of the `SP` stack pointer register.
 * 
 * @return  `true` if the `LD (a16), SP` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x08 LD (a16),SP`
 * @note    Instruction Length:     3 Bytes (Opcode + Immediate Word)
 * @note    Instruction Timing:     5 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_pA16_SP (gbProcessor* processor,
    uint16_t address);

/**
 * @brief   Executes a `POP R16` instruction, which pops a word from the stack
 *          and loads it into the given CPU context's specified 16-bit register pair.
 * 
 * If `dest` is `AF`, the CPU's `F` flags register will have its bits 7-4
 * set according to the popped value.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair into which to
 *                      load the value popped from the stack. Must be one of the
 *                      16-bit register pairs: `BC`, `DE`, `HL`, or `AF`.
 * 
 * @return  `true` if the `POP R16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xC1 POP BC`,  `0xD1 POP DE`,
 *                                  `0xE1 POP HL`,  `0xF1 POP AF`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         If `dest` is `AF`, affect according to the
 *                                  value popped from the stack; otherwise, none.
 */
GB_API bool gbExecutePOP_R16 (gbProcessor* processor,
    gbRegisterType dest);

/**
 * @brief   Executes a `PUSH R16` instruction, which pushes the value of the given
 *          CPU context's specified 16-bit register pair onto the stack.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The @a `gbRegisterType` 16-bit register pair whose value
 *                      is to be pushed onto the stack. Must be one of the
 *                      16-bit register pairs: `BC`, `DE`, `HL`, or `AF`.
 * 
 * @return  `true` if the `PUSH R16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xC5 PUSH BC`,  `0xD5 PUSH DE`,
 *                                  `0xE5 PUSH HL`,  `0xF5 PUSH AF`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecutePUSH_R16 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes an `LD R16, SP+S8` instruction, which loads the value of the
 *          given CPU context's `SP` stack pointer register plus a signed 8-bit
 *          offset into the specified 16-bit register pair.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair into which
 *                      to load the computed value. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, `HL`; or the `SP` stack
 *                      pointer register.
 * @param   offset      The signed 8-bit offset to add to the `SP` stack pointer
 *                      register's value before loading it into the destination
 *                      register pair.
 * 
 * @return  `true` if the `LD R16, SP+S8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xF8 LD HL,SP+S8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         `Z` - Clear.
 *                                  `N` - Clear.
 *                                  `H` - Set if carry from bit 3; Clear otherwise.
 *                                  `C` - Set if carry from bit 7; Clear otherwise.
 */
GB_API bool gbExecuteLD_R16_SPpS8 (gbProcessor* processor,
    gbRegisterType dest, int8_t offset);

/**
 * @brief   Executes an `LD SP, R16` instruction, which loads the value of the
 *          given CPU context's specified 16-bit register pair into the `SP`
 *          stack pointer register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context
 * @param   src         The @a `gbRegisterType` 16-bit register pair whose value
 *                      is to be loaded into the `SP` stack pointer register. Must
 *                      be one of the 16-bit register pairs: `BC`, `DE`, or `HL`.
 * 
 * @return  `true` if the `LD SP, R16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xF9 LD SP,HL`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None
 */
GB_API bool gbExecuteLD_SP_R16 (gbProcessor* processor, gbRegisterType src);

/* Public Function Declarations - 8-Bit Arithmetic/Bitwise Logic Instructions */

/**
 * @brief   Executes an `INC R8` instruction, which increments the value of the
 *          given CPU context's specified 8-bit register by 1.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register whose value is to
 *                      be incremented by 1. Must be the `A` accumulator register,
 *                      or one of the general-purpose registers: `B`, `C`, `D`,
 *                      `E`, `H`, or `L`.
 * 
 * @return  `true` if the `INC R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x04 INC B`,  `0x0C INC C`,  `0x14 INC D`,
 *                                  `0x1C INC E`,  `0x24 INC H`,  `0x2C INC L`,
 *                                  `0x3C INC A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set if carry from bit 3; Clear otherwise.
 */
GB_API bool gbExecuteINC_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `DEC R8` instruction, which decrements the value of the
 *          given CPU context's specified 8-bit register by 1.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register whose value is to
 *                      be decremented by 1. Must be the `A` accumulator register,
 *                      or one of the general-purpose registers: `B`, `C`, `D`,
 *                      `E`, `H`, or `L`.
 * 
 * @return  `true` if the `DEC R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 *
 * @note    Instruction Opcodes:    `0x05 DEC B`,  `0x0D DEC C`,  `0x15 DEC D`,
 *                                  `0x1D DEC E`,  `0x25 DEC H`,  `0x2D DEC L`,
 *                                  `0x3D DEC A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 */
GB_API bool gbExecuteDEC_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `INC (R16)` instruction, which increments the 8-bit value
 *          stored at the memory address contained in the given CPU context's
 *          specified 16-bit register pair by 1.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be incremented by 1.
 *                      Must be one of the 16-bit register pairs: `BC`, `DE`, or `HL`.
 * 
 * @return  `true` if the `INC (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x34 INC (HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set if carry from bit 3; Clear otherwise.
 */
GB_API bool gbExecuteINC_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `DEC (R16)` instruction, which decrements the 8-bit value
 *          stored at the memory address contained in the given CPU context's
 *          specified 16-bit register pair by 1.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be decremented by 1.
 *                      Must be one of the 16-bit register pairs: `BC`, `DE`, or `HL`.
 * 
 * @return  `true` if the `DEC (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x35 DEC (HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 */
GB_API bool gbExecuteDEC_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `DAA` instruction, which adjusts the value of the given
 *          CPU context's `A` accumulator register to form a correct Binary-Coded
 *          Decimal (BCD) representation after a previous addition or subtraction
 *          operation.
 * 
 * A Binary-Coded Decimal (BCD) is a form of hexadecimal representation in which
 * each nibble (4 bits) of a byte represents a decimal digit (0-9). The `DAA`
 * instruction ensures that the value in the `A` register conforms to BCD format
 * after arithmetic operations.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `DAA` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x27 DAA`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `H` - Clear.
 *                                  `C` - Set or cleared according to the adjustment.
 */
GB_API bool gbExecuteDAA (gbProcessor* processor);

/**
 * @brief   Executes a `CPL` instruction, which complements (inverts) all bits
 *          in the given CPU context's `A` accumulator register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `CPL` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x2F CPL`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `N` - Set.
 *                                  `H` - Set.
 */
GB_API bool gbExecuteCPL (gbProcessor* processor);

/**
 * @brief   Executes an `SCF` instruction, which sets the carry flag (`C`) in
 *          the given CPU context's flags register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 *
 * @return  `true` if the `SCF` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 *
 * @note    Instruction Opcodes:    `0x37 SCF`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Set.
 */
GB_API bool gbExecuteSCF (gbProcessor* processor);

/**
 * @brief   Executes a `CCF` instruction, which complements (inverts) the carry
 *          flag (`C`) in the given CPU context's flags register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `CCF` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x3F CCF`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Set if clear; Clear if set.
 */
GB_API bool gbExecuteCCF (gbProcessor* processor);

/**
 * @brief   Executes an `ADD A, R8` or `ADC A, R8` instruction, which adds the
 *          value of the specified 8-bit register to the `A` accumulator register
 *          within the given CPU context, optionally including the current value
 *          of the carry flag in the addition.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 8-bit register whose value is 
 *                      to be added to the `A` accumulator register. Must be the
 *                      `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * @param   withCarry  A @a `bool` indicating whether to include the current
 *                      value of the carry flag (`C`) in the addition (`ADC A, R8`)
 *                      or not (`ADD A, R8`).
 *
 * @return  `true` if the `ADD A, R8` or `ADC A, R8` instruction was executed
 *          successfully; `false` if an invalid processor pointer or register
 *          type was provided.
 * 
 * @note    Instruction Opcodes:    `0x80 ADD A,B`,  `0x81 ADD A,C`,  `0x82 ADD A,D`,
 *                                  `0x83 ADD A,E`,  `0x84 ADD A,H`,  `0x85 ADD A,L`,
 *                                  `0x87 ADD A,A`,  `0x88 ADC A,B`,  `0x89 ADC A,C`,
 *                                  `0x8A ADC A,D`,  `0x8B ADC A,E`,  `0x8C ADC A,H`,
 *                                  `0x8D ADC A,L`,  `0x8F ADC A,A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set if carry from bit 3; Clear otherwise.
 *                                  `C` - Set if carry from bit 7; Clear otherwise.
 */
GB_API bool gbExecuteADD_A_R8 (gbProcessor* processor,
    gbRegisterType src, bool withCarry);

/**
 * @brief   Executes an `ADD A, (R16)` or `ADC A, (R16)` instruction, which adds
 *          the 8-bit value stored at the memory address contained in the given
 *          CPU context's specified 16-bit register pair to the `A` accumulator
 *          register, optionally including the current value of the carry flag
 *          in the addition.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be added to the
 *                      `A` accumulator register. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, or `HL`.
 * @param   withCarry  A @a `bool` indicating whether to include the current
 *                      value of the carry flag (`C`) in the addition (`ADC A, (R16)`)
 *                      or not (`ADD A, (R16)`).
 *
 * @return  `true` if the `ADD A, (R16)` or `ADC A, (R16)` instruction was 
 *          executed successfully; `false` if an invalid processor pointer
 *          or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x86 ADD A,(HL)`,  `0x8E ADC A,(HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set if carry from bit 3; Clear otherwise.
 *                                  `C` - Set if carry from bit 7; Clear otherwise.
 */
GB_API bool gbExecuteADD_A_pR16 (gbProcessor* processor,
    gbRegisterType src, bool withCarry);

/**
 * @brief   Executes an `ADD A, D8` or `ADC A, D8` instruction, which adds an
 *          immediate 8-bit value to the `A` accumulator register within the given
 *          CPU context, optionally including the current value of the carry flag
 *          in the addition.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   value       The immediate 8-bit value to be added to the `A`
 *                      accumulator register.
 * @param   withCarry  A @a `bool` indicating whether to include the current
 *                      value of the carry flag (`C`) in the addition (`ADC A, D8`)
 *                      or not (`ADD A, D8`).
 * 
 * @return  `true` if the `ADD A, D8` or `ADC A, D8` instruction was executed
 *          successfully; `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xC6 ADD A,D8`,  `0xCE ADC A,D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set if carry from bit 3; Clear otherwise.
 *                                  `C` - Set if carry from bit 7; Clear otherwise.
 */
GB_API bool gbExecuteADD_A_D8 (gbProcessor* processor,
    uint8_t src, bool withCarry);

/**
 * @brief   Executes a `SUB A, R8` or `SBC A, R8` instruction, which subtracts
 *          the value of the specified 8-bit register from the `A` accumulator
 *          register within the given CPU context, optionally including the
 *          current value of the carry flag in the subtraction.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 8-bit register whose value is
 *                      to be subtracted from the `A` accumulator register. Must
 *                      be the `A` accumulator register, or one of the
 *                      general-purpose registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * @param   withCarry  A @a `bool` indicating whether to include the current
 *                      value of the carry flag (`C`) in the subtraction (`SBC A, R8`)
 *                      or not (`SUB A, R8`).
 * 
 * @return  `true` if the `SUB A, R8` or `SBC A, R8` instruction was executed
 *          successfully; `false` if an invalid processor pointer or register
 *          type was provided.
 * 
 * @note    Instruction Opcodes:    `0x90 SUB A,B`,  `0x91 SUB A,C`,  `0x92 SUB A,D`,
 *                                  `0x93 SUB A,E`,  `0x94 SUB A,H`,  `0x95 SUB A,L`,
 *                                  `0x97 SUB A,A`,  `0x98 SBC A,B`,  `0x99 SBC A,C`,
 *                                  `0x9A SBC A,D`,  `0x9B SBC A,E`,  `0x9C SBC A,H`,
 *                                  `0x9D SBC A,L`,  `0x9F SBC A,A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 *                                  `C` - Set if borrow; Clear otherwise.
 */
GB_API bool gbExecuteSUB_A_R8 (gbProcessor* processor,
    gbRegisterType src, bool withCarry);

/**
 * @brief   Executes a `SUB A, (R16)` or `SBC A, (R16)` instruction, which
 *          subtracts the 8-bit value stored at the memory address contained
 *          in the given CPU context's specified 16-bit register pair from the
 *          `A` accumulator register, optionally including the current value
 *          of the carry flag in the subtraction.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be subtracted from
 *                      the `A` accumulator register. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, or `HL`.
 * @param   withCarry  A @a `bool` indicating whether to include the current
 *                      value of the carry flag (`C`) in the subtraction (`SBC A, (R16)`)
 *                      or not (`SUB A, (R16)`).
 * 
 * @return  `true` if the `SUB A, (R16)` or `SBC A, (R16)` instruction was
 *          executed successfully; `false` if an invalid processor pointer
 *          or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0x96 SUB A,(HL)`,  `0x9E SBC A,(HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 *                                  `C` - Set if borrow; Clear otherwise.
 */
GB_API bool gbExecuteSUB_A_pR16 (gbProcessor* processor,
    gbRegisterType src, bool withCarry);

/**
 * @brief   Executes a `SUB A, D8` or `SBC A, D8` instruction, which subtracts
 *          an immediate 8-bit value from the `A` accumulator register within
 *          the given CPU context, optionally including the current value of
 *          the carry flag in the subtraction.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The immediate 8-bit value to be subtracted from the `A`
 *                      accumulator register.
 * @param   withCarry  A @a `bool` indicating whether to include the current
 *                      value of the carry flag (`C`) in the subtraction (`SBC A, D8`)
 *                      or not (`SUB A, D8`).
 * 
 * @return  `true` if the `SUB A, D8` or `SBC A, D8` instruction was executed
 *          successfully; `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xD6 SUB A,D8`,  `0xDE SBC A,D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 *                                  `C` - Set if borrow; Clear otherwise.
 */
GB_API bool gbExecuteSUB_A_D8 (gbProcessor* processor,
    uint8_t src, bool withCarry);

/**
 * @brief   Executes an `AND A, R8` instruction, which performs a bitwise AND
 *          operation between the value of the specified 8-bit register and the
 *          `A` accumulator register within the given CPU context, storing the
 *          result in the `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The @a `gbRegisterType` 8-bit register whose value is
 *                      to be ANDed with the `A` accumulator register. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `AND A, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xA0 AND A,B`,  `0xA1 AND A,C`,  `0xA2 AND A,D`,
 *                                  `0xA3 AND A,E`,  `0xA4 AND A,H`,  `0xA5 AND A,L`,
 *                                  `0xA7 AND A,A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteAND_A_R8 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes an `AND A, (R16)` instruction, which performs a bitwise AND
 *          operation between the 8-bit value stored at the memory address
 *          contained in the given CPU context's specified 16-bit register pair
 *          and the `A` accumulator register, storing the result in the `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be ANDed with the
 *                      `A` accumulator register. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, or `HL`.
 * 
 * @return  `true` if the `AND A, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xA6 AND A,(HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteAND_A_pR16 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes an `AND A, D8` instruction, which performs a bitwise AND
 *          operation between an immediate 8-bit value and the `A` accumulator
 *          register within the given CPU context, storing the result in the
 *          `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The immediate 8-bit value to be ANDed with the `A`
 *                      accumulator register.
 * 
 * @return  `true` if the `AND A, D8` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xE6 AND A,D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Set.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteAND_A_D8 (gbProcessor* processor,
    uint8_t src);

/**
 * @brief   Executes a `XOR A, R8` instruction, which performs a bitwise XOR
 *          (exclusive OR) operation between the value of the specified 8-bit
 *          register and the `A` accumulator register within the given CPU context,
 *          storing the result in the `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 8-bit register whose value is
 *                      to be XORed with the `A` accumulator register. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `XOR A, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xA8 XOR A,B`,  `0xA9 XOR A,C`,  `0xAA XOR A,D`,
 *                                  `0xAB XOR A,E`,  `0xAC XOR A,H`,  `0xAD XOR A,L`,
 *                                  `0xAF XOR A,A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteXOR_A_R8 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes a `XOR A, (R16)` instruction, which performs a bitwise XOR
 *          (exclusive OR) operation between the 8-bit value stored at the memory
 *          address contained in the given CPU context's specified 16-bit register
 *          pair and the `A` accumulator register, storing the result in the `A`
 *          register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be XORed with the
 *                      `A` accumulator register. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, or `HL`.
 * 
 * @return  `true` if the `XOR A, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xAE XOR A,(HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteXOR_A_pR16 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes a `XOR A, D8` instruction, which performs a bitwise XOR
 *          (exclusive OR) operation between an immediate 8-bit value and the
 *          `A` accumulator register within the given CPU context, storing the
 *          result in the `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The immediate 8-bit value to be XORed with the `A`
 *                      accumulator register.
 * 
 * @return  `true` if the `XOR A, D8` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xEE XOR A,D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteXOR_A_D8 (gbProcessor* processor,
    uint8_t src);

/**
 * @brief   Executes an `OR A, R8` instruction, which performs a bitwise OR
 *          operation between the value of the specified 8-bit register and the
 *          `A` accumulator register within the given CPU context, storing the
 *          result in the `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 8-bit register whose value is
 *                      to be ORed with the `A` accumulator register. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `OR A, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xB0 OR A,B`,  `0xB1 OR A,C`,  `0xB2 OR A,D`,
 *                                  `0xB3 OR A,E`,  `0xB4 OR A,H`,  `0xB5 OR A,L`,
 *                                  `0xB7 OR A,A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteOR_A_R8 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes an `OR A, (R16)` instruction, which performs a bitwise OR
 *          operation between the 8-bit value stored at the memory address
 *          contained in the given CPU context's specified 16-bit register pair
 *          and the `A` accumulator register, storing the result in the `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be ORed with the
 *                      `A` accumulator register. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, or `HL`.
 * 
 * @return  `true` if the `OR A, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xB6 OR A,(HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteOR_A_pR16 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes an `OR A, D8` instruction, which performs a bitwise OR
 *          operation between an immediate 8-bit value and the `A` accumulator
 *          register within the given CPU context, storing the result in the
 *          `A` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The immediate 8-bit value to be ORed with the `A`
 *                      accumulator register.
 * 
 * @return  `true` if the `OR A, D8` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xF6 OR A,D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteOR_A_D8 (gbProcessor* processor,
    uint8_t src);

/**
 * @brief   Executes a `CP A, R8` instruction, which compares the value of the
 *          specified 8-bit register with the `A` accumulator register within
 *          the given CPU context by performing a subtraction and setting flags
 *          accordingly, without storing the result.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 8-bit register whose value is
 *                      to be compared with the `A` accumulator register. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `CP A, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xB8 CP A,B`,  `0xB9 CP A,C`,  `0xBA CP A,D`,
 *                                  `0xBB CP A,E`,  `0xBC CP A,H`,  `0xBD CP A,L`,
 *                                  `0xBF CP A,A`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 *                                  `C` - Set if borrow (i.e., if src > A); Clear otherwise.
 */
GB_API bool gbExecuteCP_A_R8 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes a `CP A, (R16)` instruction, which compares the 8-bit value
 *          stored at the memory address contained in the given CPU context's
 *          specified 16-bit register pair with the `A` accumulator register by
 *          performing a subtraction and setting flags accordingly, without
 *          storing the result.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be compared with the
 *                      `A` accumulator register. Must be one of the 16-bit
 *                      register pairs: `BC`, `DE`, or `HL`.
 * 
 * @return  `true` if the `CP A, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xBE CP A,(HL)`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 *                                  `C` - Set if borrow (i.e., if value > A); Clear otherwise.
 */
GB_API bool gbExecuteCP_A_pR16 (gbProcessor* processor,
    gbRegisterType src);

/**
 * @brief   Executes a `CP A, D8` instruction, which compares an immediate 8-bit
 *          value with the `A` accumulator register within the given CPU context
 *          by performing a subtraction and setting flags accordingly, without
 *          storing the result.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The immediate 8-bit value to be compared with the `A`
 *                      accumulator register.
 * 
 * @return  `true` if the `CP A, D8` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xFE CP A,D8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Set.
 *                                  `H` - Set if borrow from bit 4; Clear otherwise.
 *                                  `C` - Set if borrow (i.e., if value > A); Clear otherwise.
 */
GB_API bool gbExecuteCP_A_D8 (gbProcessor* processor,
    uint8_t src);

/* Public Function Declarations - 16-Bit Arithmetic Instructions **************/

/**
 * @brief   Executes an `INC R16` instruction, which increments the value of
 *          the specified 16-bit register pair by 1 within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair to be
 *                      incremented. Must be one of the 16-bit register pairs:
 *                      `BC`, `DE`, `HL`, or `SP`.
 * 
 * @return  `true` if the `INC R16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was
 *          provided.
 * 
 * @note    Instruction Opcodes:    `0x03 INC BC`,  `0x13 INC DE`,  `0x23 INC HL`,
 *                                  `0x33 INC SP`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None.
 */
GB_API bool gbExecuteINC_R16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `ADD HL, R16` instruction, which adds the value of the
 *          specified 16-bit register pair to the `HL` register pair within the
 *          given CPU context, storing the result in the `HL` register pair.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src     The @a `gbRegisterType` 16-bit register pair whose value is
 *                      to be added to the `HL` register pair. Must be one of the
 *                      16-bit register pairs: `BC`, `DE`, `HL`, or `SP`.
 * 
 * @return  `true` if the `ADD HL, R16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was
 *          provided.
 * 
 * @note    Instruction Opcodes:    `0x09 ADD HL,BC`,  `0x19 ADD HL,DE`,
 *                                  `0x29 ADD HL,HL`,  `0x39 ADD HL,SP`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `N` - Clear.
 *                                  `H` - Set if carry from bit 11; Clear otherwise.
 *                                  `C` - Set if carry from bit 15; Clear otherwise.    
 */
GB_API bool gbExecuteADD_HL_R16 (gbProcessor* processor, gbRegisterType src);

/**
 * @brief   Executes a `DEC R16` instruction, which decrements the value of
 *          the specified 16-bit register pair by 1 within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair to be
 *                      decremented. Must be one of the 16-bit register pairs:
 *                      `BC`, `DE`, `HL`, or `SP`.
 * 
 * @return  `true` if the `DEC R16` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was
 *          provided.
 *
 * @note    Instruction Opcodes:    `0x0B DEC BC`,  `0x1B DEC DE`,  `0x2B DEC HL`,
 *                                  `0x3B DEC SP`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None.
 */
GB_API bool gbExecuteDEC_R16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `ADD SP, S8` instruction, which adds a signed 8-bit
 *          immediate value to the `SP` (Stack Pointer) register within the
 *          given CPU context, storing the result in the `SP` register.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   src         The signed 8-bit immediate value to be added to the
 *                      `SP` register.
 *
 * @return  `true` if the `ADD SP, S8` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0xE8 ADD SP,S8`
 * @note    Instruction Length:     2 Bytes (Opcode + Immediate Byte)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Clear.
 *                                  `N` - Clear.
 *                                  `H` - Set if carry from bit 3; Clear otherwise.
 *                                  `C` - Set if carry from bit 7; Clear otherwise.
 */
GB_API bool gbExecuteADD_SP_S8 (gbProcessor* processor, int8_t src);

/* Public Function Declarations - 8-Bit Rotate/Shift/Bit Instructions *********/

/**
 * @brief   Executes an `RLCA` instruction, which rotates the bits of the `A`
 *          accumulator register to the left by one position, with the most
 *          significant bit (bit 7) being moved to both the least significant
 *          bit (bit 0) and the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `RLCA` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x07 RLCA`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Clear.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteRLCA (gbProcessor* processor);

/**
 * @brief   Executes an `RLA` instruction, which rotates the bits of the `A`
 *          accumulator register to the left by one position through the carry
 *          flag, with the carry flag being moved to bit 0 and bit 7 being
 *          moved to the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `RLA` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x17 RLA`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Clear.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteRLA (gbProcessor* processor);

/**
 * @brief   Executes an `RRCA` instruction, which rotates the bits of the `A`
 *          accumulator register to the right by one position, with the least
 *          significant bit (bit 0) being moved to both the most significant
 *          bit (bit 7) and the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `RRCA` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x0F RRCA`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Clear.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteRRCA (gbProcessor* processor);

/**
 * @brief   Executes an `RRA` instruction, which rotates the bits of the `A`
 *          accumulator register to the right by one position through the carry
 *          flag, with the carry flag being moved to bit 7 and bit 0 being
 *          moved to the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * 
 * @return  `true` if the `RRA` instruction was executed successfully;
 *          `false` if an invalid processor pointer was provided.
 * 
 * @note    Instruction Opcodes:    `0x1F RRA`
 * @note    Instruction Length:     1 Byte (Opcode)
 * @note    Instruction Timing:     1 M-Cycle
 * @note    Flags Affected:         `Z` - Clear.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteRRA (gbProcessor* processor);

/**
 * @brief   Executes a `RLC R8` instruction, which rotates the bits of the
 *          specified 8-bit register to the left by one position, with the
 *          most significant bit (bit 7) being moved to both the least significant
 *          bit (bit 0) and the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register to be rotated. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `RLC R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB00 RLC B`,  `0xCB01 RLC C`,  `0xCB02 RLC D`,
 *                                  `0xCB03 RLC E`,  `0xCB04 RLC H`,  `0xCB05 RLC L`,
 *                                  `0xCB07 RLC A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteRLC_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `RLC (R16)` instruction, which rotates the bits of the
 *          8-bit value stored at the memory address contained in the specified
 *          16-bit register pair to the left by one position, with the most
 *          significant bit (bit 7) being moved to both the least significant
 *          bit (bit 0) and the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be rotated. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `RLC (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB06 RLC (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteRLC_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `RRC R8` instruction, which rotates the bits of the
 *          specified 8-bit register to the right by one position, with the
 *          least significant bit (bit 0) being moved to both the most significant
 *          bit (bit 7) and the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register to be rotated. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `RRC R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB08 RRC B`,  `0xCB09 RRC C`,  `0xCB0A RRC D`,
 *                                  `0xCB0B RRC E`,  `0xCB0C RRC H`,  `0xCB0D RRC L`,
 *                                  `0xCB0F RRC A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteRRC_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `RRC (R16)` instruction, which rotates the bits of the
 *          8-bit value stored at the memory address contained in the specified
 *          16-bit register pair to the right by one position, with the least
 *          significant bit (bit 0) being moved to both the most significant
 *          bit (bit 7) and the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be rotated. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `RRC (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB0E RRC (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteRRC_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `RL R8` instruction, which rotates the bits of the
 *          specified 8-bit register to the left by one position through the
 *          carry flag, with the carry flag being moved to bit 0 and bit 7
 *          being moved to the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register to be rotated. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `RL R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB10 RL B`,  `0xCB11 RL C`,  `0xCB12 RL D`,
 *                                  `0xCB13 RL E`,  `0xCB14 RL H`,  `0xCB15 RL L`,
 *                                  `0xCB17 RL A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteRL_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `RL (R16)` instruction, which rotates the bits of the
 *          8-bit value stored at the memory address contained in the specified
 *          16-bit register pair to the left by one position through the carry
 *          flag, with the carry flag being moved to bit 0 and bit 7 being
 *          moved to the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be rotated. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `RL (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB16 RL (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteRL_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `RR R8` instruction, which rotates the bits of the
 *          specified 8-bit register to the right by one position through the
 *          carry flag, with the carry flag being moved to bit 7 and bit 0
 *          being moved to the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register to be rotated. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `RR R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB18 RR B`,  `0xCB19 RR C`,  `0xCB1A RR D`,
 *                                  `0xCB1B RR E`,  `0xCB1C RR H`,  `0xCB1D RR L`,
 *                                  `0xCB1F RR A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteRR_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `RR (R16)` instruction, which rotates the bits of the
 *          8-bit value stored at the memory address contained in the specified
 *          16-bit register pair to the right by one position through the carry
 *          flag, with the carry flag being moved to bit 7 and bit 0 being
 *          moved to the carry flag within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be rotated. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `RR (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB1E RR (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteRR_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `SLA R8` instruction, which shifts the bits of the
 *          specified 8-bit register to the left by one position, with bit 0
 *          being set to 0 and bit 7 being moved to the carry flag within the
 *          given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register to be shifted. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `SLA R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB20 SLA B`,  `0xCB21 SLA C`,  `0xCB22 SLA D`,
 *                                  `0xCB23 SLA E`,  `0xCB24 SLA H`,  `0xCB25 SLA L`,
 *                                  `0xCB27 SLA A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteSLA_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `SLA (R16)` instruction, which shifts the bits of the
 *          8-bit value stored at the memory address contained in the specified
 *          16-bit register pair to the left by one position, with bit 0 being
 *          set to 0 and bit 7 being moved to the carry flag within the given
 *          CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be shifted. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `SLA (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB26 SLA (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 7 of the original value.
 */
GB_API bool gbExecuteSLA_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `SRA R8` instruction, which shifts the bits of the
 *          specified 8-bit register to the right by one position arithmetically,
 *          preserving bit 7 (the sign bit) and moving bit 0 to the carry flag
 *          within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register to be shifted. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `SRA R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB28 SRA B`,  `0xCB29 SRA C`,  `0xCB2A SRA D`,
 *                                  `0xCB2B SRA E`,  `0xCB2C SRA H`,  `0xCB2D SRA L`,
 *                                  `0xCB2F SRA A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteSRA_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `SRA (R16)` instruction, which shifts the bits of the
 *          8-bit value stored at the memory address contained in the specified
 *          16-bit register pair to the right by one position arithmetically,
 *          preserving bit 7 (the sign bit) and moving bit 0 to the carry flag
 *          within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be shifted. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `SRA (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB2E SRA (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteSRA_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `SWAP R8` instruction, which swaps the upper and lower
 *          nibbles (4-bit halves) of the specified 8-bit register within the
 *          given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register whose nibbles are
 *                      to be swapped. Must be the `A` accumulator register, or
 *                      one of the general-purpose registers: `B`, `C`, `D`, `E`,
 *                      `H`, or `L`.
 * 
 * @return  `true` if the `SWAP R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB30 SWAP B`,  `0xCB31 SWAP C`,  `0xCB32 SWAP D`,
 *                                  `0xCB33 SWAP E`,  `0xCB34 SWAP H`,  `0xCB35 SWAP L`,
 *                                  `0xCB37 SWAP A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteSWAP_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `SWAP (R16)` instruction, which swaps the upper and lower
 *          nibbles (4-bit halves) of the 8-bit value stored at the memory
 *          address contained in the specified 16-bit register pair within the
 *          given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value whose nibbles are to
 *                      be swapped. Must be the `HL` register pair.
 * 
 * @return  `true` if the `SWAP (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB36 SWAP (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Clear.
 */
GB_API bool gbExecuteSWAP_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `SRL R8` instruction, which shifts the bits of the
 *          specified 8-bit register to the right by one position logically,
 *          with bit 7 being set to 0 and bit 0 being moved to the carry flag
 *          within the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 8-bit register to be shifted. Must be
 *                      the `A` accumulator register, or one of the general-purpose
 *                      registers: `B`, `C`, `D`, `E`, `H`, or `L`.
 * 
 * @return  `true` if the `SRL R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB38 SRL B`,  `0xCB39 SRL C`,  `0xCB3A SRL D`,
 *                                  `0xCB3B SRL E`,  `0xCB3C SRL H`,  `0xCB3D SRL L`,
 *                                  `0xCB3F SRL A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteSRL_R8 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes an `SRL (R16)` instruction, which shifts the bits of the
 *          8-bit value stored at the memory address contained in the specified
 *          16-bit register pair to the right by one position logically, with
 *          bit 7 being set to 0 and bit 0 being moved to the carry flag within
 *          the given CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be shifted. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `SRL (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    `0xCB3E SRL (HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         `Z` - Set if result is zero; Clear otherwise.
 *                                  `N` - Clear.
 *                                  `H` - Clear.
 *                                  `C` - Bit 0 of the original value.
 */
GB_API bool gbExecuteSRL_pR16 (gbProcessor* processor, gbRegisterType dest);

/**
 * @brief   Executes a `BIT U3, R8` instruction, which tests the specified bit
 *          in the given 8-bit register within the CPU context and sets the
 *          appropriate flags.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   bit_index   The zero-based index (0-7) of the bit to be tested.
 * @param   src     The @a `gbRegisterType` 8-bit register to be tested.
 *                      Must be the `A` accumulator register, or one of the
 *                      general-purpose registers: `B`, `C`, `D`, `E`, `H`, 
 *                      or `L`.
 * 
 * @return  `true` if the `BIT U3, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    Many, from `0xCB40 BIT 0,B` to
 *                                  `0xCBBF BIT 7,A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         `Z` - Set if the tested bit is 0; Clear if it is 1.
 *                                  `N` - Clear.
 *                                  `H` - Set.
 */
GB_API bool gbExecuteBIT_U3_R8 (gbProcessor* processor,
    uint8_t bit_index, gbRegisterType src);

/**
 * @brief   Executes a `BIT U3, (R16)` instruction, which tests the specified bit
 *          in the 8-bit value stored at the memory address contained in the given
 *          16-bit register pair within the CPU context and sets the appropriate flags.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   bit_index   The zero-based index (0-7) of the bit to be tested.
 * @param   src     The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value to be tested. Must
 *                      be the `HL` register pair.
 * 
 * @return  `true` if the `BIT U3, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    Many, from `0xCB46 BIT 0,(HL)` to
 *                                  `0xCBFE BIT 7,(HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     3 M-Cycles
 * @note    Flags Affected:         `Z` - Set if the tested bit is 0; Clear if it is 1.
 *                                  `N` - Clear.
 *                                  `H` - Set.
 */
GB_API bool gbExecuteBIT_U3_pR16 (gbProcessor* processor,
    uint8_t bit_index, gbRegisterType src);

/**
 * @brief   Executes a `RES U3, R8` instruction, which resets (clears to 0) the
 *          specified bit in the given 8-bit register within the CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   bit_index   The zero-based index (0-7) of the bit to be reset.
 * @param   dest        The @a `gbRegisterType` 8-bit register in which to reset
 *                      the bit. Must be the `A` accumulator register, or one of
 *                      the general-purpose registers: `B`, `C`, `D`, `E`, `H`,
 *                      or `L`.
 * 
 * @return  `true` if the `RES U3, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    Many, from `0xCB80 RES 0,B` to
 *                                  `0xCBBF RES 7,A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None.
 */
GB_API bool gbExecuteRES_U3_R8 (gbProcessor* processor,
    uint8_t bit_index, gbRegisterType dest);

/**
 * @brief   Executes a `RES U3, (R16)` instruction, which resets (clears to 0)
 *          the specified bit in the 8-bit value stored at the memory address
 *          contained in the given 16-bit register pair within the CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   bit_index   The zero-based index (0-7) of the bit to be reset.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value in which to reset
 *                      the bit. Must be the `HL` register pair.
 * 
 * @return  `true` if the `RES U3, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    Many, from `0xCB86 RES 0,(HL)` to
 *                                  `0xCBFE RES 7,(HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         None.
 */
GB_API bool gbExecuteRES_U3_pR16 (gbProcessor* processor,
    uint8_t bit_index, gbRegisterType dest);

/**
 * @brief   Executes a `SET U3, R8` instruction, which sets (sets to 1) the
 *          specified bit in the given 8-bit register within the CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   bit_index   The zero-based index (0-7) of the bit to be set.
 * @param   dest        The @a `gbRegisterType` 8-bit register in which to set
 *                      the bit. Must be the `A` accumulator register, or one of
 *                      the general-purpose registers: `B`, `C`, `D`, `E`, `H`,
 *                      or `L`.
 * 
 * @return  `true` if the `SET U3, R8` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    Many, from `0xCBC0 SET 0,B` to
 *                                  `0xCBFF SET 7,A`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     2 M-Cycles
 * @note    Flags Affected:         None.
 */
GB_API bool gbExecuteSET_U3_R8 (gbProcessor* processor,
    uint8_t bit_index, gbRegisterType dest);

/**
 * @brief   Executes a `SET U3, (R16)` instruction, which sets (sets to 1)
 *          the specified bit in the 8-bit value stored at the memory address
 *          contained in the given 16-bit register pair within the CPU context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      execute the instruction. Pass `nullptr` to specify the
 *                      @a `gbProcessor` component associated with the current
 *                      Game Boy Emulator context.
 * @param   bit_index   The zero-based index (0-7) of the bit to be set.
 * @param   dest        The @a `gbRegisterType` 16-bit register pair which contains
 *                      the memory address of the 8-bit value in which to set
 *                      the bit. Must be the `HL` register pair.
 * 
 * @return  `true` if the `SET U3, (R16)` instruction was executed successfully;
 *          `false` if an invalid processor pointer or register type was provided.
 * 
 * @note    Instruction Opcodes:    Many, from `0xCBC6 SET 0,(HL)` to
 *                                  `0xCBFE SET 7,(HL)`
 * @note    Instruction Length:     2 Bytes (`0xCB` Prefix + Opcode)
 * @note    Instruction Timing:     4 M-Cycles
 * @note    Flags Affected:         None.
 */
GB_API bool gbExecuteSET_U3_pR16 (gbProcessor* processor,
    uint8_t bit_index, gbRegisterType dest);
