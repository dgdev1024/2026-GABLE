/**
 * @file    GB/Processor.h
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains declarations for the Game Boy Emulator Core's CPU processor
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Context.h>

/* Public Types and Forward Declarations **************************************/

/**
 * @brief   Defines a pointer to a function called by the Game Boy Emulator Core's
 *          CPU processor component when a new instruction is fetched from memory
 *          and is about to be executed.
 * 
 * @param   context         A pointer to the @a `gbContext` whose processor is
 *                          fetching the instruction.
 * @param   address         The 16-bit, absolute address from which the instruction's
 *                          opcode (and any prefix bytes) were fetched.
 * @param   opcode          A 16-bit value, the lower byte of which contains the
 *                          instruction's opcode. If the instruction is a prefixed
 *                          instruction (i.e., uses the `0xCB` prefix), the upper
 *                          byte contains the prefix byte (`0xCB`); otherwise, it
 *                          is `0x00`.
 * 
 * @return  The callback function can return `true` to allow the @a `gbProcessor`
 *          to proceed with executing the fetched instruction as normal, or `false`
 *          to skip execution of the instruction (effectively treating it as a
 *          `NOP` instruction). This can be useful for debugging, logging, or
 *          modifying the behavior of certain instructions at runtime.
 */
typedef bool (*gbInstructionFetchCallback) (gbContext* context,
    uint16_t address, uint16_t opcode);

/**
 * @brief   Defines a pointer to a function called by the Game Boy Emulator Core's
 *          CPU processor component immediately after an instruction has been
 *          executed.
 *
 * @param   context         A pointer to the @a `gbContext` whose processor executed the
 *                          instruction.
 * @param   address         The 16-bit, absolute address from which the instruction's
 *                          opcode (and any prefix bytes) were fetched.
 * @param   opcode          A 16-bit value, the lower byte of which contains the
 *                          instruction's opcode. If the instruction is a prefixed
 *                          instruction (i.e., uses the `0xCB` prefix), the upper
 *                          byte contains the prefix byte (`0xCB`); otherwise, it
 *                          is `0x00`.
 * @param   success         A boolean value indicating whether the instruction
 *                          executed successfully, without errors.
 */
typedef void (*gbInstructionExecuteCallback) (gbContext* context,
    uint16_t address, uint16_t opcode, bool success);

/**
 * @brief   Defines a pointer to a function called by the Game Boy Emulator Core's
 *          CPU processor component when it services an interrupt.
 * 
 * @param   context         A pointer to the @a `gbContext` whose processor is servicing
 *                          the interrupt.
 * @param   interrupt       The type of interrupt being serviced.
 */
typedef void (*gbInterruptServiceCallback) (gbContext* context,
    uint8_t interrupt);

/**
 * @brief   Defines a pointer to a function called by the Game Boy Emulator Core's
 *          CPU processor component in response to executing one of the `RST`
 *          restart vector instructions.
 * 
 * @param   context         A pointer to the @a `gbContext` whose processor is invoking
 *                          the restart vector. 
 * @param   restartVector   The restart vector address being invoked.
 */
typedef void (*gbRestartVectorCallback) (gbContext* context,
    uint16_t restartVector);

/* Public Constants and Enumerations ******************************************/

/**
 * @brief   Enumerates the types of registers available in the Game Boy CPU's
 *          register file.
 */
typedef enum gbRegisterType : uint8_t
{
    // 8-Bit General-Purpose Registers
    GB_RT_A,    /** @brief `A` - Accumulator Register */
    GB_RT_F,    /** @brief `F` - Processor Flags Register */
    GB_RT_B,    /** @brief `B` - General-Purpose Register */
    GB_RT_C,    /** @brief `C` - General-Purpose Register */
    GB_RT_D,    /** @brief `D` - General-Purpose Register */
    GB_RT_E,    /** @brief `E` - General-Purpose Register */
    GB_RT_H,    /** @brief `H` - General-Purpose Register */
    GB_RT_L,    /** @brief `L` - General-Purpose Register */

    // 16-Bit General-Purpose Register Pairs
    GB_RT_AF,   /** @brief `AF` - Accumulator & Flags Register Pair */
    GB_RT_BC,   /** @brief `BC` - General-Purpose Register Pair */
    GB_RT_DE,   /** @brief `DE` - General-Purpose Register Pair */
    GB_RT_HL,   /** @brief `HL` - General-Purpose Register Pair */

    // 16-Bit Special-Purpose Registers
    GB_RT_SP,   /** @brief `SP` - Stack Pointer Register */
    GB_RT_PC    /** @brief `PC` - Program Counter Register */
} gbRegisterType;

/**
 * @brief   Enumerates the processor flags accessible via the Game Boy CPU's
 *          `F` processor flags register.
 */
typedef enum gbProcessorFlag : uint8_t
{
    GB_PF_Z = 7,    /** @brief `Z` - Zero Flag */
    GB_PF_N = 6,    /** @brief `N` - Subtract Flag */
    GB_PF_H = 5,    /** @brief `H` - Half Carry Flag */
    GB_PF_C = 4     /** @brief `C` - Carry Flag */
} gbProcessorFlag;

/**
 * @brief   Enumerates the conditions used for the Game Boy CPU instruction
 *          set's branching instructions.
 */
typedef enum gbCondition : uint8_t
{   
    GB_CT_NONE = 0, /** @brief No condition - always branch */
    GB_CT_Z = 1,    /** @brief `Z` - Branch if Zero flag is set */
    GB_CT_NZ = 2,   /** @brief `NZ` - Branch if Zero flag is not set */
    GB_CT_C = 3,    /** @brief `C` - Branch if Carry flag is set */
    GB_CT_NC = 4,   /** @brief `NC` - Branch if Carry flag is not set */
} gbCondition;

/**
 * @brief   Enumerates the types of interrupts which can be requested of the
 *          Game Boy's CPU by its other hardware components.
 */
typedef enum gbInterrupt : uint8_t
{
    /**
     * @brief   `VBLANK` - Vertical Blank Interrupt
     * 
     * This interrupt is requested by the Game Boy's pixel processing unit (PPU)
     * component once it has finished drawing all visible scanlines for the
     * current frame and has entered the vertical blanking period. This
     * interrupt is typically used to perform tasks such as updating graphics
     * data, changing palettes, or preparing for the next frame's rendering, and
     * is used by many games to synchronize game logic and run their main game
     * loops.
     */
    GB_INT_VBLANK = 0,

    /**
     * @brief   `LCD_STAT` - Display Status Interrupt
     * 
     * This interrupt is requested by the Game Boy's pixel processing unit (PPU)
     * component based on one or more of the following conditions:
     * 
     * - The PPU has finished rendering all pixels of a visible scanline and
     *   entered the horizontal blanking period (`HBLANK`).
     * 
     * - The PPU has finished rendering all visible scanlines for the current
     *   frame and has entered the vertical blanking period (`VBLANK`).
     * 
     * - The PPU begins processing the next visible scanline and enters the
     *   object attribute memory (OAM) search period (`OBJECT_SCAN`).
     * 
     * - The PPU encounters a line coincidence condition, where the current
     *   scanline being rendered/processed (the value in the `LY` hardware 
     *   register) matches the value set in the scanline compare (`LYC`) hardware
     *   register.
     * 
     * This interrupt is often used to perform mid-frame graphical updates,
     * such as changing background or sprite attributes, updating palettes, or
     * performing other timing-sensitive operations that need to occur at specific
     * points during the frame rendering process.
     */
    GB_INT_LCD_STAT = 1,

    /**
     * @brief   `TIMER` - Timer Overflow Interrupt
     * 
     * This interrupt is requested by the Game Boy's timer component when its
     * counter (the `TIMA` hardware register) overflows and changes from `0xFF`
     * back to `0x00`. This interrupt is typically used to implement 
     * timing-based events in games, such as updating game logic, managing
     * animations, or handling other periodic tasks that need to occur at
     * regular intervals.
     */
    GB_INT_TIMER = 2,

    /**
     * @brief   `SERIAL` - Serial Transfer Completion Interrupt
     * 
     * This interrupt is requested by the Game Boy's serial communication
     * component when it has completed a serial data transfer operation. This
     * interrupt is used to signal that data has been fully transmitted or
     * received via the Game Boy's serial port, allowing the CPU to process the
     * transferred data or initiate further communication as needed. This
     * interrupt is particularly relevant for multiplayer games that utilize
     * the Game Boy's link cable functionality.
     */
    GB_INT_SERIAL = 3,

    /**
     * @brief   `JOYPAD` - Joypad Input Interrupt
     * 
     * This interrupt is requested by the Game Boy's joypad input component when
     * a button on the joypad, which is part of the selected button group (as
     * specified in the `P1/JOYP` hardware register), and was previously unpressed,
     * is pressed by the user. This interrupt allows the CPU to respond to user
     * input events, enabling games to react to button presses for actions such
     * as moving characters, selecting menu options, or performing in-game actions.
     */
    GB_INT_JOYPAD = 4,

    GB_INT_ENGINE1 = 5, /** @brief Engine Mode Specific Interrupt #1 */
    GB_INT_ENGINE2 = 6, /** @brief Engine Mode Specific Interrupt #2 */
    GB_INT_ENGINE3 = 7  /** @brief Engine Mode Specific Interrupt #3 */
} gbInterrupt;

/**
 * @brief   Enumerates the restart vector addresses used by the Game Boy CPU's
 *          `RST` (Restart) instructions.
 */
typedef enum gbRestartVector : uint16_t
{
    GB_RV_00H = 0x00,
    GB_RV_08H = 0x08,
    GB_RV_10H = 0x10,
    GB_RV_18H = 0x18,
    GB_RV_20H = 0x20,
    GB_RV_28H = 0x28,
    GB_RV_30H = 0x30,
    GB_RV_38H = 0x38
} gbRestartVector;

/* Public Unions and Structures ***********************************************/

/**
 * @brief   Defines a bitfield union representing the Game Boy CPU's `IE` and
 *          `IF` interrupt registers, indicating which CPU interrupts are enabled
 *          and which interrupts are currently pending, respectively.
 */
typedef union gbRegisterINT
{
    struct
    {
        uint8_t vblank  : 1;    /** @brief Bit 0: `VBLANK` Interrupt Enable/Pending (`1` = Yes; `0` = No) */
        uint8_t lcdStat : 1;    /** @brief Bit 1: `LCD_STAT` Interrupt Enable/Pending (`1` = Yes; `0` = No) */
        uint8_t timer   : 1;    /** @brief Bit 2: `TIMER` Interrupt Enable/Pending (`1` = Yes; `0` = No) */
        uint8_t serial  : 1;    /** @brief Bit 3: `SERIAL` Interrupt Enable/Pending (`1` = Yes; `0` = No) */
        uint8_t joypad  : 1;    /** @brief Bit 4: `JOYPAD` Interrupt Enable/Pending (`1` = Yes; `0` = No) */
        uint8_t         : 3;
    };

    uint8_t raw;    /** @brief The raw, 8-bit value of the register. */
} gbRegisterINT;

/**
 * @brief   Defines a bitfield union representing the Game Boy CPU's `KEY0` 
 *          hardware register, which controls DMG compatibility mode in CGB mode.
 * 
 * In CGB mode, this register is initialized depending on whether or not an
 * original, DMG Game Boy game is detected in the inserted cartridge, in order
 * to facilitate proper compatibility with such games.
 */
typedef union gbRegisterKEY0
{
    struct
    {
        uint8_t                     : 2;
        uint8_t dmgCompatibility    : 1;    /** @brief Bit 2: DMG Compatibility Mode (`1` = Enabled; `0` = Disabled) */
        uint8_t                     : 5;
    };

    uint8_t raw;    /** @brief The raw, 8-bit value of the register. */
} gbRegisterKEY0;

/**
 * @brief   Defines a bitfield union representing the Game Boy CPU's `KEY1`
 *          hardware register, which indicates the CPU's current speed mode and
 *          whether a speed switch is armed (pending) in CGB mode.
 */
typedef union gbRegisterKEY1
{
    struct
    {
        uint8_t speedSwitchArmed    : 1;    /** @brief Bit 0: Speed Switch Armed Flag (`1` = Armed; `0` = Not Armed) */
        uint8_t                     : 6;
        uint8_t speedMode           : 1;    /** @brief Bit 7: Current Speed Mode (`1` = Double Speed; `0` = Normal Speed) */
    };

    uint8_t raw;    /** @brief The raw, 8-bit value of the register. */
} gbRegisterKEY1;

/**
 * @brief   Defines a bitfield union representing the Game Boy CPU's `F` flags
 *          register, which contains the processor's status flags.
 */
typedef union gbRegisterF
{
    struct
    {
        uint8_t                 : 4;
        uint8_t carryFlag       : 1;    /** @brief Bit 4: `C` - Carry Flag */
        uint8_t halfCarryFlag   : 1;    /** @brief Bit 5: `H` - Half Carry Flag */
        uint8_t subtractFlag    : 1;    /** @brief Bit 6: `N` - Subtract Flag */
        uint8_t zeroFlag        : 1;    /** @brief Bit 7: `Z` - Zero Flag */
    };

    uint8_t raw;    /** @brief The raw, 8-bit value of the register. */
} gbRegisterF;

/**
 * @brief   Defines a structure representing the Game Boy CPU's register file,
 *          which contains all general-purpose and special-purpose CPU registers.
 */
typedef struct gbProcessorRegisterFile
{
    uint8_t         accumulator;    /** @brief `A` - Accumulator Register */
    gbRegisterF     flags;          /** @brief `F` - Processor Flags Register */
    uint8_t         b;              /** @brief `B` - General-Purpose Register */
    uint8_t         c;              /** @brief `C` - General-Purpose Register */
    uint8_t         d;              /** @brief `D` - General-Purpose Register */
    uint8_t         e;              /** @brief `E` - General-Purpose Register */
    uint8_t         h;              /** @brief `H` - General-Purpose Register */
    uint8_t         l;              /** @brief `L` - General-Purpose Register */
    uint16_t        stackPointer;   /** @brief `SP` - Stack Pointer Register */
    uint16_t        programCounter; /** @brief `PC` - Program Counter Register */
} gbProcessorRegisterFile;

/* Public Function Declarations ***********************************************/

/**
 * @brief   Allocates and creates a new CPU processor component for the given
 *          Game Boy Emulator Core context.
 * 
 * @param   parentContext   A pointer to the @a `gbContext` structure which will
 *                          own this processor component. Must not be `nullptr`.
 * 
 * @return  If successful, a pointer to the newly created @a `gbProcessor` structure.
 *          If allocation fails or if invalid parameters are provided, returns
 *          `nullptr`.
 */
GB_API gbProcessor* gbCreateProcessor (gbContext* parentContext);

/**
 * @brief   Destroys and deallocates a CPU processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to be destroyed.
 * 
 * @return  If successful, returns `true`.
 *          If no processor component is provided (i.e., `nullptr`), returns `false`.
 */
GB_API bool gbDestroyProcessor (gbProcessor* processor);

/**
 * @brief   Initializes (or resets) a CPU processor component.
 * 
 * This function initializes (or resets) the provided @a `gbProcessor` structure,
 * setting all registers to their default power-on values and clearing any
 * internal state.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to be initialized.
 * 
 * @return  If successful, returns `true`.
 *          If no processor component is provided (i.e., `nullptr`), returns `false`.
 */
GB_API bool gbInitializeProcessor (gbProcessor* processor);

/* Public Function Declarations - Helper Functions ****************************/

/**
 * @brief   Retrieves the parent context of the given CPU processor component.
 * 
 * This is needed by those instruction execution functions that require access
 * to the address space or other components of the parent context.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure whose parent
 *                      context is to be retrieved. Must not be `nullptr`.
 * 
 * @return  If successful, returns a pointer to the parent @a `gbContext` structure.
 *          If an invalid processor component is provided (i.e., `nullptr`), returns
 *          `nullptr`.
 */
GB_API gbContext* gbGetProcessorContext (gbProcessor* processor);

/**
 * @brief   Retrieves a string representation of the given CPU register type.
 * 
 * @param   regType     The @a `gbRegisterType` enumeration value representing
 *                      the CPU register type. Must be a valid value.
 * 
 * @return  If successful, returns a pointer to a null-terminated string
 *          representing the name of the register type.
 *          If an invalid register type is provided, returns `nullptr`.
 */
GB_API const char* gbStringifyRegisterType (gbRegisterType regType);

/**
 * @brief   Sets the current "Hybrid Mode" state of the given CPU processor.
 * 
 * This function is intended for internal use only, and does nothing if the CPU's
 * parent context is not operating in Engine Mode.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure for which to
 *                      set the instruction fetch callback. Pass `nullptr` to use
 *                      the current context's processor.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, or if the processor's parent context is not operating in
 *          Engine Mode, returns `false`.
 */
GB_API bool gbSetHybridMode (gbProcessor* processor, bool hybridMode);

/* Public Function Declarations - Callbacks ***********************************/

/**
 * @brief   Sets the callback function to be invoked when an instruction is fetched
 *          by the CPU processor component and is about to be executed.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure for which to
 *                      set the instruction fetch callback. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   callback    A pointer to the @a `gbInstructionFetchCallback` function
 *                      to set as the callback. Pass `nullptr` to unset any
 *                      existing callback.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `false`.
 */
GB_API bool gbSetInstructionFetchCallback (gbProcessor* processor, gbInstructionFetchCallback callback);

/**
 * @brief   Sets the callback function to be invoked when an instruction has been
 *          executed by the CPU processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure for which to
 *                      set the instruction execute callback. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   callback    A pointer to the @a `gbInstructionExecuteCallback` function
 *                      to set as the callback. Pass `nullptr` to unset any
 *                      existing callback.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `false`.
 */
GB_API bool gbSetInstructionExecuteCallback (gbProcessor* processor, gbInstructionExecuteCallback callback);

/**
 * @brief   Sets the callback function to be invoked when an interrupt is serviced
 *          by the CPU processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure for which to
 *                      set the interrupt service callback. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   callback    A pointer to the @a `gbInterruptServiceCallback` function
 *                      to set as the callback. Pass `nullptr` to unset any
 *                      existing callback.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `false`.
 */
GB_API bool gbSetInterruptServiceCallback (gbProcessor* processor, gbInterruptServiceCallback callback);

/**
 * @brief   Sets the callback function to be invoked when a restart vector is
 *          used by the CPU processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure for which to
 *                      set the restart vector callback. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   callback    A pointer to the @a `gbRestartVectorCallback` function
 *                      to set as the callback. Pass `nullptr` to unset any
 *                      existing callback.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `false`.
 */
GB_API bool gbSetRestartVectorCallback (gbProcessor* processor, gbRestartVectorCallback callback);

/**
 * @brief   Invokes the restart vector callback function for the given CPU
 *          processor component, if one is set.
 * 
 * This function is called in response to the execution of one of the CPU's `RST`
 * (Restart) instructions, and calls the registered restart vector callback function.
 * This function is intended for internal use only, and should not be called
 * directly by external code.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure for which
 *                          to invoke the restart vector callback. Pass `nullptr`
 *                          to use the current context's processor.
 * @param   restartVector   The restart vector address being invoked.
 * 
 * @return  If successful, or if no restart vector callback is set, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `false`.
 */
GB_API bool gbInvokeRestartVectorCallback (gbProcessor* processor,
    uint16_t restartVector);

/* Public Function Declarations - Ticking and Timing **************************/

/**
 * @brief   Ticks the given CPU processor component, causing it to possibly
 *          fetch, decode and execute an instruction; and/or service a pending
 *          interrupt.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to be ticked.
 * 
 * @return  If successful, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbTickProcessor (gbProcessor* processor);

/**
 * @brief   Consumes the specified number of CPU tick cycles ("T-cycles") on
 *          the given CPU processor component.
 * 
 * A "tick cycle" (henceforth, a "T-cycle") represents the smallest unit of time
 * measurement in the Game Boy CPU's operation. The CPU consumes T-cycles as it
 * accesses the memory bus, executes instructions, and performs other internal 
 * tasks. Every time a T-cycle is consumed, the CPU's internal clock ticks the
 * other components of the Game Boy Emulator Core accordingly, causing them to
 * perform their operations in sync with the CPU, ensuring accurate emulation of
 * the Game Boy's timing behavior.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which
 *                          to consume T-cycles.
 * @param   tickCycles      The number of T-cycles to consume.
 * 
 * @return  If ticking causes the other components to tick succesfully, without
 *          errors, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbConsumeTickCycles (gbProcessor* processor, size_t tickCycles);

/**
 * @brief   Consumes the specified number of CPU machine cycles ("M-cycles") on
 *          the given CPU processor component.
 * 
 * T-cycles in the Game Boy's CPU are most often consumed in multiples of four
 * (or two, in double-speed mode). This multiple of T-cycles is referred to as a
 * "machine cycle" (henceforth, an "M-cycle"). Many CPU operations, such as
 * memory accesses and instruction executions, are measured in M-cycles. By
 * consuming M-cycles, the CPU processor component can accurately simulate the
 * timing behavior of the original Game Boy hardware.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which
 *                          to consume M-cycles.
 * @param   machineCycles   The number of M-cycles to consume.
 * 
 * @return  If ticking causes the other components to tick succesfully, without
 *          errors, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbConsumeMachineCycles (gbProcessor* processor, size_t machineCycles);

/**
 * @brief   Helper function used by the instruction execution functions to
 *          simulate M-cycles consumed from instruction data fetches in Engine
 *          Mode. Does nothing if not in Engine Mode.
 * 
 * This function is exactly the same as the @a `gbConsumeMachineCycles` function, 
 * except that this function does nothing if the CPU's parent context is not
 * operating in Engine Mode, or if the CPU is currently in Hybrid Mode.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which
 *                          to consume fetch cycles.
 * @param   fetchCycles     The number of M-cycles to consume for instruction
 *                          data fetches.
 * 
 * @return  If ticking causes the other components to tick succesfully, without
 *          errors, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbConsumeFetchCycles (gbProcessor* processor, size_t fetchCycles);

/* Public Function Declarations - Processor Registers and Flags ***************/

/**
 * @brief   Retrieves a pointer to the register file of the given CPU processor
 *          component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure from which to
 *                      retrieve the register file. Pass `nullptr` to use the
 *                      current context's processor.
 * 
 * @return  If successful, returns a pointer to the associated
 *          @a `gbProcessorRegisterFile`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `nullptr`.
 */
GB_API const gbProcessorRegisterFile* gbGetRegisterFile (const gbProcessor* processor);

/**
 * @brief   Reads the value of the specified 8-bit general-purpose register from
 *          the given CPU processor component.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure from which
 *                          to read the register value. Pass `nullptr` to use the
 *                          current context's processor.
 * @param   registerType    The type of register to read. Must be one of the 8-bit
 *                          general-purpose registers (`A`, `F`, `B`, `C`, `D`, 
 *                          `E`, `H`, or `L`).
 * @param   outValue        A pointer to a `uint8_t` variable where the
 *                          retrieved register value will be stored. Must not be
 *                          `nullptr`.
 * 
 * @return  If read successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; if an invalid register type is specified; or if `outValue`
 *          is `nullptr`, returns `false`.
 */
GB_API bool gbReadRegisterByte (const gbProcessor* processor, gbRegisterType registerType, uint8_t* outValue);

/**
 * @brief   Reads the value of the specified 16-bit register from the given CPU
 *          processor component.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure from which
 *                          to read the register value. Pass `nullptr` to use the
 *                          current context's processor.
 * @param   registerType    The type of register to read. Must be one of the
 *                          16-bit registers (`AF`, `BC`, `DE`, `HL`, `SP`, or
 *                          `PC`).
 * @param   outValue        A pointer to a `uint16_t` variable where the
 *                          retrieved register value will be stored. Must not be
 *                          `nullptr`.  
 * 
 * @return  If read successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; if an invalid register type is specified; or if `outValue`
 *          is `nullptr`, returns `false`.
 */
GB_API bool gbReadRegisterWord (const gbProcessor* processor, gbRegisterType registerType, uint16_t* outValue);

/**
 * @brief   Reads the value of the specified processor flag from the given CPU
 *          processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure from which
 *                      to read the flag value. Pass `nullptr` to use the current
 *                      context's processor.
 * @param   flag        The type of processor flag to read. Must be one of the
 *                      defined @a `gbProcessorFlag` values (`Z`, `N`, `H`, or `C`).
 * @param   outValue    A pointer to a `bool` variable where the retrieved flag
 *                      value will be stored. Must not be `nullptr`.
 * 
 * @return  If read successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; if an invalid flag type is specified; or if `outValue` is
 *          `nullptr`, returns `false`.
 */
GB_API bool gbReadFlag (const gbProcessor* processor, gbProcessorFlag flag, bool* outValue);
GB_API bool gbWriteRegisterByte (gbProcessor* processor, gbRegisterType registerType, uint8_t value);
GB_API bool gbWriteRegisterWord (gbProcessor* processor, gbRegisterType registerType, uint16_t value);
GB_API bool gbWriteFlag (gbProcessor* processor, gbProcessorFlag flag, bool value);

/* Public Function Declarations - Interrupts **********************************/

/**
 * @brief   Disables CPU interrupts on the given CPU processor component by
 *          clearing the `IME` (Interrupt Master Enable) flag.
 * 
 * This function is typically called in response to the CPU executing the `DI`
 * (disable interrupts) instruction, and immediately prevents the CPU from
 * servicing any further interrupts until they are re-enabled.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      disable interrupts. Pass `nullptr` to use the current
 *                      context's processor.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `false`.
 */
GB_API bool gbDisableInterrupts (gbProcessor* processor);

/**
 * @brief   Enables CPU interrupts on the given CPU processor component by
 *          setting the `IME` (Interrupt Master Enable) flag, either immediately
 *          or after the next instruction is executed.
 * 
 * This function is typically called in response to the CPU executing the `EI`
 * (enable interrupts) or `RETI` (return from interrupt) instructions. Depending
 * on the value of the `immediately` parameter, interrupts may be enabled
 * either immediately or after the next instruction is executed.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which
 *                          to enable interrupts. Pass `nullptr` to use the
 *                          current context's processor.
 * @param   immediately     A boolean value indicating whether to enable
 *                          interrupts immediately (`true`, in response to the
 *                          `RETI` instruction) or after the next instruction is
 *                          executed (`false`, in response to the `EI` instruction).
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists, returns `false`.
 */
GB_API bool gbEnableInterrupts (gbProcessor* processor, bool immediately);

/**
 * @brief   Checks to see if the `IME` (Interrupt Master Enable) flag is set on
 *          the given CPU processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      check the `IME` flag. Pass `nullptr` to use the current
 *                      context's processor.
 * @param   outEnabled  A pointer to a `bool` variable where the result will be
 *                      stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; or if `outEnabled` is `nullptr`, returns `false`.
 */
GB_API bool gbCheckInterruptMasterEnabled (const gbProcessor* processor, bool* outEnabled);

/**
 * @brief   Checks to see if the `IME` (Interrupt Master Enable) flag is primed
 *          to be set on the given CPU processor component after it executes the
 *          next instruction (i.e., if interrupts were enabled via the `EI` 
 *          instruction).
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure on which to
 *                      check the `IME` pending flag. Pass `nullptr` to use the
 *                      current context's processor.
 * @param   outPending  A pointer to a `bool` variable where the result will be
 *                      stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; or if `outPending` is `nullptr`, returns `false`.
 */
GB_API bool gbCheckInterruptMasterPending (const gbProcessor* processor, bool* outPending);

/**
 * @brief   Checks to see if a specific interrupt is enabled on the given CPU
 *          processor component.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which to
 *                          check the interrupt enable status. Pass `nullptr` to
 *                          use the current context's processor.
 * @param   interrupt       The type of interrupt to check. Must be one of the 
 *                          defined @a `gbInterrupt` values.
 * @param   outEnabled      A pointer to a `bool` variable where the result will be
 *                          stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; if an invalid interrupt type is specified; or if `outEnabled`
 *          is `nullptr`, returns `false`.
 */
GB_API bool gbCheckInterruptEnabled (const gbProcessor* processor, gbInterrupt interrupt, bool* outEnabled);

/**
 * @brief   Checks to see if a specific interrupt is currently pending on the
 *          given CPU processor component.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which to
 *                          check the interrupt pending status. Pass `nullptr` to
 *                          use the current context's processor.
 * @param   interrupt       The type of interrupt to check. Must be one of the 
 *                          defined @a `gbInterrupt` values.
 * @param   outPending      A pointer to a `bool` variable where the result will be
 *                          stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; if an invalid interrupt type is specified; or if `outPending`
 *          is `nullptr`, returns `false`.
 */
GB_API bool gbCheckInterruptPending (const gbProcessor* processor, gbInterrupt interrupt, bool* outPending);

/**
 * @brief   Checks to see if any interrupts are currently pending on the given
 *          CPU processor component.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which to
 *                          check for pending interrupts. Pass `nullptr` to use
 *                          the current context's processor.
 * @param   outPending      A pointer to a `bool` variable where the result will be
 *                          stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; or if `outPending` is `nullptr`, returns `false`.
 */
GB_API bool gbCheckAnyInterruptPending (const gbProcessor* processor, bool* outPending);

/**
 * @brief   Requests a specific interrupt on the given CPU processor component by
 *          setting its corresponding bit in the `IF` (Interrupt Flag) register.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which to
 *                          request the interrupt. Pass `nullptr` to use the
 *                          current context's processor.
 * @param   interrupt       The type of interrupt to request. Must be one of the 
 *                          defined @a `gbInterrupt` values.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; or if an invalid interrupt type is specified, returns `false`.
 */
GB_API bool gbRequestInterrupt (gbProcessor* processor, gbInterrupt interrupt);

/**
 * @brief   Cancels a specific interrupt on the given CPU processor component by
 *          clearing its corresponding bit in the `IF` (Interrupt Flag) register.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which to
 *                          cancel the interrupt. Pass `nullptr` to use the
 *                          current context's processor.
 * @param   interrupt       The type of interrupt to cancel. Must be one of the 
 *                          defined @a `gbInterrupt` values.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; or if an invalid interrupt type is specified, returns `false`.
 */
GB_API bool gbCancelInterrupt (gbProcessor* processor, gbInterrupt interrupt);

/**
 * @brief   Services the highest-priority pending interrupt on the given CPU
 *          processor component, if interrupts are enabled.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure on which to
 *                          service the interrupt. Pass `nullptr` to use the
 *                          current context's processor.
 * 
 * @return  If an interrupt was successfully serviced, returns `true`.
 *          If no interrupt was serviced (either because none were pending or
 *          because interrupts were disabled), returns `false`.
 */
GB_API bool gbServiceInterrupt (gbProcessor* processor);

/* Public Function Declarations - `HALT` and `STOP` ***************************/

/**
 * @brief   Causes the given CPU processor component to enter the `HALT` state,
 *          also checking for and triggering the `HALT` bug if applicable.
 * 
 * In the `HALT` state, the CPU processor component ceases executing instructions
 * until an interrupt is requested. However, if interrupts are disabled and no
 * interrupt is pending, the CPU enters a low-power mode but does not advance the
 * program counter.
 * 
 * This function also checks for the conditions that trigger the infamous
 * `HALT` bug. If the `HALT` bug is triggered, the CPU's program counter will not
 * increment on the next instruction fetch, causing the same instruction to be
 * fetched and executed twice.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to enter the
 *                      `HALT` state. Pass `nullptr` to use the current context's
 *                      processor.
 * 
 * @return  If successful, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbEnterHaltState (gbProcessor* processor);

/**
 * @brief   Causes the given CPU processor component to exit the `HALT` state.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to exit the
 *                      `HALT` state. Pass `nullptr` to use the current context's
 *                      processor.
 * 
 * @return  If successful, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbExitHaltState (gbProcessor* processor);

/**
 * @brief   Causes the given CPU processor component to enter the `STOP` state.
 * 
 * In the `STOP` state, the CPU processor's clock stops entirely until an
 * external event (such as a button press, a reset or some other event) wakes
 * it up. This state is typically used to conserve power when the system is
 * idle.
 * 
 * If a `STOP` state is attempted while the CPU's speed switch is armed (pending),
 * the speed switch is performed instead, toggling the CPU between normal and
 * double speed modes.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to enter the
 *                      `STOP` state. Pass `nullptr` to use the current context's
 *                      processor.
 * 
 * @return  If successful, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbEnterStopState (gbProcessor* processor);

/**
 * @brief   Causes the given CPU processor component to exit the `STOP` state.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to exit the
 *                      `STOP` state. Pass `nullptr` to use the current context's
 *                      processor.
 * 
 * @return  If successful, returns `true`.
 *          If an error occurs or if no processor is provided (i.e., `nullptr`),
 *          returns `false`.
 */
GB_API bool gbExitStopState (gbProcessor* processor);

/**
 * @brief   Checks to see if the given CPU processor component is currently in
 *          the `HALT` state.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to check the
 *                      `HALT` state. Pass `nullptr` to use the current context's
 *                      processor.
 * @param   outHalted   A pointer to a `bool` variable where the result will be
 *                      stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) or if `outHalted` is
 *          `nullptr`, returns `false`.
 */
GB_API bool gbCheckHaltState (const gbProcessor* processor, bool* outHalted);

/**
 * @brief   Checks to see if the `HALT` bug is currently active on the given CPU
 *          processor component.
 * 
 * The `HALT` bug occurs when the CPU enters the `HALT` state while interrupts
 * are disabled and at least one interrupt is pending. In this scenario, the CPU's
 * program counter does not increment on the next instruction fetch, causing the
 * same instruction to be fetched and executed twice (or `0xCB` to be fetched
 * twice if the instruction is a prefixed one).
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to check for
 *                      the `HALT` bug. Pass `nullptr` to use the current context's
 *                      processor.
 * @param   outHaltBug  A pointer to a `bool` variable where the result will be
 *                      stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) or if `outHaltBug` is
 *          `nullptr`, returns `false`.
 */
GB_API bool gbCheckForHaltBug (const gbProcessor* processor, bool* outHaltBug);

/**
 * @brief   Checks to see if the given CPU processor component is currently in
 *          the `STOP` state.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to check the
 *                      `STOP` state. Pass `nullptr` to use the current context's
 *                      processor.
 * @param   outStopped  A pointer to a `bool` variable where the result will be
 *                      stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) or if `outStopped` is
 *          `nullptr`, returns `false`.
 */
GB_API bool gbCheckStopState (const gbProcessor* processor, bool* outStopped);

/* Public Function Declarations - Speed Mode **********************************/

/**
 * @brief   Checks to see if the CPU speed switch is currently armed (pending)
 *          on the given CPU processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to check the
 *                      speed switch state. Pass `nullptr` to use the current
 *                      context's processor.
 * @param   outArmed    A pointer to a `bool` variable where the result will be
 *                      stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) or if `outArmed` is
 *          `nullptr`, returns `false`.
 */
GB_API bool gbCheckSpeedSwitchArmed (const gbProcessor* processor, bool* outArmed);

/**
 * @brief   Checks to see if the CPU speed switch is currently in progress
 *          (i.e., if a speed switch has been requested but not yet completed)
 *          on the given CPU processor component.
 * 
 * During an ongoing speed switch, the CPU temporarily halts instruction
 * execution for a brief period to safely transition between normal and double
 * speed modes. During this brief period, the Game Boy Emulator Core operates in
 * a strange state where the timer component does not tick (even its divider does
 * not advance), the PPU's internal memory access rules are altered, and other
 * hardware components may behave differently.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure to check the
 *                          speed switch state. Pass `nullptr` to use the current
 *                          context's processor.
 * @param   outSwitching    A pointer to a `bool` variable where the result will be
 *                          stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) or if `outSwitching` is
 *          `nullptr`, returns `false`.
 */
GB_API bool gbCheckSpeedSwitchState (const gbProcessor* processor, bool* outSwitching);

/**
 * @brief   Checks to see if the CPU is currently operating in double speed mode
 *          on the given CPU processor component.
 * 
 * @param   processor       A pointer to the @a `gbProcessor` structure to check the
 *                          current speed mode. Pass `nullptr` to use the current
 *                          context's processor.
 * @param   outDoubleSpeed  A pointer to a `bool` variable where the result will be
 *                          stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) or if `outDoubleSpeed` is
 *          `nullptr`, returns `false`.
 */
GB_API bool gbCheckCurrentSpeedMode (const gbProcessor* processor, bool* outDoubleSpeed);

/* Public Function Declarations - Hardware Register Access ********************/

/**
 * @brief   Reads the value of the `IF` hardware register from the given CPU
 *          processor component.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure from which to
 *                      read the `IF` register. Pass `nullptr` to use the current
 *                      context's processor.
 * @param   outValue    A pointer to a `uint8_t` variable where the retrieved
 *                      register value will be stored. Must not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when reading the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If read successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current processor
 *          exists; or if `outValue` is `nullptr`, returns `false`.
 */
GB_API bool gbReadIF (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules);

/**
 * @brief   Reads the value of the `IE` (Interrupt Enable) hardware register
 *          from the given CPU processor component.
 * 
 * The `IE` register (at address `$FFFF`) controls which interrupt handlers
 * may be called. Each bit corresponds to a specific interrupt type:
 * 
 * - Bit 0: VBlank interrupt
 * 
 * - Bit 1: LCD STAT interrupt
 * 
 * - Bit 2: Timer interrupt
 * 
 * - Bit 3: Serial interrupt
 * 
 * - Bit 4: Joypad interrupt
 * 
 * An interrupt will only be serviced if both its corresponding bit in `IE`
 * is set and the `IME` (Interrupt Master Enable) flag is enabled.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure from which
 *                      to read the `IE` register. Pass `nullptr` to use the
 *                      current context's processor.
 * @param   outValue    A pointer to a `uint8_t` variable where the retrieved
 *                      register value will be stored. Must not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when reading the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If read successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current
 *          processor exists; or if `outValue` is `nullptr`, returns `false`.
 */
GB_API bool gbReadIE (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules);

/**
 * @brief   Reads the value of the `KEY0` (CPU Mode Select) hardware register
 *          from the given CPU processor component.
 * 
 * The `KEY0` register (at address `$FF4C`) is a CGB-only register that
 * controls DMG compatibility mode. It is written only by the CGB boot ROM
 * and becomes locked after boot ROM execution completes. Once locked, the
 * system's behavior cannot be changed without a reset.
 * 
 * - Bit 0: DMG compatibility mode (`0` = Disabled/full CGB mode,
 *          `1` = Enabled/DMG compatibility mode)
 * 
 * - Bits 1-7: Reserved/unused
 * 
 * This register is used to ensure proper compatibility when running
 * original DMG (monochrome Game Boy) cartridges on CGB hardware.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure from which
 *                      to read the `KEY0` register. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   outValue    A pointer to a `uint8_t` variable where the retrieved
 *                      register value will be stored. Must not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when reading the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If read successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current
 *          processor exists; or if `outValue` is `nullptr`, returns `false`.
 */
GB_API bool gbReadKEY0 (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules);

/**
 * @brief   Reads the value of the `KEY1` (Prepare Speed Switch) hardware
 *          register from the given CPU processor component.
 * 
 * The `KEY1` register (at address `$FF4D`) is a CGB-only register used to
 * prepare the Game Boy to switch between normal speed and double speed
 * modes. The actual speed switch occurs when a `STOP` instruction is
 * executed after bit 0 has been set.
 * 
 * - Bit 0: Switch armed (`0` = No, `1` = Armed for speed switch)
 * 
 * - Bits 1-6: Unused
 * 
 * - Bit 7: Current speed (Read-only: `0` = Normal speed, `1` = Double speed)
 * 
 * In double speed mode, the CPU runs at 2.10 MHz (twice the normal speed),
 * while the LCD, HDMA, and audio timings remain unchanged.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure from which
 *                      to read the `KEY1` register. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   outValue    A pointer to a `uint8_t` variable where the retrieved
 *                      register value will be stored. Must not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when reading the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If read successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current
 *          processor exists; or if `outValue` is `nullptr`, returns `false`.
 */
GB_API bool gbReadKEY1 (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules);

/**
 * @brief   Writes a value to the `IF` hardware register of the given CPU
 *          processor component.
 * 
 * The `IF` register (at address `$FF0F`) is used to request interrupts by
 * setting the corresponding bits. Each bit corresponds to a specific interrupt
 * type:
 * 
 * - Bit 0: VBlank interrupt
 * 
 * - Bit 1: LCD STAT interrupt
 * 
 * - Bit 2: Timer interrupt
 * 
 * - Bit 3: Serial interrupt
 * 
 * - Bit 4: Joypad interrupt
 * 
 * Setting a bit to `1` requests the corresponding interrupt, while clearing it
 * to `0` cancels the request. The CPU will service requested interrupts
 * when they are enabled in the `IE` register and the `IME` flag is set.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to which
 *                      to write the `IF` register. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   value       The byte value to write to the `IF` register.
 * @param   outActual   A pointer to a `uint8_t` variable where the actual
 *                      value written to the register will be stored. Must
 *                      not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when writing the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If write successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current
 *          processor exists; or if `outActual` is `nullptr`, returns `false`.
 */
GB_API bool gbWriteIF (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);

/**
 * @brief   Writes a value to the `IE` (Interrupt Enable) hardware register
 *          of the given CPU processor component.
 * 
 * The `IE` register (at address `$FFFF`) controls which interrupt handlers
 * may be called. Each bit corresponds to a specific interrupt type:
 * 
 * - Bit 0: VBlank interrupt
 * 
 * - Bit 1: LCD STAT interrupt
 * 
 * - Bit 2: Timer interrupt
 * 
 * - Bit 3: Serial interrupt
 * 
 * - Bit 4: Joypad interrupt
 * 
 * Setting a bit to `1` enables the corresponding interrupt handler, while
 * clearing it to `0` disables it. An interrupt will only be serviced if
 * both its bit in `IE` is set and the `IME` flag is enabled.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to which
 *                      to write the `IE` register. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   value       The byte value to write to the `IE` register.
 * @param   outActual   A pointer to a `uint8_t` variable where the actual
 *                      value written to the register will be stored. Must
 *                      not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when writing the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If write successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current
 *          processor exists; or if `outActual` is `nullptr`, returns `false`.
 */
GB_API bool gbWriteIE (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);

/**
 * @brief   Writes a value to the `KEY0` (CPU Mode Select) hardware register
 *          of the given CPU processor component.
 * 
 * The `KEY0` register (at address `$FF4C`) is a CGB-only register that
 * controls DMG compatibility mode. It is written only by the CGB boot ROM
 * and becomes locked after boot ROM execution completes. Once locked,
 * writes have no effect and the system's behavior cannot be changed without
 * a reset.
 * 
 * - Bit 0: DMG compatibility mode (`0` = Disabled/full CGB mode,
 *          `1` = Enabled/DMG compatibility mode)
 * 
 * - Bits 1-7: Reserved/unused
 * 
 * Note: This register is typically not writable after boot ROM unmapping
 * except in special emulator modes or hardware test scenarios.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to which
 *                      to write the `KEY0` register. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   value       The byte value to write to the `KEY0` register.
 * @param   outActual   A pointer to a `uint8_t` variable where the actual
 *                      value written to the register will be stored. Must
 *                      not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when writing the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If write successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current
 *          processor exists; or if `outActual` is `nullptr`, returns `false`.
 */
GB_API bool gbWriteKEY0 (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);

/**
 * @brief   Writes a value to the `KEY1` (Prepare Speed Switch) hardware
 *          register of the given CPU processor component.
 * 
 * The `KEY1` register (at address `$FF4D`) is a CGB-only register used to
 * prepare the Game Boy for a speed switch between normal and double speed
 * modes. Writing `$01` to this register arms the speed switch, which will
 * be executed when a `STOP` instruction is subsequently executed.
 * 
 * - Bit 0: Switch armed (Write `1` to arm speed switch, cleared after switch)
 * 
 * - Bits 1-6: Unused (writes ignored)
 * 
 * - Bit 7: Current speed (Read-only, writes ignored)
 * 
 * After a `STOP` instruction with bit 0 set, the CPU halts for 2050 M-cycles
 * (8200 T-cycles) while switching speeds, during which the system enters a
 * special state where DIV doesn't tick and VRAM/OAM access behaves
 * differently depending on the PPU mode when the switch begins.
 * 
 * @param   processor   A pointer to the @a `gbProcessor` structure to which
 *                      to write the `KEY1` register. Pass `nullptr` to use
 *                      the current context's processor.
 * @param   value       The byte value to write to the `KEY1` register.
 * @param   outActual   A pointer to a `uint8_t` variable where the actual
 *                      value written to the register will be stored. Must
 *                      not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` structure defining
 *                      any access rules to enforce when writing the register.
 *                      Pass `nullptr` to use default rules.
 * 
 * @return  If write successfully, returns `true`.
 *          If no processor is provided (i.e., `nullptr`) and no current
 *          processor exists; or if `outActual` is `nullptr`, returns `false`.
 */
GB_API bool gbWriteKEY1 (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules);
