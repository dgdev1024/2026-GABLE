/**
 * @file    GB/Context.h
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains declarations for the Game Boy Emulator Core context.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Common.h>

/* Public Types and Forward Declarations **************************************/

/**
 * @brief   Defines an opaque structure representing the main context of the
 *          Game Boy Emulator Core.
 * 
 * This structure encapsulates all of the emulator's components, including CPU,
 * memory, input/output, and other subsystems necessary for emulating the Game
 * Boy and Game Boy Color hardware.
 */
typedef struct gbContext gbContext;

/**
 * @brief   Defines an opaque structure representing a Game Boy cartridge
 *          device.
 * 
 * This structure encapsulates the ROM and RAM data, as well as any other
 * hardware features (such as memory bank controllers) found in a Game Boy
 * cartridge device.
 * 
 * The cartridge is an external component which is attached to the Game Boy
 * Emulator Core context. Its memory is not managed by the core context itself,
 * and will need to be freed separately when no longer needed.
 */
typedef struct gbCartridge gbCartridge;

/**
 * @brief   Defines an opaque structure representing the Game Boy Emulator Core's
 *          internal, general-purpose random-access memory (RAM) component.
 * 
 * This structure encapsulates the Game Boy system's work RAM (WRAM) and
 * zero-page "high" RAM (HRAM) areas, providing methods for reading from and
 * writing to these memory regions.
 */
typedef struct gbMemory gbMemory;

/**
 * @brief   Defines an opaque structure representing the Game Boy Emulator Core's
 *          CPU processor component.
 * 
 * This structure encapsulates the CPU's registers, flags, and execution state,
 * providing methods for fetching, decoding, and executing instructions, as well
 * as handling interrupts and other CPU-related operations.
 */
typedef struct gbProcessor gbProcessor;

/**
 * @brief   Defines an opaque structure representing the Game Boy Emulator Core's
 *          internal timer component.
 * 
 * This structure encapsulates the timer registers and state, providing methods
 * for managing the Game Boy's internal timing mechanisms.
 */
typedef struct gbTimer gbTimer;

/**
 * @brief   Defines an opaque structure representing the Game Boy Emulator Core's
 *          internal pixel processing unit (PPU) component (henceforth, the
 *          "renderer").
 * 
 * This structure encapsulates the PPU's registers, state, and rendering
 * mechanisms, providing methods for managing the Game Boy's graphics output.
 */
typedef struct gbRenderer gbRenderer;

/**
 * @brief   Defines a pointer to a function called by the Game Boy Emulator Core
 *          context when a read operation is attempted on its emulated, 16-bit
 *          address bus.
 * 
 * @param   context     A pointer to the @a `gbContext` attempting the read operation.
 * @param   address     The 16-bit, absolute address from which data is being read.
 * @param   value       The byte value which was read from the bus.
 */
typedef void (*gbBusReadCallback) (const gbContext* context, uint16_t address,
    uint8_t value);

/**
 * @brief   Defines a pointer to a function called by the Game Boy Emulator Core
 *          context when a write operation is attempted on its emulated, 16-bit
 *          address bus.
 * 
 * @param   context     A pointer to the @a `gbContext` attempting the write operation.
 * @param   address     The 16-bit, absolute address to which data is being written.
 * @param   value       The byte value being written to the bus.
 * @param   actual      The byte value that was actually written to the bus,
 *                      which may differ from @a `value` due to hardware quirks
 *                      or restrictions.
 */
typedef void (*gbBusWriteCallback) (gbContext* context, uint16_t address,
    uint8_t value, uint8_t actual);

/* Public Constants and Enumerations ******************************************/

/**
 * @brief   Defines constants indicating the start and end addresses of key
 *          regions within the Game Boy's 16-bit address space, and their sizes
 *          in bytes.
 */
#define GB_ROM0_START       0x0000
#define GB_ROM0_END         0x3FFF
#define GB_ROMX_START       0x4000
#define GB_ROMX_END         0x7FFF
#define GB_ROM_BANK_SIZE    0x4000
#define GB_ROM_SIZE         0x8000
#define GB_VRAM_START       0x8000
#define GB_VRAM_END         0x9FFF
#define GB_VRAM_SIZE        0x2000
#define GB_EXTRAM_START     0xA000
#define GB_EXTRAM_END       0xBFFF
#define GB_EXTRAM_SIZE      0x2000
#define GB_WRAM0_START      0xC000
#define GB_WRAM0_END        0xCFFF
#define GB_WRAMX_START      0xD000
#define GB_WRAMX_END        0xDFFF
#define GB_WRAM_BANK_SIZE   0x1000
#define GB_WRAM_SIZE        0x2000
#define GB_ECHO_START       0xE000
#define GB_ECHO_END         0xFDFF
#define GB_ECHO_SIZE        0x1E00
#define GB_OAM_START        0xFE00
#define GB_OAM_END          0xFE9F
#define GB_OAM_SIZE         0x00A0
#define GB_UNUSED_START     0xFEA0
#define GB_UNUSED_END       0xFEFF
#define GB_UNUSED_SIZE      0x0060
#define GB_HRAM_START       0xFF80
#define GB_HRAM_END         0xFFFE
#define GB_HRAM_SIZE        0x007F

/**
 * @brief   Enumerates specific addresses in the Game Boy's 16-bit address
 *          space which are mapped to port registers provided by the Game Boy's
 *          hardware components.
 */
typedef enum gbPortRegister : uint16_t
{
    GB_PR_P1    = 0xFF00,   /** @brief `P1`, `JOYP` - Joypad Input (Mixed) */
    GB_PR_SB    = 0xFF01,   /** @brief `SB` - Serial Transfer Data (R/W) */
    GB_PR_SC    = 0xFF02,   /** @brief `SC` - Serial Transfer Control (R/W) */
    GB_PR_DIV   = 0xFF04,   /** @brief `DIV` - Timer Divider (R/W) */
    GB_PR_TIMA  = 0xFF05,   /** @brief `TIMA` - Timer Counter (R/W) */
    GB_PR_TMA   = 0xFF06,   /** @brief `TMA` - Timer Modulo (R/W) */
    GB_PR_TAC   = 0xFF07,   /** @brief `TAC` - Timer Control (R/W) */
    GB_PR_IF    = 0xFF0F,   /** @brief `IF` - CPU Interrupt Flag (R/W) */
    GB_PR_NR10  = 0xFF10,   /** @brief `NR10` - APU Pulse Channel 1 Sweep (R/W) */
    GB_PR_NR11  = 0xFF11,   /** @brief `NR11` - APU Pulse Channel 1 Length Timer & Duty Cycle (Mixed) */
    GB_PR_NR12  = 0xFF12,   /** @brief `NR12` - APU Pulse Channel 1 Volume & Envelope (R/W) */
    GB_PR_NR13  = 0xFF13,   /** @brief `NR13` - APU Pulse Channel 1 Period Low (W) */
    GB_PR_NR14  = 0xFF14,   /** @brief `NR14` - APU Pulse Channel 1 Period High & Control (Mixed) */
    GB_PR_NR21  = 0xFF16,   /** @brief `NR21` - APU Pulse Channel 2 Length Timer & Duty Cycle (Mixed) */
    GB_PR_NR22  = 0xFF17,   /** @brief `NR22` - APU Pulse Channel 2 Volume & Envelope (R/W) */
    GB_PR_NR23  = 0xFF18,   /** @brief `NR23` - APU Pulse Channel 2 Period Low (W) */
    GB_PR_NR24  = 0xFF19,   /** @brief `NR24` - APU Pulse Channel 2 Period High & Control (Mixed) */
    GB_PR_NR30  = 0xFF1A,   /** @brief `NR30` - APU Wave Channel DAC Enable (R/W) */
    GB_PR_NR31  = 0xFF1B,   /** @brief `NR31` - APU Wave Channel Length Timer (W) */
    GB_PR_NR32  = 0xFF1C,   /** @brief `NR32` - APU Wave Channel Output Level (R/W) */
    GB_PR_NR33  = 0xFF1D,   /** @brief `NR33` - APU Wave Channel Period Low (W) */
    GB_PR_NR34  = 0xFF1E,   /** @brief `NR34` - APU Wave Channel Period High & Control (Mixed) */
    GB_PR_NR41  = 0xFF20,   /** @brief `NR41` - APU Noise Channel Length Timer (W) */
    GB_PR_NR42  = 0xFF21,   /** @brief `NR42` - APU Noise Channel Volume & Envelope (R/W) */
    GB_PR_NR43  = 0xFF22,   /** @brief `NR43` - APU Noise Channel Frequency & Randomness (R/W) */
    GB_PR_NR44  = 0xFF23,   /** @brief `NR44` - APU Noise Channel Control (Mixed) */
    GB_PR_NR50  = 0xFF24,   /** @brief `NR50` - APU Master Volume & VIN Panning (R/W) */
    GB_PR_NR51  = 0xFF25,   /** @brief `NR51` - APU Sound Panning (R/W) */
    GB_PR_NR52  = 0xFF26,   /** @brief `NR52` - APU Sound On/Off (Mixed) */
    GB_PR_LCDC  = 0xFF40,   /** @brief `LCDC` - PPU LCD Control (R/W) */
    GB_PR_STAT  = 0xFF41,   /** @brief `STAT` - PPU LCD Status (Mixed) */
    GB_PR_SCY   = 0xFF42,   /** @brief `SCY` - PPU Viewport Y Position (R/W) */
    GB_PR_SCX   = 0xFF43,   /** @brief `SCX` - PPU Viewport X Position (R/W) */
    GB_PR_LY    = 0xFF44,   /** @brief `LY` - PPU LCD Y Coordinate (R) */
    GB_PR_LYC   = 0xFF45,   /** @brief `LYC` - PPU LY Compare (R/W) */
    GB_PR_DMA   = 0xFF46,   /** @brief `DMA` - PPU OAM DMA Source Address & Start (R/W) */
    GB_PR_BGP   = 0xFF47,   /** @brief `BGP` - PPU Monochrome BG Palette Data (R/W) */
    GB_PR_OBP0  = 0xFF48,   /** @brief `OBP0` - PPU Monochrome OBJ Palette 0 Data (R/W) */
    GB_PR_OBP1  = 0xFF49,   /** @brief `OBP1` - PPU Monochrome OBJ Palette 1 Data (R/W) */
    GB_PR_WY    = 0xFF4A,   /** @brief `WY` - PPU Window Y Position (R/W) */
    GB_PR_WX    = 0xFF4B,   /** @brief `WX` - PPU Window X Position + 7 (R/W) */
    GB_PR_KEY0  = 0xFF4C,   /** @brief `KEY0`, `SYS` - CGB Only - CPU Mode Select (Mixed) */
    GB_PR_KEY1  = 0xFF4D,   /** @brief `KEY1`, `SPD` - CGB Only - CPU Prepare Speed Switch (Mixed) */
    GB_PR_VBK   = 0xFF4F,   /** @brief `VBK` - CGB Only - VRAM Bank Select (R/W) */
    GB_PR_BANK  = 0xFF50,   /** @brief `BANK` - Boot ROM Mapping Control (W) */
    GB_PR_HDMA1 = 0xFF51,   /** @brief `HDMA1` - CGB Only - VRAM DMA Source High (W) */
    GB_PR_HDMA2 = 0xFF52,   /** @brief `HDMA2` - CGB Only - VRAM DMA Source Low (W) */
    GB_PR_HDMA3 = 0xFF53,   /** @brief `HDMA3` - CGB Only - VRAM DMA Destination High (W) */
    GB_PR_HDMA4 = 0xFF54,   /** @brief `HDMA4` - CGB Only - VRAM DMA Destination Low (W) */
    GB_PR_HDMA5 = 0xFF55,   /** @brief `HDMA5` - CGB Only - VRAM DMA Length/Mode/Start (R/W) */
    GB_PR_RP    = 0xFF56,   /** @brief `RP` - CGB Only - Infrared Communications Port (Mixed) */
    GB_PR_BCPS  = 0xFF68,   /** @brief `BCPS` - CGB Only - BG Color Palette Specification (R/W) */
    GB_PR_BCPD  = 0xFF69,   /** @brief `BCPD` - CGB Only - BG Color Palette Data (R/W) */
    GB_PR_OCPS  = 0xFF6A,   /** @brief `OCPS` - CGB Only - OBJ Color Palette Specification (R/W) */
    GB_PR_OCPD  = 0xFF6B,   /** @brief `OCPD` - CGB Only - OBJ Color Palette Data (R/W) */
    GB_PR_OPRI  = 0xFF6C,   /** @brief `OPRI` - CGB Only - Object Priority Mode (R/W) */
    GB_PR_SVBK  = 0xFF70,   /** @brief `SVBK` - CGB Only - WRAM Bank Select (R/W) */
    GB_PR_PCM12 = 0xFF76,   /** @brief `PCM12` - CGB Only - APU Digital Outputs 1 & 2 (R) */
    GB_PR_PCM34 = 0xFF77,   /** @brief `PCM34` - CGB Only - APU Digital Outputs 3 & 4 (R) */
    GB_PR_IE    = 0xFFFF    /** @brief `IE` - CPU Interrupt Enable (R/W) */
} gbPortRegister;

/* Public Unions and Structures ***********************************************/

/**
 * @brief   Defines a bitfield union representing the rules for which checks
 *          and access restrictions should be enfored when accessing certain
 *          areas of the Game Boy Emulator Core's memory map.
 */
typedef union gbCheckRules
{
    struct
    {
        uint8_t external    : 1;    /** @brief Enforce rules for external access (eg. from the CPU). */
        uint8_t internal    : 1;    /** @brief Enforce rules for internal access (eg. from within the managing component). */
        uint8_t speedSwitch : 1;    /** @brief CGB only - Enforce rules for internal access during a CPU speed switch. */
        uint8_t oamDMA      : 1;    /** @brief Enforce rules for access during an OAM DMA transfer. */
        uint8_t hblankDMA   : 1;    /** @brief CGB only - Enforce rules for access during an H-Blank DMA transfer. */
        uint8_t component1  : 1;    /** @brief Enforce component-specific rule 1. */
        uint8_t component2  : 1;    /** @brief Enforce component-specific rule 2. */
        uint8_t component3  : 1;    /** @brief Enforce component-specific rule 3. */
    };

    uint8_t value;
} gbCheckRules;

/* Public Function Declarations ***********************************************/

/**
 * @brief   Allocates and creates a new Game Boy Emulator Core context.
 * 
 * @param   engineMode  Create the engine to run in a special mode, which is yet
 *                      to be defined. Unused for now.
 * 
 * @return  If successful, a pointer to the newly created @a `gbContext` structure.
 *          If allocation fails, returns `nullptr`.
 */
GB_API gbContext* gbCreateContext (bool engineMode);

/**
 * @brief   Destroys and deallocates a Game Boy Emulator Core context.
 * 
 * @param   context     A pointer to the @a `gbContext` structure to be destroyed.
 *                      Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`. 
 *          If no context is provided (i.e., `nullptr`), returns `false`.
 */
GB_API bool gbDestroyContext (gbContext* context);

/**
 * @brief   Initializes (or resets) a Game Boy Emulator Core context.
 * 
 * This function initializes (or resets) the provided @a `gbContext` structure,
 * setting up all necessary components for emulation. This involves clearing any
 * buffers (RAM, VRAM, OAM, etc.), initializing internal states, resetting CPU
 * and hardware registers to their default power-on values, and preparing the 
 * context for execution.
 * 
 * @param   context     A pointer to the @a `gbContext` structure to be initialized
 *                      or reset. Pass `nullptr` to use the current context.
 * 
 * @return  If successful, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current
 *          context exists, returns `false`.
 */
GB_API bool gbInitializeContext (gbContext* context);

/* Public Function Declarations - Cartridge ***********************************/

/**
 * @brief   Attaches a cartridge device to the given Game Boy Emulator Core
 *          context.
 * 
 * Attaching (or detaching) a cartridge device causes the context and its
 * components to reset their states as if the system had been powered on or reset.
 * 
 * @param   context     A pointer to the @a `gbContext` structure to which to
 *                      attach the cartridge. Pass `nullptr` to use the current
 *                      context.
 * @param   cartridge   A pointer to the @a `gbCartridge` structure representing
 *                      the cartridge device to be attached. Pass `nullptr` to
 *                      detach any currently attached cartridge.
 * 
 * @return  If successful, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 */
GB_API bool gbAttachCartridge (gbContext* context,
    gbCartridge* cartridge);

/* Public Function Declarations - Context Operation Mode **********************/

/**
 * @brief   Checks whether the given Game Boy context is in Color Game Boy (CGB)
 *          mode, based on its attached cartridge and internal state.
 * 
 * @param   context         A pointer to the @a `gbContext` structure to be checked.
 *                          Pass `nullptr` to use the current context.
 * @param   outIsCGBMode    A pointer to a boolean variable where the result
 *                          indicating whether the context is in CGB mode will be
 *                          stored. Must not be `nullptr`.
 * 
 * @return  If checked successfully, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 */
GB_API bool gbCheckCGBMode (const gbContext* context, bool* outIsCGBMode);

/**
 * @brief   Checks whether the given Game Boy context is operating in "engine
 *          mode".
 * 
 * "Engine mode" is a planned, yet-unimplemented, special mode of operation for
 * the Game Boy Emulator Core. When enabled, it may alter the behavior of certain
 * components or features of the emulator. The specifics of this mode are yet to
 * be defined, but the intent is to provide a way use the emulator core as the
 * backing for a more complex game engine for native game development.
 * 
 * @param   context         A pointer to the @a `gbContext` structure to be checked.
 *                          Pass `nullptr` to use the current context.
 * @param   outIsEngineMode A pointer to a boolean variable where the result
 *                          indicating whether the context is in engine mode will be
 *                          stored. Must not be `nullptr`.
 * 
 * @return  If checked successfully, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 */
GB_API bool gbCheckEngineMode (const gbContext* context, bool* outIsEngineMode);

/* Public Function Declarations - Current Context and Components **************/

/**
 * @brief   Designates the provided Game Boy Emulator Core context as the current
 *          context for subsequent operations.
 * 
 * By default, nearly all functions in the Game Boy Emulator Core library's
 * public API fall back to operating on a "current context" if one is not
 * provided explicitly. This function sets the specified context as that current
 * context.
 * 
 * @param   context     A pointer to the @a `gbContext` structure to become the
 *                      current context. Pass `nullptr` to un-set any current
 *                      context.
 * 
 * @return  If a context is set as the current context, returns `true`.
 *          If the current context is unset (by passing `nullptr`), returns
 *          `false` (this is not an error).
 */
GB_API bool gbMakeContextCurrent (gbContext* context);

/**
 * @brief   Retrieves the current Game Boy Emulator Core context.
 * 
 * @return  If a current context is set, returns a pointer to the current
 *          @a `gbContext` structure.
 *          If no current context is set, returns `nullptr`.
 */
GB_API gbContext* gbGetCurrentContext ();

/**
 * @brief   Retrieves the memory component associated with the given Game Boy
 *          Emulator Core context.
 * 
 * @param   context     A pointer to the @a `gbContext` structure from which to
 *                      retrieve the memory component. Pass `nullptr` to use
 *                      the current context.
 *
 * @return  If successful, returns a pointer to the associated @a `gbMemory`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `nullptr`.
 */
GB_API gbMemory* gbGetMemory (const gbContext* context);

/**
 * @brief   Retrieves the processor component associated with the given Game Boy
 *          Emulator Core context.
 * 
 * @param   context     A pointer to the @a `gbContext` structure from which to
 *                      retrieve the processor component. Pass `nullptr` to use
 *                      the current context.
 *
 * @return  If successful, returns a pointer to the associated @a `gbProcessor`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `nullptr`.
 */
GB_API gbProcessor* gbGetProcessor (const gbContext* context);

/**
 * @brief   Retrieves the timer component associated with the given Game Boy
 *          Emulator Core context.
 * 
 * @param   context     A pointer to the @a `gbContext` structure from which to
 *                      retrieve the timer component. Pass `nullptr` to use the
 *                      current context.
 *
 * @return  If successful, returns a pointer to the associated @a `gbTimer`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `nullptr`.
 */
GB_API gbTimer* gbGetTimer (const gbContext* context);

/* Public Function Declarations - Userdata ************************************/

/**
 * @brief   Sets the userdata pointer associated with the given Game Boy
 *          Emulator Core context.
 * 
 * @param   context     A pointer to the @a `gbContext` structure for which to
 *                      set the userdata pointer. Pass `nullptr` to use the
 *                      current context.
 * @param   userdata    A pointer to the userdata to associate with the context.
 *                      This can be any user-defined data or structure. Pass
 *                      `nullptr` to unset any existing userdata pointer.
 * 
 * @return  If successful, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 */
GB_API bool gbSetUserdata (gbContext* context, void* userdata);

/**
 * @brief   Retrieves the userdata pointer associated with the given Game Boy
 *          Emulator Core context.
 * 
 * @param   context     A pointer to the @a `gbContext` structure from which to
 *                      retrieve the userdata pointer. Pass `nullptr` to use the
 *                      current context.
 * 
 * @return  If a userdata pointer is set, returns the pointer.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `nullptr`.
 */
GB_API void* gbGetUserdata (const gbContext* context);

/* Public Function Declarations - Callbacks ***********************************/

/**
 * @brief   Sets the callback function to be invoked when a read operation is
 *          attempted on the given context's emulated address bus.
 * 
 * @param   context     A pointer to the @a `gbContext` structure for which to
 *                      set the bus read callback. Pass `nullptr` to use the
 *                      current context.
 * @param   callback    A pointer to the @a `gbBusReadCallback` function to be
 *                      invoked on bus read operations. Pass `nullptr` to clear
 *                      any existing callback.
 * 
 * @return  If successful, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 */
GB_API bool gbSetBusReadCallback (gbContext* context,
    gbBusReadCallback callback);

/**
 * @brief   Sets the callback function to be invoked when a write operation is
 *          attempted on the given context's emulated address bus.
 * 
 * @param   context     A pointer to the @a `gbContext` structure for which to
 *                      set the bus write callback. Pass `nullptr` to use the
 *                      current context.
 * @param   callback    A pointer to the @a `gbBusWriteCallback` function to be
 *                      invoked on bus write operations. Pass `nullptr` to clear
 *                      any existing callback.
 *
 * @return  If successful, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 */
GB_API bool gbSetBusWriteCallback (gbContext* context,
    gbBusWriteCallback callback);

/* Public Function Declarations - Ticking *************************************/

/**
 * @brief   Ticks the given Game Boy Emulator Core context, causing its processor
 *          component, and thereby its other components, to tick and perform
 *          their operations.
 * 
 * @param   context     A pointer to the @a `gbContext` structure to be ticked.
 *                      Pass `nullptr` to use the current context.
 * 
 * @return  If successful, returns `true`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 */
GB_API bool gbTick (gbContext* context);

/* Public Function Declarations - Address Bus *********************************/

/**
 * @brief   Reads a byte from the specified address on the given Game Boy
 *          Emulator Core context's 16-bit address bus.
 * 
 * @param   context     A pointer to the @a `gbContext` structure from which to
 *                      read the byte. Pass `nullptr` to use the current context.
 * @param   address     The 16-bit, absolute address from which to read the byte.
 * @param   outValue    A pointer to a byte variable where the value read from
 *                      the bus will be stored. Must not be `nullptr`.
 * @param   rules       A pointer to a @a `gbCheckRules` union specifying the
 *                      access rules to enforce during this read operation.
 *                      Pass `nullptr` to specify default CPU external access
 *                      rules, or a zeroed-out union to bypass all checks.
 * 
 * @return  If successful, returns `true` and stores the read byte in
 *          @a `outValue`.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, or if @a `outValue` is `nullptr`, returns `false`.
 * 
 * @note    If @a `address` is not mapped or access is denied based on the
 *          provided @a `rules`, the function will set @a `outValue` to an
 *          open-bus value (`0xFF`) and still return `true`.
 */
GB_API bool gbReadByte (const gbContext* context, uint16_t address,
    uint8_t* outValue, const gbCheckRules* rules);

/**
 * @brief   Writes a byte to the specified address on the given Game Boy
 *          Emulator Core context's 16-bit address bus.
 * 
 * @param   context     A pointer to the @a `gbContext` structure to which to
 *                      write the byte. Pass `nullptr` to use the current context.
 * @param   address     The 16-bit, absolute address to which to write the byte.
 * @param   value       The byte value to write to the bus.
 * @param   rules       A pointer to a @a `gbCheckRules` union specifying the
 *                      access rules to enforce during this write operation.
 *                      Pass `nullptr` to specify default CPU external access
 *                      rules, or a zeroed-out union to bypass all checks.
 * @param   outActual   A pointer to a byte variable where the actual value
 *                      written to the bus will be stored, which may differ from
 *                      the requested value due to hardware quirks or restrictions.
 *                      Pass `nullptr` if this information is not needed.
 * 
 * @return  If successful, returns `true` and stores the actual written byte in
 *          @a `outActual` if provided.
 *          If no context is provided (i.e., `nullptr`) and no current context
 *          exists, returns `false`.
 * 
 * @note    If @a `address` is not mapped or access is denied based on the
 *          provided @a `rules`, the function will not perform any write and
 *          will still return `true`, with @a `outActual` set to an open-bus
 *          value (`0xFF`) if provided.
 */
GB_API bool gbWriteByte (gbContext* context, uint16_t address,
    uint8_t value, const gbCheckRules* rules, uint8_t* outActual);
