/**
 * @file    GB/Instructions.c
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-18
 * 
 * @brief   Contains definitions for the Game Boy Emulator CPU's instruction 
 *          execution functions.
 */

/* Private Includes ***********************************************************/

#include <GB/Instructions.h>

/* Private Function Declarations - Helper Functions ***************************/

static bool gbCheckCondition (gbProcessor* processor, gbCondition condition);
static bool gbPerformByteAddition (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand, bool withCarry);
static bool gbPerformByteSubtraction (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand, bool withCarry, bool storeResult);
static bool gbPerformByteAND (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand);
static bool gbPerformByteOR (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand);
static bool gbPerformByteXOR (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand);

/* Private Function Definitions - Helper Functions ****************************/

bool gbCheckCondition (gbProcessor* processor, gbCondition condition)
{
    gbAssert(processor != nullptr);
    gbAssert(condition <= GB_CT_NC);

    switch (condition)
    {
        case GB_CT_NONE:
            return true;
        
        case GB_CT_Z:
        case GB_CT_NZ:
            bool zeroFlag = false;
            gbReadFlag(processor, GB_PF_Z, &zeroFlag);
            return (condition == GB_CT_Z) ? zeroFlag : !zeroFlag;

        case GB_CT_C:
        case GB_CT_NC:
            bool carryFlag = false;
            gbReadFlag(processor, GB_PF_C, &carryFlag);
            return (condition == GB_CT_C) ? carryFlag : !carryFlag;

    }

    /*
     * All enumeration values are handled above; silence compiler warnings about
     * control reaching the end of a non-void function by returning `false` here.
     */
    return false;
}

bool gbPerformByteAddition (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand, bool withCarry)
{
    gbAssert(processor != nullptr);

    // - Check if the Carry Flag is to be included in the addition.
    bool carryFlag = false;
    uint8_t carryValue = 0;
    if (withCarry == true)
    {
        if (gbReadFlag(processor, GB_PF_C, &carryFlag) == false)
        {
            return false;
        }

        carryValue = carryFlag ? 1 : 0;
    }

    // - Evaluate `lefthand + righthand + carryValue`, check for carry and store
    //   the result.
    // - Evaluate `(lefthand & 0x0F) + (righthand & 0x0F) + carryValue` and check
    //   for half-carry.
    uint16_t result = lefthand + righthand + carryValue;
    uint8_t halfCarryCheck = (lefthand & 0x0F) + (righthand & 0x0F) + carryValue;

    // - Write back result and update flags.
    if (
        gbWriteRegisterByte(processor, GB_RT_A, result & 0xFF) == false ||
        gbWriteFlag(processor, GB_PF_Z, ((result & 0xFF) == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, (halfCarryCheck > 0x0F)) == false ||
        gbWriteFlag(processor, GB_PF_C, (result > 0xFF)) == false
    )
    {
        return false;
    }
    
    return true;
}

bool gbPerformByteSubtraction (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand, bool withCarry, bool storeResult)
{
    gbAssert(processor != nullptr);

    // - Check if the Carry Flag is to be included in the subtraction.
    bool carryFlag = false;
    uint8_t carryValue = 0;
    if (withCarry == true)
    {
        if (gbReadFlag(processor, GB_PF_C, &carryFlag) == false)
        {
            return false;
        }

        carryValue = carryFlag ? 1 : 0;
    }

    // - Evaluate `lefthand - righthand - carryValue`, check for borrow and store
    //   the result.
    // - Evaluate `(lefthand & 0x0F) - (righthand & 0x0F) - carryValue` and check
    //   for half-borrow.
    int16_t result = lefthand - righthand - carryValue;
    int8_t halfBorrowCheck = (lefthand & 0x0F) - (righthand & 0x0F) - carryValue;

    // - Write back result and update flags.
    if (
        (
            storeResult == true &&
            gbWriteRegisterByte(processor, GB_RT_A, result & 0xFF) == false
        ) ||
        gbWriteFlag(processor, GB_PF_Z, ((result & 0xFF) == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, true) == false ||
        gbWriteFlag(processor, GB_PF_H, (halfBorrowCheck < 0)) == false ||
        gbWriteFlag(processor, GB_PF_C, (result < 0)) == false
    )
    {
        return false;
    }
    
    return true;
}

bool gbPerformByteAND (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand)
{
    gbAssert(processor != nullptr);

    uint8_t result = lefthand & righthand;

    if (
        gbWriteRegisterByte(processor, GB_RT_A, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, true) == false ||
        gbWriteFlag(processor, GB_PF_C, false) == false
    )
    {
        return false;
    }

    return true;
}

bool gbPerformByteOR (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand)
{
    gbAssert(processor != nullptr);

    uint8_t result = lefthand | righthand;

    if (
        gbWriteRegisterByte(processor, GB_RT_A, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, false) == false
    )
    {
        return false;
    }

    return true;
}

bool gbPerformByteXOR (gbProcessor* processor, uint8_t lefthand,
    uint8_t righthand)
{
    gbAssert(processor != nullptr);

    uint8_t result = lefthand ^ righthand;

    if (
        gbWriteRegisterByte(processor, GB_RT_A, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, false) == false
    )
    {
        return false;
    }

    return true;
}

/* Public Function Definitions - CPU Control Instructions *********************/

bool gbExecuteNOP (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'NOP': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'NOP': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Nothing to be done.

    return true;
}

bool gbExecuteSTOP (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'STOP': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - No additional cycles for unused immediate byte
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'STOP': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Enter `STOP` State
    if (gbEnterStopState(processor) == false)
    {
        gbLogError("'STOP': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteHALT (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'HALT': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'HALT': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Enter `HALT` State
    if (gbEnterHaltState(processor) == false)
    {
        gbLogError("'HALT': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteDI (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'DI': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'DI': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Disable Interrupts
    if (gbDisableInterrupts(processor) == false)
    {
        gbLogError("'DI': Failed to disable interrupts.");
        return false;
    }

    return true;
}

bool gbExecuteEI (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'EI': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'EI': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Enable Interrupts (after next instruction)
    if (gbEnableInterrupts(processor, false) == false)
    {
        gbLogError("'EI': Failed to prime interrupt master enable.");
        return false;
    }

    return true;
}

/* Public Function Definitions - Branching Instructions ***********************/

bool gbExecuteJR_S8 (gbProcessor* processor, gbCondition condition,
    int8_t offset, bool* outTaken)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'JR': No processor available.");

    // - Validate Inputs
    //   - Branching Condition
    if (condition > GB_CT_NC)
    {
        gbLogError("'JR': Invalid branching condition enum '%d'.", condition);
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'JR': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Check branching condition. If not met, early out.
    //   - Read value of `PC` register.
    //   - Write value plus signed offset back to `PC` register.
    //   - Consume 1 M-Cycle for jump.
    bool taken = gbCheckCondition(processor, condition);
    if (outTaken != nullptr) { *outTaken = taken; }
    if (taken == true)
    {
        uint16_t pc = 0;
        if (
            gbReadRegisterWord(processor, GB_RT_PC, &pc) == false ||
            gbWriteRegisterWord(processor, GB_RT_PC, (pc + offset)) == false ||
            gbConsumeMachineCycles(processor, 1) == false
        )
        {
            gbLogError("'JR': Failed to execute instruction.");
            return false;
        }
    }

    return true;
}

bool gbExecuteRET (gbProcessor* processor, gbCondition condition,
    bool* outTaken)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RET': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RET': No parent context available.");

    // - Validate Inputs
    //   - Branching Condition
    if (condition > GB_CT_NC)
    {
        gbLogError("'RET': Invalid branching condition enum '%d'.", condition);
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'RET': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Check branching condition.
    //   - Consume 1 M-cycle if branch is conditional.
    //   - If branching condition is not met, early out.
    //   - Read `SP` register.
    //   - Pop return address low byte from stack, then increment `SP`.
    //   - Consume 1 M-cycle for memory read.
    //   - Pop return address high byte from stack, then increment `SP`.
    //   - Consume 1 M-cycle for memory read.
    //   - Write return address to `PC` register.
    //   - Consume 1 M-cycle for jump.
    bool taken = gbCheckCondition(processor, condition);
    if (outTaken != nullptr) { *outTaken = taken; }
    
    if (
        condition != GB_CT_NONE &&
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'RET': Failed to consume conditional cycle.");
        return false;
    }

    if (taken == true)
    {
        uint16_t sp = 0;
        uint8_t lowByte = 0;
        uint8_t highByte = 0;
        if (
            gbReadRegisterWord(processor, GB_RT_SP, &sp) == false ||
            gbReadByte(context, sp++, &lowByte, nullptr) == false ||
            gbConsumeMachineCycles(processor, 1) == false ||
            gbReadByte(context, sp++, &highByte, nullptr) == false ||
            gbConsumeMachineCycles(processor, 1) == false ||
            gbWriteRegisterWord(processor, GB_RT_PC,
                ((uint16_t) (highByte << 8) | lowByte)) == false ||
            gbWriteRegisterWord(processor, GB_RT_SP, sp) == false ||
            gbConsumeMachineCycles(processor, 1) == false
        )
        {
            gbLogError("'RET': Failed to execute instruction.");
            return false;
        }
    }

    return true;
}

bool gbExecuteJP_A16 (gbProcessor* processor, gbCondition condition,
    uint16_t address, bool* outTaken)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'JP': No processor available.");

    // - Validate Inputs
    //   - Branching Condition
    if (condition > GB_CT_NC)
    {
        gbLogError("'JP': Invalid branching condition enum '%d'.", condition);
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 2 M-Cycles for Immediate Word Fetch
    if (gbConsumeFetchCycles(processor, 3) == false)
    {
        gbLogError("'JP': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Check branching condition. If not met, early out.
    //   - Write immediate address to `PC` register.
    //   - Consume 1 M-Cycle for jump.
    bool taken = gbCheckCondition(processor, condition);
    if (outTaken != nullptr) { *outTaken = taken; }
    if (taken == true)
    {
        if (
            gbWriteRegisterWord(processor, GB_RT_PC, address) == false ||
            gbConsumeMachineCycles(processor, 1) == false
        )
        {
            gbLogError("'JP': Failed to execute instruction.");
            return false;
        }
    }

    return true;
}

bool gbExecuteCALL_A16 (gbProcessor* processor, gbCondition condition,
    uint16_t address, bool* outTaken)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'CALL': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'CALL': No parent context available.");

    // - Validate Inputs
    //   - Branching Condition
    if (condition > GB_CT_NC)
    {
        gbLogError("'CALL': Invalid branching condition enum '%d'.", condition);
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 2 M-Cycles for Immediate Word Fetch
    if (gbConsumeFetchCycles(processor, 3) == false)
    {
        gbLogError("'CALL': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Check branching condition. If not met, early out.
    //   - Read `PC` and `SP` registers.
    //   - Push `PC` high byte onto stack, decrement `SP`.
    //   - Consume 1 M-cycle for memory write.
    //   - Push `PC` low byte onto stack, decrement `SP`.
    //   - Consume 1 M-cycle for memory write.
    //   - Write immediate address to `PC` register.
    //   - Consume 1 M-cycle for jump.
    bool taken = gbCheckCondition(processor, condition);
    if (outTaken != nullptr) { *outTaken = taken; }
    if (taken == true)
    {
        uint16_t pc = 0;
        uint16_t sp = 0;
        if (
            gbReadRegisterWord(processor, GB_RT_PC, &pc) == false ||
            gbReadRegisterWord(processor, GB_RT_SP, &sp) == false ||
            gbWriteByte(context, --sp, ((pc >> 8) & 0xFF), nullptr, nullptr) == false ||
            gbConsumeMachineCycles(processor, 1) == false ||
            gbWriteByte(context, --sp, (pc & 0xFF), nullptr, nullptr) == false ||
            gbConsumeMachineCycles(processor, 1) == false ||
            gbWriteRegisterWord(processor, GB_RT_PC, address) == false ||
            gbWriteRegisterWord(processor, GB_RT_SP, sp) == false ||
            gbConsumeMachineCycles(processor, 1) == false
        )
        {
            gbLogError("'CALL': Failed to execute instruction.");
            return false;
        }
    }

    return true;
}

bool gbExecuteRST (gbProcessor* processor, gbRestartVector vector, bool* outTaken)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RST': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RST': No parent context available.");

    // - Validate Inputs
    //   - Restart Vector
    if (vector > GB_RV_38H || (vector & 0b111) != 0)
    {
        gbLogError("'RST': Invalid restart vector enum '%d'.", vector);
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'RST': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Branch is always taken.
    //   - Read `PC` and `SP` registers.
    //   - Push `PC` high byte onto stack, decrement `SP`.
    //   - Consume 1 M-cycle for memory write.
    //   - Push `PC` low byte onto stack, decrement `SP`.
    //   - Consume 1 M-cycle for memory write.
    //   - Write restart vector address to `PC` register.
    //   - Consume 1 M-cycle for jump.
    if (outTaken != nullptr) { *outTaken = true; }
    
    uint16_t pc = 0;
    uint16_t sp = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_PC, &pc) == false ||
        gbReadRegisterWord(processor, GB_RT_SP, &sp) == false ||
        gbWriteByte(context, --sp, ((pc >> 8) & 0xFF), nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteByte(context, --sp, (pc & 0xFF), nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterWord(processor, GB_RT_PC, (uint16_t) vector) == false ||
        gbWriteRegisterWord(processor, GB_RT_SP, sp) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'RST': Failed to execute instruction.");
        return false;
    }
    
    return true;
}

bool gbExecuteRETI (gbProcessor* processor, bool* outTaken)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RETI': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RETI': No parent context available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'RETI': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Branch is always taken.
    //   - Read `SP` register.
    //   - Pop return address low byte from stack, then increment `SP`.
    //   - Consume 1 M-cycle for memory read.
    //   - Pop return address high byte from stack, then increment `SP`.
    //   - Consume 1 M-cycle for memory read.
    //   - Write return address to `PC` register.
    //   - Consume 1 M-cycle for jump.
    //   - Enable Interrupts.
    if (outTaken != nullptr) { *outTaken = true; }

    uint16_t sp = 0;
    uint8_t lowByte = 0;
    uint8_t highByte = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_SP, &sp) == false ||
        gbReadByte(context, sp++, &lowByte, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbReadByte(context, sp++, &highByte, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterWord(processor, GB_RT_PC,
            ((uint16_t) (highByte << 8) | lowByte)) == false ||
        gbWriteRegisterWord(processor, GB_RT_SP, sp) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbEnableInterrupts(processor, true) == false
    )
    {
        gbLogError("'RETI': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteJP_HL (gbProcessor* processor, uint16_t* outAddress, bool* outTaken)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'JP HL': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'JP HL': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Branch is always taken.
    //   - Read `HL` register.
    //   - Write `HL` value to `PC` register.
    //   - Consume 1 M-Cycle for jump.
    //   - Output address if requested.
    if (outTaken != nullptr) { *outTaken = true; }

    uint16_t address = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_HL, &address) == false ||
        gbWriteRegisterWord(processor, GB_RT_PC, address) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'JP HL': Failed to execute instruction.");
        return false;
    }

    if (outAddress != nullptr) { *outAddress = address; }

    return true;
}

/* Public Function Definitions - 8-Bit Load Instructions **********************/

bool gbExecuteLD_pR16_R8 (gbProcessor* processor, gbRegisterType dest, 
    gbRegisterType src, int8_t adjust)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD (R16), R8': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LD (R16), R8': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    //   - Source Register (`A` through `L`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'LD (R16), R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }
    else if (src > GB_RT_L)
    {
        gbLogError("'LD (R16), R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'LD (R16), R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from source register.
    //   - Read address from destination register.
    //   - Write value to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - If `adjust < 0`, decrement destination register value by 1.
    //   - If `adjust > 0`, increment destination register value by 1.
    uint8_t value = 0;
    uint16_t address = 0;
    if (
        gbReadRegisterByte(processor, src, &value) == false ||
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbWriteByte(context, address, value, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        (
            (adjust < 0 && gbWriteRegisterWord(processor, dest, address - 1) == false) ||
            (adjust > 0 && gbWriteRegisterWord(processor, dest, address + 1) == false)
        )
    )
    {
        gbLogError("'LD (R16), R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_R8_D8 (gbProcessor* processor, gbRegisterType dest, uint8_t src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD R8, D8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'LD R8, D8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'LD R8, D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Write immediate value to destination register.
    if (gbWriteRegisterByte(processor, dest, src) == false)
    {
        gbLogError("'LD R8, D8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_R8_pR16 (gbProcessor* processor, gbRegisterType dest, 
    gbRegisterType src, int8_t adjust)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD R8, (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LD R8, (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (dest > GB_RT_L)
    {
        gbLogError("'LD R8, (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }
    else if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'LD R8, (R16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'LD R8, (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read address from source register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Write value to destination register.
    //   - If `adjust < 0`, decrement source register value by 1.
    //   - If `adjust > 0`, increment source register value by 1.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterByte(processor, dest, value) == false ||
        (
            (adjust < 0 && gbWriteRegisterWord(processor, src, address - 1) == false) ||
            (adjust > 0 && gbWriteRegisterWord(processor, src, address + 1) == false)
        )
    )
    {
        gbLogError("'LD R8, (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_pR16_D8 (gbProcessor* processor, gbRegisterType dest, uint8_t src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD (R16), D8': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LD (R16), D8': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'LD (R16), D8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'LD (R16), D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read address from destination register.
    //   - Write immediate value to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    uint16_t address = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbWriteByte(context, address, src, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'LD (R16), D8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_R8_R8 (gbProcessor* processor, gbRegisterType dest, 
    gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD R8, R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    //   - Source Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'LD R8, R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }
    else if (src > GB_RT_L)
    {
        gbLogError("'LD R8, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'LD R8, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Read value from source register.
    //   - Write value to destination register.
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, src, &value) == false ||
        gbWriteRegisterByte(processor, dest, value) == false
    )
    {
        gbLogError("'LD R8, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLDH_pA8_R8 (gbProcessor* processor, uint8_t offset, gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LDH (A8), R8': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LDH (A8), R8': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`) 
    if (src > GB_RT_L)
    {
        gbLogError("'LDH (A8), R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'LDH (A8), R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from source register.
    //   - Calculate absolute address: `$FF00 + offset`.
    //   - Write value to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    uint8_t value = 0;
    uint16_t address = 0xFF00 + offset;
    if (
        gbReadRegisterByte(processor, src, &value) == false ||
        gbWriteByte(context, address, value, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'LDH (A8), R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLDH_pC_R8 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LDH (C), R8': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LDH (C), R8': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`) 
    if (src > GB_RT_L)
    {
        gbLogError("'LDH (C), R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'LDH (C), R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from source register.
    //   - Read offset from `C` register.
    //   - Calculate absolute address: `$FF00 + offset`.
    //   - Write value to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    uint8_t value = 0;
    uint8_t offset = 0;
    uint16_t address = 0;
    if (
        gbReadRegisterByte(processor, src, &value) == false ||
        gbReadRegisterByte(processor, GB_RT_C, &offset) == false ||
        gbWriteByte(context, (0xFF00 + offset), value, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'LDH (C), R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_pA16_R8 (gbProcessor* processor, uint16_t address, 
    gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD (A16), R8': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LD (A16), R8': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`) 
    if (src > GB_RT_L)
    {
        gbLogError("'LD (A16), R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 2 M-Cycles for Immediate Word Fetch
    if (gbConsumeFetchCycles(processor, 3) == false)
    {
        gbLogError("'LD (A16), R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from source register.
    //   - Write value to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, src, &value) == false ||
        gbWriteByte(context, address, value, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'LD (A16), R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLDH_R8_pA8 (gbProcessor* processor, gbRegisterType dest, 
    uint8_t offset)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LDH R8, (A8)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LDH R8, (A8)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'LDH R8, (A8)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'LDH R8, (A8)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Calculate absolute address: `$FF00 + offset`.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Write value to destination register.
    uint8_t value = 0;
    uint16_t address = 0xFF00 + offset;     
    if (
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterByte(processor, dest, value) == false
    )
    {
        gbLogError("'LDH R8, (A8)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLDH_R8_pC (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LDH R8, (C)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LDH R8, (C)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'LDH R8, (C)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'LDH R8, (C)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read offset from `C` register.
    //   - Calculate absolute address: `$FF00 + offset`.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Write value to destination register.
    uint8_t value = 0;
    uint8_t offset = 0;
    uint16_t address = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_C, &offset) == false ||
        gbReadByte(context, (0xFF00 + offset), &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterByte(processor, dest, value) == false
    )
    {
        gbLogError("'LDH R8, (C)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_R8_pA16 (gbProcessor* processor, gbRegisterType dest, 
    uint16_t address)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD R8, (A16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LD R8, (A16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'LD R8, (A16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 2 M-Cycles for Immediate Word Fetch
    if (gbConsumeFetchCycles(processor, 3) == false)
    {
        gbLogError("'LD R8, (A16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Write value to destination register.
    uint8_t value = 0;
    if (
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterByte(processor, dest, value) == false
    )
    {
        gbLogError("'LD R8, (A16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

/* Public Function Definitions - 16-Bit Load Instructions *********************/

bool gbExecuteLD_R16_D16 (gbProcessor* processor, gbRegisterType dest, uint16_t src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD R16, D16': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`, `AF`)
    if (dest < GB_RT_AF || dest > GB_RT_SP)
    {
        gbLogError("'LD R16, D16': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 2 M-Cycles for Immediate Word Fetch
    if (gbConsumeFetchCycles(processor, 3) == false)
    {
        gbLogError("'LD R16, D16': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Write immediate value to destination register.
    if (gbWriteRegisterWord(processor, dest, src) == false)
    {
        gbLogError("'LD R16, D16': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_pA16_SP (gbProcessor* processor, uint16_t address)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD (A16), SP': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'LD (A16), SP': No parent context available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 2 M-Cycles for Immediate Word Fetch
    if (gbConsumeFetchCycles(processor, 3) == false)
    {
        gbLogError("'LD (A16), SP': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read `SP` register.
    //   - Write low byte of `SP` to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Write high byte of `SP` to memory at address + 1.
    //   - Consume 1 M-Cycle for memory write.
    uint16_t sp = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_SP, &sp) == false ||
        gbWriteByte(context, address, (sp & 0xFF), nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteByte(context, address + 1, ((sp >> 8) & 0xFF), nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'LD (A16), SP': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecutePOP_R16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'POP R16': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'POP R16': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `AF`)
    if (dest < GB_RT_AF || dest > GB_RT_HL)
    {
        gbLogError("'POP R16': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'POP R16': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read `SP` register.
    //   - Pop low byte from stack, then increment `SP`.
    //   - Consume 1 M-cycle for memory read.
    //   - Pop high byte from stack, then increment `SP`.
    //   - Consume 1 M-cycle for memory read.
    //   - Write combined value to destination register.
    uint16_t sp = 0;
    uint8_t lowByte = 0;
    uint8_t highByte = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_SP, &sp) == false ||
        gbReadByte(context, sp++, &lowByte, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbReadByte(context, sp++, &highByte, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterWord(processor, dest,
            ((uint16_t) (highByte << 8) | lowByte)) == false ||
        gbWriteRegisterWord(processor, GB_RT_SP, sp) == false
    )
    {
        gbLogError("'POP R16': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecutePUSH_R16 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'PUSH R16': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'PUSH R16': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `AF`)
    if (src < GB_RT_AF || src > GB_RT_HL)
    {
        gbLogError("'PUSH R16': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'PUSH R16': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read `SP` register.
    //   - Decrement `SP`, then write high byte of source register to stack.
    //   - Consume 1 M-cycle for memory write.
    //   - Decrement `SP`, then write low byte of source register to stack.
    //   - Consume 1 M-cycle for memory write.
    //   - Write updated `SP` value back to register.
    //   - Consume 1 M-Cycle for instruction execution.
    uint16_t sp = 0;
    uint16_t value = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_SP, &sp) == false ||
        gbReadRegisterWord(processor, src, &value) == false ||
        gbWriteByte(context, --sp, ((value >> 8) & 0xFF), nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteByte(context, --sp, (value & 0xFF), nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteRegisterWord(processor, GB_RT_SP, sp) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'PUSH R16': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_R16_SPpS8 (gbProcessor* processor, gbRegisterType dest, 
    int8_t offset)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD R16, SP+S8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`)
    if (dest < GB_RT_BC || dest > GB_RT_HL)
    {
        gbLogError("'LD R16, SP+S8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Signed Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'LD R16, SP+S8': Failed to consume fetch cycles.");
        return false;   
    }

    // - Execute Instruction (in the following order):
    //   - Read `SP` register.
    //   - Evaluate `SP + offset`; save the result.
    //   - Evaluate `(SP & 0xFF) + (offset & 0xFF)` to check for carry.
    //   - Evaluate `(SP & 0x0F) + (offset & 0x0F)` to check for half-carry.
    //   - Write result to destination register.
    //   - Update flags as appropriate.
    //   - Consume 1 M-Cycle for instruction execution.
    uint16_t sp = 0;
    if (gbReadRegisterWord(processor, GB_RT_SP, &sp) == false)
    {
        gbLogError("'LD R16, SP+S8': Failed to execute instruction.");
        return false;
    }

    uint32_t result = sp + offset;
    uint16_t carryCheck = (sp & 0xFF) + (offset & 0xFF);
    uint8_t halfCarryCheck = (sp & 0x0F) + (offset & 0x0F);

    if (
        gbWriteRegisterWord(processor, dest, result & 0xFFFF) == false ||
        gbWriteFlag(processor, GB_PF_Z, false) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, (halfCarryCheck > 0x0F)) == false ||
        gbWriteFlag(processor, GB_PF_C, (carryCheck > 0xFF)) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'LD R16, SP+S8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteLD_SP_R16 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'LD SP, R16': No processor available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`)
    if (src < GB_RT_BC || src > GB_RT_HL)
    {
        gbLogError("'LD SP, R16': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'LD SP, R16': Failed to consume fetch cycles.");
        return false;   
    }

    // - Execute Instruction:
    //   - Read value from source register.
    //   - Write value to `SP` register.
    //   - Consume 1 M-Cycle for instruction execution.
    uint16_t value = 0;
    if (
        gbReadRegisterWord(processor, src, &value) == false ||
        gbWriteRegisterWord(processor, GB_RT_SP, value) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'LD SP, R16': Failed to execute instruction.");
        return false;
    }

    return true;
}

/* Public Function Definitions - 8-Bit Arithmetic/Bitwise Logic Instructions **/

bool gbExecuteINC_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'INC R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'INC R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'INC R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from destination register.
    //   - Increment value by 1.
    //   - Write value back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'INC R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value + 1;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, (value & 0xF) == 0xF) == false
    )
    {
        gbLogError("'INC R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteDEC_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'DEC R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'DEC R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'DEC R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from destination register.
    //   - Decrement value by 1.
    //   - Write value back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'DEC R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value - 1;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, true) == false ||
        gbWriteFlag(processor, GB_PF_H, (value & 0x0F) == 0x00) == false
    )
    {
        gbLogError("'DEC R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteINC_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'INC (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'INC (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'INC (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'INC (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Increment value by 1.
    //   - Write value back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'INC (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value + 1;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, (value & 0x0F) == 0x0F) == false
    )
    {
        gbLogError("'INC (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteDEC_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'DEC (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'DEC (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'DEC (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'DEC (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Decrement value by 1.
    //   - Write value back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'DEC (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value - 1;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, true) == false ||
        gbWriteFlag(processor, GB_PF_H, (value & 0x0F) == 0x00) == false
    )
    {
        gbLogError("'DEC (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteDAA (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'DAA': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'DAA': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Get value of `A` accumulator register.
    //   - Read subtraction, half-carry, and carry flags.
    //   - Decimal-adjust value as appropriate (BCD has all digits in range 0-9).
    //   - Write adjusted value back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    bool subtractFlag = false;
    bool halfCarryFlag = false;
    bool carryFlag = false;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadFlag(processor, GB_PF_N, &subtractFlag) == false ||
        gbReadFlag(processor, GB_PF_H, &halfCarryFlag) == false ||
        gbReadFlag(processor, GB_PF_C, &carryFlag) == false
    )
    {
        gbLogError("'DAA': Failed to execute instruction.");
        return false;
    }

    uint8_t correction = 0;
    if (halfCarryFlag || (!subtractFlag && (a & 0x0F) > 0x09))
    {
        correction |= 0x06;
    }

    if (carryFlag || (!subtractFlag && a > 0x99))
    {
        correction |= 0x60;
        carryFlag = true;
    }

    if (subtractFlag) { a -= correction; }
    else { a += correction; }

    if (
        gbWriteRegisterByte(processor, GB_RT_A, a) == false ||
        gbWriteFlag(processor, GB_PF_Z, (a == 0)) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carryFlag) == false
    )
    {
        gbLogError("'DAA': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteCPL (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'CPL': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'CPL': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Read value from `A` register.
    //   - Complement value (bitwise NOT).
    //   - Write complemented value back to `A` register.
    //   - Set `N` and `H` flags.
    uint8_t a = 0;
    if (gbReadRegisterByte(processor, GB_RT_A, &a) == false)
    {
        gbLogError("'CPL': Failed to execute instruction.");
        return false;
    }

    if (
        gbWriteRegisterByte(processor, GB_RT_A, ~a) == false ||
        gbWriteFlag(processor, GB_PF_N, true) == false ||
        gbWriteFlag(processor, GB_PF_H, true) == false
    )
    {
        gbLogError("'CPL': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSCF (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SCF': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'SCF': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Set `C` flag.
    //   - Clear `N` and `H` flags.
    if (
        gbWriteFlag(processor, GB_PF_C, true) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false
    )
    {
        gbLogError("'SCF': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteCCF (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'CCF': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'CCF': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction:
    //   - Complement `C` flag.
    //   - Clear `N` and `H` flags.
    bool carryFlag = false;
    if (
        gbReadFlag(processor, GB_PF_C, &carryFlag) == false ||
        gbWriteFlag(processor, GB_PF_C, !carryFlag) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false
    )
    {
        gbLogError("'CCF': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteADD_A_R8 (gbProcessor* processor, gbRegisterType src, bool withCarry)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'ADD A, R8': No processor available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`)
    if (src > GB_RT_L)
    {
        gbLogError("'ADD A, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'ADD A, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Read value from source register.
    //   - If `withCarry` is true, read `C` flag and include it in the addition.
    //   - Evaluate `A + src (+ carry)`; save the result, also check for carry.
    //   - Evaluate `(A & 0x0F) + (src & 0x0F) (+ carry)` to check for half-carry.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterByte(processor, src, &value) == false ||
        gbPerformByteAddition(processor, a, value, withCarry) == false
    )
    {
        gbLogError("'ADD A, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteADD_A_pR16 (gbProcessor* processor, gbRegisterType src, bool withCarry)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'ADD A, (pR16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'ADD A, (pR16)': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'ADD A, (pR16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'ADD A, (pR16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Read address from source register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - If `withCarry` is true, read `C` flag and include it in the addition.
    //   - Evaluate `A + src (+ carry)`; save the result, also check for carry.
    //   - Evaluate `(A & 0x0F) + (src & 0x0F) (+ carry)` to check for half-carry.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbPerformByteAddition(processor, a, value, withCarry) == false
    )
    {
        gbLogError("'ADD A, (pR16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteADD_A_D8 (gbProcessor* processor, uint8_t src, bool withCarry)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'ADD A, D8': No processor available.");

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'ADD A, D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - If `withCarry` is true, read `C` flag and include it in the addition.
    //   - Evaluate `A + src (+ carry)`; save the result, also check for carry.
    //   - Evaluate `(A & 0x0F) + (src & 0x0F) (+ carry)` to check for half-carry.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbPerformByteAddition(processor, a, src, withCarry) == false
    )
    {
        gbLogError("'ADD A, D8': Failed to execute instruction.");
        return false;
    }
    
    return true;
}

bool gbExecuteSUB_A_R8 (gbProcessor* processor, gbRegisterType src, bool withCarry)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SUB A, R8': No processor available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`)
    if (src > GB_RT_L)
    {
        gbLogError("'SUB A, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'SUB A, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Read value from source register.
    //   - If `withCarry` is true, read `C` flag and include it in the subtraction.
    //   - Evaluate `A - src (- carry)`; save the result, also check for carry.
    //   - Evaluate `(A & 0x0F) - (src & 0x0F) (- carry)` to check for half-carry.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterByte(processor, src, &value) == false ||
        gbPerformByteSubtraction(processor, a, value, withCarry, true) == false
    )
    {
        gbLogError("'SUB A, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSUB_A_pR16 (gbProcessor* processor, gbRegisterType src, bool withCarry)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SUB A, (pR16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'SUB A, (pR16)': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'SUB A, (pR16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'SUB A, (pR16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Read address from source register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - If `withCarry` is true, read `C` flag and include it in the subtraction.
    //   - Evaluate `A - src (- carry)`; save the result, also check for carry.
    //   - Evaluate `(A & 0x0F) - (src & 0x0F) (- carry)` to check for half-carry.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbPerformByteSubtraction(processor, a, value, withCarry, true) == false
    )
    {
        gbLogError("'SUB A, (pR16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSUB_A_D8 (gbProcessor* processor, uint8_t src, bool withCarry)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SUB A, D8': No processor available.");

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'SUB A, D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Use immediate value `src`.
    //   - If `withCarry` is true, read `C` flag and include it in the subtraction.
    //   - Evaluate `A - src (- carry)`; save the result, also check for carry.
    //   - Evaluate `(A & 0x0F) - (src & 0x0F) (- carry)` to check for half-carry.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbPerformByteSubtraction(processor, a, src, withCarry, true) == false
    )
    {
        gbLogError("'SUB A, D8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteAND_A_R8 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'AND A, R8': No processor available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`)
    if (src > GB_RT_L)
    {
        gbLogError("'AND A, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'AND A, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterByte(processor, src, &value) == false ||
        gbPerformByteAND(processor, a, value) == false
    )
    {
        gbLogError("'AND A, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteAND_A_pR16 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'AND A, (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'AND A, (R16)': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'AND A, (R16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'AND A, (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbPerformByteAND(processor, a, value) == false
    )
    {
        gbLogError("'AND A, (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteAND_A_D8 (gbProcessor* processor, uint8_t src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'AND A, D8': No processor available.");

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'AND A, D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbPerformByteAND(processor, a, src) == false
    )
    {
        gbLogError("'AND A, D8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteXOR_A_R8 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'XOR A, R8': No processor available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`)
    if (src > GB_RT_L)
    {
        gbLogError("'XOR A, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'XOR A, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterByte(processor, src, &value) == false ||
        gbPerformByteXOR(processor, a, value) == false
    )
    {
        gbLogError("'XOR A, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteXOR_A_pR16 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'XOR A, (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'XOR A, (R16)': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'XOR A, (R16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'XOR A, (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbPerformByteXOR(processor, a, value) == false
    )
    {
        gbLogError("'XOR A, (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteXOR_A_D8 (gbProcessor* processor, uint8_t src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'XOR A, D8': No processor available.");

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'XOR A, D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbPerformByteXOR(processor, a, src) == false
    )
    {
        gbLogError("'XOR A, D8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteOR_A_R8 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'OR A, R8': No processor available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`)
    if (src > GB_RT_L)
    {
        gbLogError("'OR A, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'OR A, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterByte(processor, src, &value) == false ||
        gbPerformByteOR(processor, a, value) == false
    )
    {
        gbLogError("'OR A, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteOR_A_pR16 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'OR A, (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'OR A, (R16)': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'OR A, (R16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'OR A, (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbPerformByteOR(processor, a, value) == false
    )
    {
        gbLogError("'OR A, (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteOR_A_D8 (gbProcessor* processor, uint8_t src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'OR A, D8': No processor available.");

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'OR A, D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    uint8_t a = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbPerformByteOR(processor, a, src) == false
    )
    {
        gbLogError("'OR A, D8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteCP_A_R8 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'CP A, R8': No processor available.");

    // - Validate Inputs
    //   - Source Register (`A` through `L`)
    if (src > GB_RT_L)
    {
        gbLogError("'CP A, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'CP A, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Read value from source register.
    //   - Evaluate `A - src` (without saving result).
    //   - Update flags as appropriate.
    uint8_t a = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterByte(processor, src, &value) == false ||
        gbPerformByteSubtraction(processor, a, value, false, false) == false
    )
    {
        gbLogError("'CP A, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteCP_A_pR16 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'CP A, (pR16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'CP A, (pR16)': No parent context available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (src < GB_RT_BC || src > GB_RT_SP)
    {        
        gbLogError("'CP A, (pR16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'CP A, (pR16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Read address from source register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Evaluate `A - src` (without saving result).
    //   - Update flags as appropriate.
    uint8_t a = 0;
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbPerformByteSubtraction(processor, a, value, false, false) == false
    )
    {
        gbLogError("'CP A, (pR16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteCP_A_D8 (gbProcessor* processor, uint8_t src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'CP A, D8': No processor available.");

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'CP A, D8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction (in the following order):
    //   - Read value from `A` register.
    //   - Use immediate value `src`.
    //   - Evaluate `A - src` (without saving result).
    //   - Update flags as appropriate.
    uint8_t a = 0;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbPerformByteSubtraction(processor, a, src, false, false) == false
    )
    {
        gbLogError("'CP A, D8': Failed to execute instruction.");
        return false;
    }

    return true;
}

/* Public Function Definitions - 16-Bit Arithmetic Instructions ***************/

bool gbExecuteINC_R16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'INC R16': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'INC R16': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'INC R16': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Increment value by 1.
    //   - Write result back to destination register.
    //   - Consume 1 M-Cycle for instruction execution.
    uint16_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &value) == false ||
        gbWriteRegisterWord(processor, dest, value + 1) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'INC R16': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteADD_HL_R16 (gbProcessor* processor, gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'ADD HL, R16': No processor available.");

    // - Validate Inputs
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'ADD HL, R16': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'ADD HL, R16': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from `HL` register.
    //   - Read value from source register.
    //   - Evaluate `HL + src`; save the result, also check for carry.
    //   - Evaluate `(HL & 0x0FFF) + (src & 0x0FFF)` to check for half-carry.
    //   - Write result back to `HL` register.
    //   - Update flags as appropriate.
    //   - Consume 1 M-Cycle for instruction execution.
    uint16_t hl = 0;
    uint16_t value = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_HL, &hl) == false ||
        gbReadRegisterWord(processor, src, &value) == false
    )
    {
        gbLogError("'ADD HL, R16': Failed to execute instruction.");
        return false;
    }

    uint32_t result = hl + value;
    uint16_t halfCarryCheck = (hl & 0x0FFF) + (value & 0x0FFF);
    if (
        gbWriteRegisterWord(processor, GB_RT_HL, (result & 0xFFFF)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, (halfCarryCheck > 0x0FFF)) == false ||
        gbWriteFlag(processor, GB_PF_C, (result > 0xFFFF)) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'ADD HL, R16': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteDEC_R16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'DEC R16': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP) 
    {
        gbLogError("'DEC R16': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'DEC R16': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Decrement value by 1.
    //   - Write result back to destination register.
    //   - Consume 1 M-Cycle for instruction execution.
    uint16_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &value) == false ||
        gbWriteRegisterWord(processor, dest, value - 1) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'DEC R16': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteADD_SP_S8 (gbProcessor* processor, int8_t src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'ADD SP, S8': No processor available.");

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    //   - 1 M-Cycle for Immediate Byte Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'ADD SP, S8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from `SP` register.
    //   - Evaluate `SP + src`; save the result.
    //   - Evaluate `(SP & 0xFF) + (src & 0xFF)` to check for carry.
    //   - Evaluate `(SP & 0x0F) + (src & 0x0F)` to check for half-carry.
    //   - Write result back to `SP` register.
    //   - Update flags as appropriate.
    //   - Consume 2 M-Cycles for instruction execution.
    uint16_t sp = 0;
    if (
        gbReadRegisterWord(processor, GB_RT_SP, &sp) == false
    )
    {
        gbLogError("'ADD SP, S8': Failed to execute instruction.");
        return false;
    }

    uint32_t result = sp + src;
    uint16_t halfCarryCheck = (sp & 0x0F) + (src & 0x0F);
    uint16_t carryCheck = (sp & 0xFF) + (src & 0xFF);
    if (
        gbWriteRegisterWord(processor, GB_RT_SP, (result & 0xFFFF)) == false ||
        gbWriteFlag(processor, GB_PF_Z, false) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, (halfCarryCheck > 0x0F)) == false ||
        gbWriteFlag(processor, GB_PF_C, (carryCheck > 0xFF)) == false ||
        gbConsumeMachineCycles(processor, 2) == false
    )
    {
        gbLogError("'ADD SP, S8': Failed to execute instruction.");
        return false;
    }

    return true;
}

/* Public Function Definitions - 8-Bit Rotate/Shift/Bit Instructions **********/

bool gbExecuteRLCA (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RLCA': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'RLCA': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from `A` register.
    //   - Rotate value left by one bit.
    //   - Write original bit 7 to bit 0.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    if (gbReadRegisterByte(processor, GB_RT_A, &a) == false)
    {
        gbLogError("'RLCA': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (a << 1) | (a >> 7);
    bool carry = (a & 0b10000000) != 0;
    if (
        gbWriteRegisterByte(processor, GB_RT_A, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, false) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'RLCA': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRLA (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RLA': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'RLA': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from `A` register.
    //   - Read carry flag.
    //   - Rotate value left by one bit.
    //   - Write original bit 7 to carry flag.
    //   - Write carry flag to bit 0.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    bool carry = false;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadFlag(processor, GB_PF_C, &carry) == false
    )
    {
        gbLogError("'RLA': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (a << 1) | (carry ? 1 : 0);
    bool newCarry = (a & 0b10000000) != 0;
    if (
        gbWriteRegisterByte(processor, GB_RT_A, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, false) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, newCarry) == false
    )
    {
        gbLogError("'RLA': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRRCA (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RRCA': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'RRCA': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from `A` register.
    //   - Rotate value right by one bit.
    //   - Write original bit 0 to bit 7.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    if (gbReadRegisterByte(processor, GB_RT_A, &a) == false)
    {
        gbLogError("'RRCA': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (a >> 1) | (a << 7);
    bool carry = (a & 0b00000001) != 0;
    if (
        gbWriteRegisterByte(processor, GB_RT_A, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, false) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'RRCA': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRRA (gbProcessor* processor)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RRA': No processor available.");

    // - Validate Inputs
    //   - None

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 1) == false)
    {
        gbLogError("'RRA': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from `A` register.
    //   - Read carry flag.
    //   - Rotate value right by one bit.
    //   - Write original bit 0 to carry flag.
    //   - Write carry flag to bit 7.
    //   - Write result back to `A` register.
    //   - Update flags as appropriate.
    uint8_t a = 0;
    bool carry = false;
    if (
        gbReadRegisterByte(processor, GB_RT_A, &a) == false ||
        gbReadFlag(processor, GB_PF_C, &carry) == false
    )
    {
        gbLogError("'RRA': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (a >> 1) | (carry ? 0b10000000 : 0);
    bool newCarry = (a & 0b00000001) != 0;
    if (
        gbWriteRegisterByte(processor, GB_RT_A, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, false) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, newCarry) == false
    )
    {
        gbLogError("'RRA': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRLC_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RLC R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'RLC R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RLC R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Rotate value left by one bit.
    //   - Write original bit 7 to bit 0.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'RLC R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value << 1) | (value >> 7);
    bool carry = (value & 0b10000000) != 0;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'RLC R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRLC_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RLC (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RLC (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'RLC (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RLC (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Rotate value left by one bit.
    //   - Write original bit 7 to bit 0.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'RLC (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value << 1) | (value >> 7);
    bool carry = (value & 0b10000000) != 0;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'RLC (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRRC_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RRC R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'RRC R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RRC R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Rotate value right by one bit.
    //   - Write original bit 0 to bit 7.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'RRC R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value >> 1) | (value << 7);
    bool carry = (value & 0b00000001) != 0;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'RRC R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRRC_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RRC (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RRC (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'RRC (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RRC (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Rotate value right by one bit.
    //   - Write original bit 0 to bit 7.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'RRC (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value >> 1) | (value << 7);
    bool carry = (value & 0b00000001) != 0;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'RRC (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRL_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RL R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'RL R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RL R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Read carry flag.
    //   - Rotate value left by one bit.
    //   - Write original bit 7 to carry flag.
    //   - Write carry flag to bit 0.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    bool carry = false;
    if (
        gbReadRegisterByte(processor, dest, &value) == false ||
        gbReadFlag(processor, GB_PF_C, &carry) == false
    )
    {
        gbLogError("'RL R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value << 1) | (carry ? 1 : 0);
    bool newCarry = (value & 0b10000000) != 0;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, newCarry) == false
    )
    {
        gbLogError("'RL R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRL_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RL (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RL (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'RL (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RL (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Read carry flag.
    //   - Rotate value left by one bit.
    //   - Write original bit 7 to carry flag.
    //   - Write carry flag to bit 0.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    bool carry = false;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbReadFlag(processor, GB_PF_C, &carry) == false
    )
    {
        gbLogError("'RL (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value << 1) | (carry ? 1 : 0);
    bool newCarry = (value & 0b10000000) != 0;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, newCarry) == false
    )
    {
        gbLogError("'RL (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRR_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RR R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'RR R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RR R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Read carry flag.
    //   - Rotate value right by one bit.
    //   - Write original bit 0 to carry flag.
    //   - Write carry flag to bit 7.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    bool carry = false;
    if (
        gbReadRegisterByte(processor, dest, &value) == false ||
        gbReadFlag(processor, GB_PF_C, &carry) == false
    )
    {
        gbLogError("'RR R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value >> 1) | (carry ? 0b10000000 : 0);
    bool newCarry = (value & 0b00000001) != 0;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, newCarry) == false
    )
    {
        gbLogError("'RR R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRR_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RR (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RR (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'RR (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RR (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Read carry flag.
    //   - Rotate value right by one bit.
    //   - Write original bit 0 to carry flag.
    //   - Write carry flag to bit 7.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    bool carry = false;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbReadFlag(processor, GB_PF_C, &carry) == false
    )
    {
        gbLogError("'RR (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value >> 1) | (carry ? 0b10000000 : 0);
    bool newCarry = (value & 0b00000001) != 0;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, newCarry) == false
    )
    {
        gbLogError("'RR (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSLA_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SLA R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'SLA R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SLA R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Shift value left by one bit.
    //   - Write 0 to bit 0.
    //   - Write original bit 7 to carry flag.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'SLA R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value << 1;
    bool carry = (value & 0b10000000) != 0;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'SLA R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSLA_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SLA (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'SLA (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'SLA (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SLA (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Shift value left by one bit.
    //   - Write 0 to bit 0.
    //   - Write original bit 7 to carry flag.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'SLA (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value << 1;
    bool carry = (value & 0b10000000) != 0;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'SLA (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSRA_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SRA R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'SRA R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SRA R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Shift value right by one bit.
    //   - Preserve bit 7 (arithmetic shift).
    //   - Write original bit 0 to carry flag.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'SRA R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value >> 1) | (value & 0b10000000);
    bool carry = (value & 0b00000001) != 0;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'SRA R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSRA_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SRA (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'SRA (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'SRA (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SRA (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Shift value right by one bit.
    //   - Preserve bit 7 (arithmetic shift).
    //   - Write original bit 0 to carry flag.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'SRA (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = (value >> 1) | (value & 0b10000000);
    bool carry = (value & 0b00000001) != 0;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'SRA (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSWAP_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SWAP R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'SWAP R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SWAP R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Swap upper and lower nibbles.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'SWAP R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, false) == false
    )
    {
        gbLogError("'SWAP R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSWAP_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SWAP (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'SWAP (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'SWAP (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SWAP (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Swap upper and lower nibbles.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'SWAP (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, false) == false
    )
    {
        gbLogError("'SWAP (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSRL_R8 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SRL R8': No processor available.");

    // - Validate Inputs
    //   - Destination Register (`A` through `L`)
    if (dest > GB_RT_L)
    {
        gbLogError("'SRL R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SRL R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Shift value right by one bit.
    //   - Write 0 to bit 7 (logical shift).
    //   - Write original bit 0 to carry flag.
    //   - Write result back to destination register.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'SRL R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value >> 1;
    bool carry = (value & 0b00000001) != 0;
    if (
        gbWriteRegisterByte(processor, dest, result) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'SRL R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSRL_pR16 (gbProcessor* processor, gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SRL (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'SRL (R16)': No parent context available.");

    // - Validate Inputs
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'SRL (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SRL (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Shift value right by one bit.
    //   - Write 0 to bit 7 (logical shift).
    //   - Write original bit 0 to carry flag.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'SRL (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value >> 1;
    bool carry = (value & 0b00000001) != 0;
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, (result == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, false) == false ||
        gbWriteFlag(processor, GB_PF_C, carry) == false
    )
    {
        gbLogError("'SRL (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteBIT_U3_R8 (gbProcessor* processor, uint8_t bit_index, 
    gbRegisterType src)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'BIT U3, R8': No processor available.");

    // - Validate Inputs
    //   - Bit Index (0-7)
    //   - Source Register (`A` through `L`)
    if (bit_index > 7)
    {
        gbLogError("'BIT U3, R8': Invalid bit index '%u'.", bit_index);
        return false;
    }
    else if (src > GB_RT_L)
    {
        gbLogError("'BIT U3, R8': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'BIT U3, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from source register.
    //   - Test specified bit.
    //   - Update flags as appropriate.
    uint8_t value = 0;
    if (
        gbReadRegisterByte(processor, src, &value) == false ||
        gbWriteFlag(processor, GB_PF_Z, ((value & (1 << bit_index)) == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, true) == false
    )
    {
        gbLogError("'BIT U3, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteBIT_U3_pR16 (gbProcessor* processor, uint8_t bit_index, 
    gbRegisterType src)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'BIT U3, (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'BIT U3, (R16)': No parent context available.");

    // - Validate Inputs
    //   - Bit Index (0-7)
    //   - Source Register (`BC`, `DE`, `HL`, `SP`)
    if (bit_index > 7)
    {
        gbLogError("'BIT U3, (R16)': Invalid bit index '%u'.", bit_index);
        return false;
    }
    else if (src < GB_RT_BC || src > GB_RT_SP)
    {
        gbLogError("'BIT U3, (R16)': Invalid source register '%s'.",
            gbStringifyRegisterType(src));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'BIT U3, (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from source register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Test specified bit.
    //   - Update flags as appropriate.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, src, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false ||
        gbWriteFlag(processor, GB_PF_Z, ((value & (1 << bit_index)) == 0)) == false ||
        gbWriteFlag(processor, GB_PF_N, false) == false ||
        gbWriteFlag(processor, GB_PF_H, true) == false
    )
    {
        gbLogError("'BIT U3, (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRES_U3_R8 (gbProcessor* processor, uint8_t bit_index, 
    gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RES U3, R8': No processor available.");

    // - Validate Inputs
    //   - Bit Index (0-7)
    //   - Destination Register (`A` through `L`)
    if (bit_index > 7)
    {
        gbLogError("'RES U3, R8': Invalid bit index '%u'.", bit_index);
        return false;
    }
    else if (dest > GB_RT_L)
    {
        gbLogError("'RES U3, R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RES U3, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Reset specified bit to 0.
    //   - Write result back to destination register.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'RES U3, R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value & ~(1 << bit_index);
    if (gbWriteRegisterByte(processor, dest, result) == false)
    {
        gbLogError("'RES U3, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteRES_U3_pR16 (gbProcessor* processor, uint8_t bit_index, 
    gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'RES U3, (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'RES U3, (R16)': No parent context available.");

    // - Validate Inputs
    //   - Bit Index (0-7)
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (bit_index > 7)
    {
        gbLogError("'RES U3, (R16)': Invalid bit index '%u'.", bit_index);
        return false;
    }
    else if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'RES U3, (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'RES U3, (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Reset specified bit to 0.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'RES U3, (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value & ~(1 << bit_index);
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'RES U3, (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSET_U3_R8 (gbProcessor* processor, uint8_t bit_index, 
    gbRegisterType dest)
{
    // - Validate Processor
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SET U3, R8': No processor available.");

    // - Validate Inputs
    //   - Bit Index (0-7)
    //   - Destination Register (`A` through `L`)
    if (bit_index > 7)
    {
        gbLogError("'SET U3, R8': Invalid bit index '%u'.", bit_index);
        return false;
    }
    else if (dest > GB_RT_L)
    {
        gbLogError("'SET U3, R8': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SET U3, R8': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read value from destination register.
    //   - Set specified bit to 1.
    //   - Write result back to destination register.
    uint8_t value = 0;
    if (gbReadRegisterByte(processor, dest, &value) == false)
    {
        gbLogError("'SET U3, R8': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value | (1 << bit_index);
    if (gbWriteRegisterByte(processor, dest, result) == false)
    {
        gbLogError("'SET U3, R8': Failed to execute instruction.");
        return false;
    }

    return true;
}

bool gbExecuteSET_U3_pR16 (gbProcessor* processor, uint8_t bit_index, 
    gbRegisterType dest)
{
    // - Validate Processor and Parent Context
    gbFallback(processor, gbGetProcessor(nullptr));
    gbCheckv(processor, false, "'SET U3, (R16)': No processor available.");
    gbContext* context = gbGetProcessorContext(processor);
    gbCheckv(context != nullptr, false, "'SET U3, (R16)': No parent context available.");

    // - Validate Inputs
    //   - Bit Index (0-7)
    //   - Destination Register (`BC`, `DE`, `HL`, `SP`)
    if (bit_index > 7)
    {
        gbLogError("'SET U3, (R16)': Invalid bit index '%u'.", bit_index);
        return false;
    }
    else if (dest < GB_RT_BC || dest > GB_RT_SP)
    {
        gbLogError("'SET U3, (R16)': Invalid destination register '%s'.",
            gbStringifyRegisterType(dest));
        return false;
    }

    // - Consume Fetch Cycles (Engine Mode)
    //   - 1 M-Cycle for Prefix Fetch
    //   - 1 M-Cycle for Opcode Fetch
    if (gbConsumeFetchCycles(processor, 2) == false)
    {
        gbLogError("'SET U3, (R16)': Failed to consume fetch cycles.");
        return false;
    }

    // - Execute Instruction
    //   - Read address from destination register.
    //   - Read value from memory at address.
    //   - Consume 1 M-Cycle for memory read.
    //   - Set specified bit to 1.
    //   - Write result back to memory at address.
    //   - Consume 1 M-Cycle for memory write.
    uint16_t address = 0;
    uint8_t value = 0;
    if (
        gbReadRegisterWord(processor, dest, &address) == false ||
        gbReadByte(context, address, &value, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {
        gbLogError("'SET U3, (R16)': Failed to execute instruction.");
        return false;
    }

    uint8_t result = value | (1 << bit_index);
    if (
        gbWriteByte(context, address, result, nullptr, nullptr) == false ||
        gbConsumeMachineCycles(processor, 1) == false
    )
    {        
        gbLogError("'SET U3, (R16)': Failed to execute instruction.");
        return false;
    }

    return true;
}
