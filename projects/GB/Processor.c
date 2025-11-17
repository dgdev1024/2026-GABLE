/**
 * @file    GB/Processor.c
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-17
 * 
 * @brief   Contains implementations for the Game Boy Emulator Core's CPU
 *          processor component.
 */

/* Private Includes ***********************************************************/

#include <GB/Processor.h>

/* Private Constants and Enumerations *****************************************/

/**
 * @brief   Defines the number of interrupts supported by the Game Boy CPU.
 */
static const uint8_t GB_INTERRUPT_COUNT = 5;

/**
 * @brief   Defines the number of interrupts supported by the Game Boy CPU when
 *          its parent context is operating in Engine Mode.
 */
static const uint8_t GB_INTERRUPT_COUNT_ENGINE = 8;

/**
 * @brief   Defines a lookup table mapping `gbRegisterType` values to their
 *          corresponding human-readable register names.
 */
static const char* GB_REGISTER_NAMES[] = {
    [GB_RT_A]   = "A",
    [GB_RT_F]   = "F",
    [GB_RT_B]   = "B",
    [GB_RT_C]   = "C",
    [GB_RT_D]   = "D",
    [GB_RT_E]   = "E",
    [GB_RT_H]   = "H",
    [GB_RT_L]   = "L",
    [GB_RT_AF]  = "AF",
    [GB_RT_BC]  = "BC",
    [GB_RT_DE]  = "DE",
    [GB_RT_HL]  = "HL",
    [GB_RT_SP]  = "SP",
    [GB_RT_PC]  = "PC",
    [0xFF]      = "??"
};

/**
 * @brief   Defines a lookup table mapping `gbInterrupt` values to their
 *          corresponding human-readable interrupt names.
 */
static const char* GB_INTERRUPT_NAMES[] = {
    [GB_INT_VBLANK]    = "VBLANK",
    [GB_INT_LCD_STAT]  = "LCD_STAT",
    [GB_INT_TIMER]     = "TIMER",
    [GB_INT_SERIAL]    = "SERIAL",
    [GB_INT_JOYPAD]    = "JOYPAD",
    [GB_INT_ENGINE1]   = "ENGINE1",
    [GB_INT_ENGINE2]   = "ENGINE2",
    [GB_INT_ENGINE3]   = "ENGINE3",
    [0xFF]             = "??"
};

/* Private Unions and Structures **********************************************/

struct gbProcessor
{
    // Parent Context
    gbContext*                      parent;

    // Callbacks
    gbInstructionFetchCallback      instructionFetchCallback;
    gbInstructionExecuteCallback    instructionExecuteCallback;
    gbInterruptServiceCallback      interruptServiceCallback;

    // Register File and Hardware Registers
    gbProcessorRegisterFile         registers;
    gbRegisterKEY0                  key0;
    gbRegisterKEY1                  key1;
    gbRegisterINT                   ienable;
    gbRegisterINT                   iflags;

    // Internal State
    size_t                          tickCyclesConsumed;
    uint16_t                        fetchedOpcodeAddress;
    uint16_t                        fetchedOpcode;
    uint16_t                        fetchedWordAddress;
    uint16_t                        fetchedWord;
    uint16_t                        fetchedByteAddress;
    uint8_t                         fetchedByte;
    bool                            interruptMaster;
    bool                            interruptMasterPending;
    bool                            halted;
    bool                            stopped;
    bool                            haltBug;
    bool                            speedSwitching;
    bool                            isEngineMode;
    bool                            isCGBMode;

};

/* Private Function Declarations - Data Fetching ******************************/

static bool gbFetchOpcode (gbProcessor* processor);
static bool gbFetchIMM8 (gbProcessor* processor);
static bool gbFetchIMM16 (gbProcessor* processor);

/* Private Function Declarations - Instructions *******************************/

static bool gbExecuteInstruction (gbProcessor* processor, uint8_t opcode);
static bool gbExecuteInstructionCB (gbProcessor* processor, uint8_t opcode);

/* Private Function Definitions - Data Fetching *******************************/

bool gbFetchOpcode (gbProcessor* processor)
{
    gbAssert(processor);
    gbAssert(processor->isEngineMode == false);

    // - Store the address of the opcode being fetched.
    processor->fetchedOpcodeAddress = processor->registers.programCounter;

    // - Fetch a byte from memory at the current `PC` address.
    // - Consume one M-cycle for Memory Read.
    uint8_t opcode = 0x00;
    gbReadByte(processor->parent, processor->registers.programCounter,
        &opcode, nullptr);
        if (gbConsumeMachineCycles(processor, 1) == false)
        {
            gbLogError("Error consuming machine cycles during opcode fetch.");
            return false;
        }
        
    // - Advance the `PC` by one, unless the `HALT` bug is active.
    if (processor->haltBug == true)
        { processor->haltBug = false; }
    else
        { processor->registers.programCounter++; }

    // - Is this a prefix opcode (`0xCB`)?
    if (opcode == 0xCB)
    {
        // - Fetch the next byte as the actual opcode.
        // - Consume one M-cycle for Memory Read.
        uint8_t prefixedOpcode = 0x00;
        gbReadByte(processor->parent, processor->registers.programCounter,
            &prefixedOpcode, nullptr);
        if (gbConsumeMachineCycles(processor, 1) == false)
        {
            gbLogError("Error consuming machine cycles during prefixed opcode fetch.");
            return false;
        }

        // - Advance the `PC` by one.
        processor->registers.programCounter++;

        // - Store the full prefixed opcode in `fetchedOpcode`.
        processor->fetchedOpcode = (uint16_t) ((0xCB << 8) | prefixedOpcode);
    }   
    else
    {
        // - Otherwise, store the single-byte opcode in `fetchedOpcode`.
        processor->fetchedOpcode = opcode;
    } 

    return true;
}

bool gbFetchIMM8 (gbProcessor* processor)
{
    gbAssert(processor);
    gbAssert(processor->isEngineMode == false);

    // - Store the address of the immediate byte being fetched.
    processor->fetchedByteAddress = processor->registers.programCounter;

    // - Fetch a byte from memory at the current `PC` address.
    // - Consume one M-cycle for Memory Read.
    uint8_t value = 0x00;
    gbReadByte(processor->parent, processor->registers.programCounter,
        &value, nullptr);
    if (gbConsumeMachineCycles(processor, 1) == false)
    {
        gbLogError("Error consuming machine cycles during immediate byte fetch.");
        return false;
    }

    // - Advance the `PC` by one.
    processor->registers.programCounter++;

    // - Store the fetched byte.
    processor->fetchedByte = value;
    return true;
}

bool gbFetchIMM16 (gbProcessor* processor)
{
    gbAssert(processor);
    gbAssert(processor->isEngineMode == false);

    // - Store the address of the immediate word being fetched.
    processor->fetchedWordAddress = processor->registers.programCounter;

    // - Fetch the low byte from memory at the current `PC` address.
    // - Consume one M-cycle for Memory Read.
    uint8_t lowByte = 0x00;
    gbReadByte(processor->parent, processor->registers.programCounter,
        &lowByte, nullptr);
    if (gbConsumeMachineCycles(processor, 1) == false)
    {
        gbLogError("Error consuming machine cycles during immediate word low byte fetch.");
        return false;
    }

    // - Advance the `PC` by one.
    processor->registers.programCounter++;

    // - Fetch the high byte from memory at the current `PC` address.
    // - Consume one M-cycle for Memory Read.
    uint8_t highByte = 0x00;
    gbReadByte(processor->parent, processor->registers.programCounter,
        &highByte, nullptr);
    if (gbConsumeMachineCycles(processor, 1) == false)
    {
        gbLogError("Error consuming machine cycles during immediate word high byte fetch.");
        return false;
    }

    // - Advance the `PC` by one.
    processor->registers.programCounter++;

    // - Combine the low and high bytes into a 16-bit word and store it.
    processor->fetchedWord = (uint16_t) ((highByte << 8) | lowByte);
    return true;
}

/* Private Function Definitions - Instructions ********************************/

bool gbExecuteInstruction (gbProcessor* processor, uint8_t opcode)
{
    gbAssert(processor);

    switch (opcode)
    {
        default:
            gbLogError(
                "Invalid or unimplemented opcode '0x%02X' at address '$%04X'.",
                opcode, processor->fetchedOpcodeAddress);
            return false;
    }
}

bool gbExecuteInstructionCB (gbProcessor* processor, uint8_t opcode)
{
    gbAssert(processor);

    switch (opcode)
    {
        default:
            gbLogError(
                "Unimplemented opcode '0xCB%02X' at address '$%04X'.",
                opcode, processor->fetchedOpcodeAddress);
            return false;
    }
}

/* Public Function Definitions ************************************************/

gbProcessor* gbCreateProcessor (gbContext* parentContext)
{
    gbCheckv(parentContext != nullptr, nullptr,
        "Parent context pointer is null");

    gbProcessor* processor = gbCreateZero(1, gbProcessor);
    gbCheckpv(processor != nullptr, nullptr,
        "Error allocating memory for 'gbProcessor'");

    processor->parent = parentContext;
    return processor;
}

bool gbDestroyProcessor (gbProcessor* processor)
{
    gbCheckqv(processor, false);
    gbDestroy(processor);
    return true;
}

bool gbInitializeProcessor (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckqv(processor, false);

    // - Check for CGB Mode and Engine Mode
    gbCheckCGBMode(processor->parent, &processor->isCGBMode);
    gbCheckEngineMode(processor->parent, &processor->isEngineMode);

    // - Initialize Register File
    processor->registers.stackPointer   = 0xFFFE;
    processor->registers.programCounter = 0x0100;
    if (processor->isCGBMode == true)
    {
        processor->registers.accumulator    = 0x11;
        processor->registers.flags.raw      = 0b10000000;
        processor->registers.b              = 0x00;
        processor->registers.c              = 0x00;
        processor->registers.d              = 0xFF;
        processor->registers.e              = 0x56;
        processor->registers.h              = 0x00;
        processor->registers.l              = 0x0D;
    }
    else
    {
        processor->registers.accumulator    = 0x01;
        processor->registers.flags.raw      = 0b10000000;
        processor->registers.b              = 0x00;
        processor->registers.c              = 0x13;
        processor->registers.d              = 0x00;
        processor->registers.e              = 0xD8;
        processor->registers.h              = 0x01;
        processor->registers.l              = 0x4D;
    }

    // - Initialize Hardware Registers
    processor->ienable.raw = 0x00;
    processor->iflags.raw  = 0xE1;
    if (processor->isCGBMode == true)
    {
        processor->key0.raw = 0x00;
        processor->key1.raw = 0x7E;
    }
    else
    {
        processor->key0.raw = 0xFF;
        processor->key1.raw = 0x00;
    }

    // - Initialize Internal State
    processor->tickCyclesConsumed        = 0;
    processor->fetchedOpcodeAddress      = 0x0000;
    processor->fetchedOpcode             = 0x0000;
    processor->fetchedWordAddress        = 0x0000;
    processor->fetchedWord               = 0x0000;
    processor->fetchedByteAddress        = 0x0000;
    processor->fetchedByte               = 0x00;
    processor->interruptMaster           = false;
    processor->interruptMasterPending    = false;
    processor->halted                    = false;
    processor->stopped                   = false;
    processor->speedSwitching            = false;
    processor->haltBug                   = false;

    return true;
}

/* Public Function Definitions - Callbacks ************************************/

bool gbSetInstructionFetchCallback (gbProcessor* processor, gbInstructionFetchCallback callback)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    processor->instructionFetchCallback = callback;
    return true;
}

bool gbSetInstructionExecuteCallback (gbProcessor* processor, gbInstructionExecuteCallback callback)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    processor->instructionExecuteCallback = callback;
    return true;
}

bool gbSetInterruptServiceCallback (gbProcessor* processor, gbInterruptServiceCallback callback)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    processor->interruptServiceCallback = callback;
    return true;
}

/* Public Function Definitions - Ticking and Timing ***************************/

bool gbTickProcessor (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(processor->isEngineMode == false, false,
        "The 'gbProcessor' is in Engine Mode, which does not support ticking.");

    // - Check for `STOP` state. If so, do nothing this tick.
    if (processor->stopped == true)
        { return true; }

    // - Check for `HALT` state.
    if (processor->halted == true)
    {
        // - Exit `HALT` if any enabled interrupt is pending.
        bool anyInterruptPending = false;
        gbCheckAnyInterruptPending(processor, &anyInterruptPending);
        if (anyInterruptPending == true)
        {
            gbExitHaltState(processor);
        }
        else
        {
            return gbConsumeMachineCycles(processor, 1);
        }
    }

    // - Service a pending interrupt, if any.
    if (gbServiceInterrupt(processor) == false)
    {
        return false;
    }

    // - Prepare the fetch state for the next instruction.
    processor->fetchedOpcodeAddress = 0x0000;
    processor->fetchedOpcode        = 0x0000;
    processor->fetchedWordAddress   = 0x0000;
    processor->fetchedWord          = 0x0000;
    processor->fetchedByteAddress   = 0x0000;
    processor->fetchedByte          = 0x00;

    // - Fetch the next opcode.
    if (gbFetchOpcode(processor) == false)
    {
        return false;
    }

    // - Invoke the instruction fetch callback, if set.
    bool allowExecution = true;
    if (processor->instructionFetchCallback != nullptr)
    {
        allowExecution = processor->instructionFetchCallback(
            processor,
            processor->fetchedOpcodeAddress,
            processor->fetchedOpcode
        );
    }

    // - Execute the fetched opcode, if allowed.
    if (allowExecution == true)
    {
        // - Execute the instruction.
        bool success = false;
        if ((processor->fetchedOpcode & 0xFF00) == 0xCB00)
        {
            success = gbExecuteInstructionCB(processor,
                processor->fetchedOpcode & 0xFF);
        }
        else
        {
            success = gbExecuteInstruction(processor,
                processor->fetchedOpcode & 0xFF);
        }

        // - Invoke the instruction execute callback, if set.
        if (processor->instructionExecuteCallback != nullptr)
        {
            processor->instructionExecuteCallback(
                processor,
                processor->fetchedOpcodeAddress,
                processor->fetchedOpcode,
                success
            );
        }

        if (success == false)
        {
            return false;
        }
    }

    // - If the `IME` is pending activation, enable it now.
    if (processor->interruptMasterPending == true)
    {
        processor->interruptMaster = true;
        processor->interruptMasterPending = false;
    }

    return true;
}

bool gbConsumeTickCycles (gbProcessor* processor, size_t tickCycles)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    for (size_t i = 0; i < tickCycles; ++i)
    {
        processor->tickCyclesConsumed++;
    }

    return true;
}

bool gbConsumeMachineCycles (gbProcessor* processor, size_t machineCycles)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    return gbConsumeTickCycles(
        processor,
        machineCycles * ((processor->key1.speedMode == true) ? 2 : 4)
    );
}

/* Public Function Definitions - Processor Registers and Flags ****************/

const gbProcessorRegisterFile* gbGetRegisterFile (const gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, nullptr,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, nullptr,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    return &processor->registers;
}

bool gbReadRegisterByte (const gbProcessor* processor, gbRegisterType registerType, uint8_t* outValue)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outValue != nullptr, false,
        "The output value pointer is null.");

    switch (registerType)
    {
        case GB_RT_A:    *outValue = processor->registers.accumulator; break;
        case GB_RT_F:    *outValue = processor->registers.flags.raw & 0xF0; break;
        case GB_RT_B:    *outValue = processor->registers.b; break;
        case GB_RT_C:    *outValue = processor->registers.c; break;
        case GB_RT_D:    *outValue = processor->registers.d; break;
        case GB_RT_E:    *outValue = processor->registers.e; break;
        case GB_RT_H:    *outValue = processor->registers.h; break;
        case GB_RT_L:    *outValue = processor->registers.l; break;
        default:
            gbLogError("Register '%s' is not a valid 8-bit register.",
                GB_REGISTER_NAMES[registerType]);
            return false;
    }

    return true;
}

bool gbReadRegisterWord (const gbProcessor* processor, gbRegisterType registerType, uint16_t* outValue)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outValue != nullptr, false,
        "The output value pointer is null.");

    switch (registerType)
    {
        case GB_RT_AF:
            *outValue = (uint16_t) ((processor->registers.accumulator << 8) |
                (processor->registers.flags.raw & 0xF0));
            break;
        case GB_RT_BC:
            *outValue = (uint16_t) ((processor->registers.b << 8) |
                processor->registers.c);
            break;
        case GB_RT_DE:
            *outValue = (uint16_t) ((processor->registers.d << 8) |
                processor->registers.e);
            break;
        case GB_RT_HL:
            *outValue = (uint16_t) ((processor->registers.h << 8) |
                processor->registers.l);
            break;
        case GB_RT_SP:
            *outValue = processor->registers.stackPointer;
            break;
        case GB_RT_PC:
            *outValue = processor->registers.programCounter;
            break;
        default:
            gbLogError("Register '%s' is not a valid 16-bit register.",
                GB_REGISTER_NAMES[registerType]);
            return false;
    }

    return true;
}

bool gbReadFlag (const gbProcessor* processor, gbProcessorFlag flag, bool* outValue)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outValue != nullptr, false,
        "The output value pointer is null.");

    switch (flag)
    {
        case GB_PF_Z: *outValue = (processor->registers.flags.zeroFlag == 1); break;
        case GB_PF_N: *outValue = (processor->registers.flags.subtractFlag == 1); break;
        case GB_PF_H: *outValue = (processor->registers.flags.halfCarryFlag == 1); break;
        case GB_PF_C: *outValue = (processor->registers.flags.carryFlag == 1); break;
        default:
            gbLogError("Processor flag '%d' is not valid.", flag);
            return false;
    }

    return true;
}

bool gbWriteRegisterByte (gbProcessor* processor, gbRegisterType registerType, uint8_t value)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    switch (registerType)
    {
        case GB_RT_A: processor->registers.accumulator = value; break;
        case GB_RT_F: processor->registers.flags.raw = value & 0xF0; break;
        case GB_RT_B: processor->registers.b = value; break;
        case GB_RT_C: processor->registers.c = value; break;
        case GB_RT_D: processor->registers.d = value; break;
        case GB_RT_E: processor->registers.e = value; break;
        case GB_RT_H: processor->registers.h = value; break;
        case GB_RT_L: processor->registers.l = value; break;
        default:
            gbLogError("Register '%s' is not a valid 8-bit register.",
                GB_REGISTER_NAMES[registerType]);
            return false;
    }

    return true;
}

bool gbWriteRegisterWord (gbProcessor* processor, gbRegisterType registerType, uint16_t value)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    switch (registerType)
    {
        case GB_RT_AF:
            processor->registers.accumulator = (uint8_t) (value >> 8);
            processor->registers.flags.raw = (uint8_t) (value & 0xF0);
            break;
        case GB_RT_BC:
            processor->registers.b = (uint8_t) (value >> 8);
            processor->registers.c = (uint8_t) (value & 0xFF);
            break;
        case GB_RT_DE:
            processor->registers.d = (uint8_t) (value >> 8);
            processor->registers.e = (uint8_t) (value & 0xFF);
            break;
        case GB_RT_HL:
            processor->registers.h = (uint8_t) (value >> 8);
            processor->registers.l = (uint8_t) (value & 0xFF);
            break;
        case GB_RT_SP:
            processor->registers.stackPointer = value;
            break;
        case GB_RT_PC:
            processor->registers.programCounter = value;
            break;
        default:
            gbLogError("Register '%s' is not a valid 16-bit register.",
                GB_REGISTER_NAMES[registerType]);
            return false;
    }

    return true;
}

bool gbWriteFlag (gbProcessor* processor, gbProcessorFlag flag, bool value)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    switch (flag)
    {
        case GB_PF_Z: processor->registers.flags.zeroFlag = value ? 1 : 0; break;
        case GB_PF_N: processor->registers.flags.subtractFlag = value ? 1 : 0; break;
        case GB_PF_H: processor->registers.flags.halfCarryFlag = value ? 1 : 0; break;
        case GB_PF_C: processor->registers.flags.carryFlag = value ? 1 : 0; break;
        default:
            gbLogError("Processor flag '%d' is not valid.", flag);
            return false;
    }

    return true;
}

/* Public Function Definitions - Interrupts ***********************************/

bool gbDisableInterrupts (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    processor->interruptMaster = false;
    return true;
}

bool gbEnableInterrupts (gbProcessor* processor, bool immediately)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    if (immediately == true)
    {
        processor->interruptMaster = true;
    }
    else
    {
        processor->interruptMasterPending = true;
    }

    return true;
}

bool gbCheckInterruptMasterEnabled (const gbProcessor* processor, bool* outEnabled)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outEnabled != nullptr, false,
        "The output value pointer is null.");

    *outEnabled = processor->interruptMaster;
    return true;
}

bool gbCheckInterruptMasterPending (const gbProcessor* processor, bool* outPending)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outPending != nullptr, false,
        "The output value pointer is null.");

    *outPending = processor->interruptMasterPending;
    return true;
}

bool gbCheckInterruptEnabled (const gbProcessor* processor, gbInterrupt interrupt, bool* outEnabled)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outEnabled != nullptr, false,
        "The output value pointer is null.");
    gbCheckv(
        interrupt < (
            (processor->isEngineMode == true) ?
                GB_INTERRUPT_COUNT_ENGINE :
                GB_INTERRUPT_COUNT
        ),
        false,
        "Interrupt type '%d' is not valid in the current mode.", interrupt
    );

    *outEnabled = ((processor->ienable.raw >> interrupt) & 0x01) == 1;
    return true;
}

bool gbCheckInterruptPending (const gbProcessor* processor, gbInterrupt interrupt, bool* outPending)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outPending != nullptr, false,
        "The output value pointer is null.");
    gbCheckv(
        interrupt < (
            (processor->isEngineMode == true) ?
                GB_INTERRUPT_COUNT_ENGINE :
                GB_INTERRUPT_COUNT
        ),
        false,
        "Interrupt type '%d' is not valid in the current mode.", interrupt
    );

    *outPending = ((processor->iflags.raw >> interrupt) & 0x01) == 1;
    return true;
}

bool gbCheckAnyInterruptPending (const gbProcessor* processor, bool* outPending)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outPending != nullptr, false,
        "The output value pointer is null.");

    *outPending = ((processor->iflags.raw & processor->ienable.raw) != 0);
    return true;
}

bool gbRequestInterrupt (gbProcessor* processor, gbInterrupt interrupt)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(
        interrupt < (
            (processor->isEngineMode == true) ?
                GB_INTERRUPT_COUNT_ENGINE :
                GB_INTERRUPT_COUNT
        ),
        false,
        "Interrupt type '%d' is not valid in the current mode.", interrupt
    );

    processor->iflags.raw |= (1 << interrupt);
    return true;
}

bool gbCancelInterrupt (gbProcessor* processor, gbInterrupt interrupt)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(
        interrupt < (
            (processor->isEngineMode == true) ?
                GB_INTERRUPT_COUNT_ENGINE :
                GB_INTERRUPT_COUNT
        ),
        false,
        "Interrupt type '%d' is not valid in the current mode.", interrupt
    );

    processor->iflags.raw &= ~(1 << interrupt);
    return true;
}

bool gbServiceInterrupt (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    // - Don't service interrupts if the `IME` is disabled.
    if (processor->interruptMaster == false)
        { return true; }

    // - Iterate over all possible interrupts.
    for (
        uint8_t interrupt = 0;
        interrupt < (
            (processor->isEngineMode == true) ?
                GB_INTERRUPT_COUNT_ENGINE :
                GB_INTERRUPT_COUNT
        );
        ++interrupt
    )
    {
        // - Check if this interrupt is both requested and enabled.
        bool isRequested = ((processor->iflags.raw >> interrupt) & 0x01) == 1;
        bool isEnabled   = ((processor->ienable.raw >> interrupt) & 0x01) == 1;

        if (isRequested == true && isEnabled == true)
        {
            // - Acknowledge the interrupt by clearing its request flag, the
            //   `IME` and the `HALT` state.
            processor->iflags.raw &= ~(1 << interrupt);
            processor->interruptMaster = false;
            processor->halted = false;

            // - Service the interrupt:
            //   - Wait 2 M-cycles
            //   - Push `PC` High Byte
            //   - Consume 1 M-cycle for Memory Write
            //   - Push `PC` Low Byte
            //   - Consume 1 M-cycle for Memory Write
            //   - Update `SP`
            //   - Set `PC` to Interrupt Vector
            //   - Consume 1 M-cycle for Jump
            bool ok =
                gbConsumeMachineCycles(processor, 2) &&
                gbWriteByte(processor->parent, --processor->registers.stackPointer,
                    ((processor->registers.programCounter >> 8) & 0xFF), nullptr, nullptr) &&
                gbConsumeMachineCycles(processor, 1) &&
                gbWriteByte(processor->parent, --processor->registers.stackPointer,
                    (processor->registers.programCounter & 0xFF), nullptr, nullptr) &&
                gbConsumeMachineCycles(processor, 1) &&
                gbWriteRegisterWord(processor, GB_RT_PC, (0x40 + (interrupt * 8))) &&
                gbConsumeMachineCycles(processor, 1);

            if (ok == false)
            {
                gbLogError("Error servicing interrupt '%s'.",
                    GB_INTERRUPT_NAMES[interrupt]);
            }
            else if (processor->interruptServiceCallback != nullptr)
            {
                processor->interruptServiceCallback(processor, 
                    (gbInterrupt) interrupt);
            }

            return ok;
        }
    }

    return true;
}

/* Public Function Definitions - `HALT` and `STOP` ****************************/

bool gbEnterHaltState (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    // - Check for Engine Mode. If so, then the `HALT` bug cannot be simulated.
    //   Just enter `HALT` normally.
    if (processor->isEngineMode == true)
    {
        processor->halted = true;
        processor->haltBug = false;
        return true;
    }

    // - Check for the `HALT` bug condition.
    //   This will be triggered if the `HALT` instruction is executed while
    //   the `IME` is disabled and there is at least one pending interrupt.
    //   If the `HALT` bug condition is met, then the processor will not
    //   actually enter the `HALT` state, and the next opcode fetch will
    //   read from the current `PC` address again.
    bool anyInterruptPending = false;
    gbCheckAnyInterruptPending(processor, &anyInterruptPending);
    if (processor->interruptMaster == false && anyInterruptPending == true)
    {
        processor->haltBug = true;
        processor->halted = false;
    }
    else
    {
        processor->halted = true;
        processor->haltBug = false;
    }

    return true;
}

bool gbExitHaltState (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    processor->halted = false;
    return true;
}

bool gbEnterStopState (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    if (
        processor->isCGBMode == true && 
        processor->key1.speedSwitchArmed == true
    )
    {
        processor->key1.speedSwitchArmed = false;
        processor->speedSwitching = true;
        if (gbConsumeMachineCycles(processor, 2050) == false)
        {
            gbLogError("Error consuming machine cycles during speed switch process.");
            return false;
        }
        processor->speedSwitching = false;
        processor->key1.speedMode = !processor->key1.speedMode;

        return true;
    }

    processor->stopped = true;
    return true;
}

bool gbExitStopState (gbProcessor* processor)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    processor->stopped = false;
    return true;
}

bool gbCheckHaltState (const gbProcessor* processor, bool* outHalted)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outHalted != nullptr, false,
        "The output value pointer is null.");

    *outHalted = processor->halted;
    return true;
}

bool gbCheckForHaltBug (const gbProcessor* processor, bool* outHaltBug)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outHaltBug != nullptr, false,
        "The output value pointer is null.");

    *outHaltBug = processor->haltBug;
    return true;
}

bool gbCheckStopState (const gbProcessor* processor, bool* outStopped)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outStopped != nullptr, false,
        "The output value pointer is null.");

    *outStopped = processor->stopped;
    return true;
}


/* Public Function Definitions - Speed Mode ***********************************/

bool gbCheckSpeedSwitchArmed (const gbProcessor* processor, bool* outArmed)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outArmed != nullptr, false,
        "The output value pointer is null.");

    *outArmed = processor->key1.speedSwitchArmed;
    return true;
}

bool gbCheckSpeedSwitchState (const gbProcessor* processor, bool* outSwitching)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outSwitching != nullptr, false,
        "The output value pointer is null.");

    *outSwitching = processor->speedSwitching;
    return true;
}

bool gbCheckCurrentSpeedMode (const gbProcessor* processor, bool* outDoubleSpeed)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outDoubleSpeed != nullptr, false,
        "The output value pointer is null.");

    *outDoubleSpeed = processor->key1.speedMode;
    return true;
}

/* Public Function Definitions - Hardware Register Access *********************/

bool gbReadIF (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outValue != nullptr, false,
        "The output value pointer is null.");
        
    // - In Engine Mode:
    //   - All bits of `IF` are readable.
    // - Otherwise:
    //   - Bits 5-7 of `IF` are unused; read as `1`.
    //   - Bits 0-4 of `IF` are readable.
    if (processor->isEngineMode == true)
    {
        *outValue = processor->iflags.raw;
    }
    else
    {
        *outValue =
            0b11100000 |                            // Bits 5-7 unused, read as `1`
            (processor->iflags.raw & 0b00011111);   // Bits 0-4 readable
    }

    return true;
}

bool gbReadIE (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outValue != nullptr, false,
        "The output value pointer is null.");
        
    // - In Engine Mode:
    //   - All bits of `IE` are readable.
    // - Otherwise:
    //   - Bits 5-7 of `IE` are unused; read as `1`.
    //   - Bits 0-4 of `IE` are readable.
    if (processor->isEngineMode == true)
    {
        *outValue = processor->ienable.raw;
    }
    else
    {
        *outValue =
            0b11100000 |                            // Bits 5-7 unused, read as `1`
            (processor->ienable.raw & 0b00011111);  // Bits 0-4 readable
    }

    return true;
}

bool gbReadKEY0 (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outValue != nullptr, false,
        "The output value pointer is null.");

    // - In CGB Mode:
    //   - Bits 0-1 and 3-7 are unused; read as `1`.
    //   - Bit 2 is readable.
    // - Otherwise:
    //   - `KEY0` is not accessible; read as `0xFF`.
    if (processor->isCGBMode == true)
    {
        *outValue =
            0b11111011 |                            // Bits 0-1 and 3-7 unused, read as `1`
            (processor->key0.raw & 0b00000100);     // Bit 2 readable
    }
    else
    {
        *outValue = 0xFF;
    }

    return true;
}

bool gbReadKEY1 (const gbProcessor* processor, uint8_t* outValue, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    gbCheckv(outValue != nullptr, false,
        "The output value pointer is null.");

    // - In CGB Mode:
    //   - Bits 1-6 are unused; read as `1`.
    //   - Bits 0 and 7 are readable.
    // - Otherwise:
    //   - `KEY1` is not accessible; read as `0x00`.
    if (processor->isCGBMode == true)
    {
        *outValue =
            0b01111110 |                            // Bits 1-6 unused, read as `1`
            (processor->key1.raw & 0b10000001);     // Bits 0 and 7 readable
    }
    else
    {
        *outValue = 0x00;
    }

    return true;
}

bool gbWriteIF (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");
    
    // - In Engine Mode:
    //   - All bits of `IF` are writable.
    // - Otherwise:
    //   - Bits 5-7 of `IF` are unused; write as `1`.
    //   - Bits 0-4 of `IF` are writable.
    if (processor->isEngineMode == true)
    {
        processor->iflags.raw = value;
    }
    else
    {
        processor->iflags.raw =
            0b11100000 |                            // Bits 5-7 unused, write as `1`
            (value & 0b00011111);                   // Bits 0-4 writable
    }

    if (outActual != nullptr)
    {
        *outActual = processor->iflags.raw;
    }

    return true;
}

bool gbWriteIE (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    // - In Engine Mode:
    //   - All bits of `IE` are writable.
    // - Otherwise:
    //   - Bits 5-7 of `IE` are unused; write as `1`.
    //   - Bits 0-4 of `IE` are writable.
    if (processor->isEngineMode == true)
    {
        processor->ienable.raw = value;
    }
    else
    {
        processor->ienable.raw =
            0b11100000 |                            // Bits 5-7 unused, write as `1`
            (value & 0b00011111);                   // Bits 0-4 writable
    }

    if (outActual != nullptr)
    {
        *outActual = processor->ienable.raw;
    }

    return true;
}

bool gbWriteKEY0 (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    // - `KEY0` is read-only in all modes; ignore writes.
    if (outActual != nullptr)
    {
        *outActual = 0xFF;
    }

    return true;
}

bool gbWriteKEY1 (gbProcessor* processor, uint8_t value, uint8_t* outActual, const gbCheckRules* rules)
{
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor != nullptr, false,
        "No valid 'gbProcessor' provided, and no current processor is set.");
    gbCheckv(processor->parent != nullptr, false,
        "The 'gbProcessor' has no valid parent 'gbContext'.");

    // - In CGB Mode:
    //   - Bits 1-6 are unused; write as `1`.
    //   - Bit 7 is read-only; write original value.
    //   - Bits 0 is writable.
    // - Otherwise:
    //   - `KEY1` is not accessible; ignore writes.
    if (processor->isCGBMode == true)
    {
        processor->key1.raw =
            0b01111110 |                            // Bits 1-6 unused, write as `1`
            (processor->key1.raw & 0b10000000) |    // Bit 7 read-only, write original value
            (value & 0b00000001);                   // Bit 0 writable
    }
    else
    {
        processor->key1.raw = 0xFF;
    }

    if (outActual != nullptr)
    {
        *outActual = processor->key1.raw;
    }

    return true;
}
