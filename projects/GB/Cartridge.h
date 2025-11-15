/**
 * @file    GB/Cartridge.h
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-11-15
 * 
 * @brief   Contains declarations for the Game Boy Emulator's cartridge
 *          component.
 */

#pragma once

/* Public Includes ************************************************************/

#include <GB/Context.h>

/* Public Constants and Enumerations ******************************************/

/**
 * @brief   Enumerates valid cartridge type bytes defined in a Game Boy
 *          cartridge device's header.
 */
typedef enum gbCartridgeType
{ 
    // Basic Cartridges (no MBC)
    GB_CT_BASIC                     = 0x00, /** @brief ROM only */
    GB_CT_BASIC_RAM                 = 0x08, /** @brief ROM with RAM */
    GB_CT_BASIC_RAM_BATTERY         = 0x09, /** @brief ROM with RAM, Battery */

    // MBC1 Cartridges
    GB_CT_MBC1                      = 0x01, /** @brief MBC1, ROM only */
    GB_CT_MBC1_RAM                  = 0x02, /** @brief MBC1 with RAM */
    GB_CT_MBC1_RAM_BATTERY          = 0x03, /** @brief MBC1 with RAM, Battery */

    // MBC2 Cartridges
    GB_CT_MBC2                      = 0x05, /** @brief MBC2, ROM with built-in RAM */
    GB_CT_MBC2_BATTERY              = 0x06, /** @brief MBC2 with Battery */

    // MBC3 Cartridges
    GB_CT_MBC3                      = 0x11, /** @brief MBC3, ROM only */
    GB_CT_MBC3_RAM                  = 0x12, /** @brief MBC3 with RAM */
    GB_CT_MBC3_RAM_BATTERY          = 0x13, /** @brief MBC3 with RAM, Battery */
    GB_CT_MBC3_TIMER_BATTERY        = 0x0F, /** @brief MBC3 with Timer, Battery */
    GB_CT_MBC3_TIMER_RAM_BATTERY    = 0x10, /** @brief MBC3 with Timer, RAM, Battery */

    // MBC5 Cartridges
    GB_CT_MBC5                      = 0x19, /** @brief MBC5, ROM only */
    GB_CT_MBC5_RAM                  = 0x1A, /** @brief MBC5 with RAM */
    GB_CT_MBC5_RAM_BATTERY          = 0x1B, /** @brief MBC5 with RAM, Battery */
    GB_CT_MBC5_RUMBLE               = 0x1C, /** @brief MBC5 with Rumble */
    GB_CT_MBC5_RUMBLE_RAM           = 0x1D, /** @brief MBC5 with Rumble, RAM */
    GB_CT_MBC5_RUMBLE_RAM_BATTERY   = 0x1E, /** @brief MBC5 with Rumble, RAM, Battery */
} gbCartridgeType;

/* Public Unions and Structures ***********************************************/

/**
 * @brief   Defines a structure representing the 80-byte header of a Game Boy
 *          cartridge ROM image.
 */
typedef struct gbCartridgeHeader
{
    uint8_t             entryPoint[4];                  /** @brief `$0100` - Entry Point Instructions */
    uint8_t             nintendoLogo[48];               /** @brief `$0104` - Nintendo Logo Bitmap Data */
    union
    {
        char            title[16];                      /** @brief `$0134` - Game Title (ASCII) */
        struct
        {
            char        shortTitle[15];                 /** @brief `$0134` - Short Game Title + Manufacturer Code (ASCII) */
            uint8_t     cgbFlag;                        /** @brief `$0143` - Indicates Game Boy Color support */
        };
    };
    uint8_t             newLicenseeCode[2];             /** @brief `$0144` - New Licensee Code (ASCII) */
    uint8_t             sgbSupport;                     /** @brief `$0146` - Indicates Super Game Boy support */
    uint8_t             cartridgeType;                  /** @brief `$0147` - Cartridge Type (see @a `gbCartridgeType`) */
    uint8_t             romSizeByte;                    /** @brief `$0148` - ROM Size Indicator */
    uint8_t             ramSizeByte;                    /** @brief `$0149` - RAM Size Indicator */
    uint8_t             destinationCode;                /** @brief `$014A` - Destination Code (`0` = Japanese, `1` = Non-Japanese) */
    uint8_t             oldLicenseeCode;                /** @brief `$014B` - Old Licensee Code */
    uint8_t             maskRomVersion;                 /** @brief `$014C` - Mask ROM Version Number */
    uint8_t             headerChecksum;                 /** @brief `$014D` - Header Checksum */
    uint16_t            globalChecksum;                 /** @brief `$014E` - Global Checksum */
} gbCartridgeHeader;

/* Public Function Declarations ***********************************************/

/**
 * @brief   Creates and loads a Game Boy cartridge device by loading a ROM image
 *          from the specified file path.
 *
 * @param   filepath    A pointer to a null-terminated string containing the
 *                      path to the ROM image file to be loaded. Must not be
 *                      `nullptr`, and also must not be an empty string.
 * 
 * @return  If successful, a pointer to the newly created @a `gbCartridge`
 *          structure containing the loaded ROM image.
 *          If loading fails (e.g., file not found, invalid format, allocation
 *          failure, etc.), returns `nullptr`.
 */
GB_API gbCartridge* gbCreateCartridge (const char* filepath);

/**
 * @brief   Destroys and deallocates a Game Boy cartridge device.
 * 
 * @param   cartridge   A pointer to the @a `gbCartridge` structure to be
 *                      destroyed. Must not be `nullptr`.
 *
 * @return  If successful, returns `true`.
 *          If no cartridge is provided (i.e., `nullptr`), returns `false`.
 */
GB_API bool gbDestroyCartridge (gbCartridge* cartridge);

/* Public Function Declarations - Cartridge Header ****************************/

/**
 * @brief   Retrieves a pointer to the header information of the specified
 *          Game Boy cartridge device.
 *
 * @param   cartridge   A pointer to the @a `gbCartridge` structure whose header
 *                      information is to be retrieved. Must not be `nullptr`.
 *
 * @return  A pointer to the @a `gbCartridgeHeader` structure containing the
 *          header information of the specified cartridge.
 */
GB_API const gbCartridgeHeader* gbGetCartridgeHeader (
    const gbCartridge* cartridge);

/**
 * @brief   Retrieves the title of the Game Boy cartridge from its header.
 *
 * @param   header      A pointer to the @a `gbCartridgeHeader` structure from
 *                      which to retrieve the cartridge title. Must not be
 *                      `nullptr`.
 * 
 * @return  A pointer to a null-terminated string containing the cartridge title.
 */
GB_API const char* gbGetCartridgeTitle (const gbCartridgeHeader* header);

/**
 * @brief   Checks whether the specified Game Boy cartridge supports or requires
 *          the enhanced features of the Game Boy Color, based on its header
 *          information.
 * 
 * @param   header          A pointer to the @a `gbCartridgeHeader` structure
 *                          to be checked. Must not be `nullptr`.
 * @param   outSupportsCGB  A pointer to a boolean variable where the result
 *                          indicating whether the cartridge supports Game Boy
 *                          Color features will be stored. Must not be `nullptr`.
 * @param   outRequiresCGB  A pointer to a boolean variable where the result
 *                          indicating whether the cartridge requires Game Boy
 *                          Color features will be stored. Pass `nullptr` if this
 *                          information is not needed.
 * 
 * @return  If successful, returns `true`.
 *          If checking fails (e.g., null pointers, etc.), returns `false`.
 */
GB_API bool gbCheckCartridgeCGBSupport (const gbCartridgeHeader* header,
    bool* outSupportsCGB, bool* outRequiresCGB);

/**
 * @brief   Converts a cartridge type byte from a Game Boy cartridge header into
 *          a human-readable string representation.
 * 
 * @param   header      A pointer to the @a `gbCartridgeHeader` structure whose
 *                      cartridge type is to be converted. Must not be `nullptr`.
 *
 * @return  If the cartridge type is recognized, returns a pointer to a
 *          null-terminated string describing the cartridge type.
 *          If the cartridge type is unrecognized, returns `nullptr`.
 */
GB_API const char* gbStringifyCartridgeType (const gbCartridgeHeader* header);

/**
 * @brief   Retrieves the size of the ROM area of a Game Boy cartridge, in bytes,
 *          based on its header information.
 *
 * @param   header      A pointer to the @a `gbCartridgeHeader` structure from
 *                      which to retrieve the ROM size. Must not be `nullptr`.
 * 
 * @return  If the ROM size is successfully determined, returns the size in bytes.
 *          If the ROM size cannot be determined, returns `0`.
 */
GB_API size_t gbGetCartridgeROMSize (const gbCartridgeHeader* header);

/**
 * @brief   Retrieves the size of the RAM area of a Game Boy cartridge, in bytes,
 *          based on its header information.
 *
 * @param   header      A pointer to the @a `gbCartridgeHeader` structure from
 *                      which to retrieve the RAM size. Must not be `nullptr`.
 * 
 * @return  If the RAM size is successfully determined, returns the size in bytes.
 *          If the RAM size cannot be determined, returns `0`.
 */
GB_API size_t gbGetCartridgeRAMSize (const gbCartridgeHeader* header);

/* Public Function Declarations - Battery-Backed RAM **************************/

/**
 * @brief   Loads the battery-backed RAM data for a Game Boy cartridge from
 *          the specified file path.
 * 
 * @param   cartridge           A pointer to the @a `gbCartridge` structure for
 *                              which to load the RAM data. Must not be `nullptr`.
 * @param   filepath            A pointer to a null-terminated string containing the
 *                              path to the RAM data file to be loaded. Must not be
 *                              `nullptr`, and also must not be an empty string.
 * @param   evenIfNoBattery     If `true`, load RAM data even if the cartridge type
 *                              does not indicate the presence of a battery.
 * 
 * @return  If successful, or if the cartridge RAM is not loaded (eg. no external 
 *          RAM, no battery, file not found, etc.), returns `true`.
 *          If loading fails due to an error (e.g., file read error, allocation
 *          failure, etc.), returns `false`.
 */
GB_API bool gbLoadCartridgeRAM (gbCartridge* cartridge, const char* filepath,
    bool evenIfNoBattery);

/**
 * @brief   Saves the battery-backed RAM data for a Game Boy cartridge to the
 *          specified file path.
 * 
 * @param   cartridge           A pointer to the @a `gbCartridge` structure for
 *                              which to save the RAM data. Must not be `nullptr`.
 * @param   filepath            A pointer to a null-terminated string containing the
 *                              path to the RAM data file to be saved. Must not be
 *                              `nullptr`, and also must not be an empty string.
 * @param   evenIfNoBattery     If `true`, save RAM data even if the cartridge type
 *                              does not indicate the presence of a battery.
 *
 * @return  If successful, or if the cartridge RAM is not saved (eg. no external 
 *          RAM, no battery, etc.), returns `true`.
 *          If saving fails due to an error (e.g., file write error, etc.),
 *          returns `false`.
 */
GB_API bool gbSaveCartridgeRAM (const gbCartridge* cartridge,
    const char* filepath, bool evenIfNoBattery);

/* Public Function Declarations - Memory Access *******************************/

/**
 * @brief   Reads a byte from the specified address within the given Game Boy
 *          cartridge device's ROM area.
 * 
 * @param   cartridge   A pointer to the @a `gbCartridge` structure from which to
 *                      read data. Must not be `nullptr`.
 * @param   address     The 16-bit address from which to read the byte, relative
 *                      to the start of the ROM area. Must be within the range
 *                      `0x0000` to `0x7FFF`.
 * @param   outValue    A pointer to a byte variable where the value read from
 *                      the cartridge will be stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If reading fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 */
GB_API bool gbReadCartridgeROM (const gbCartridge* cartridge, uint16_t address,
    uint8_t* outValue);

/**
 * @brief   Reads a byte from the specified address within the given Game Boy
 *          cartridge device's RAM area.
 * 
 * @param   cartridge   A pointer to the @a `gbCartridge` structure from which to
 *                      read data. Must not be `nullptr`.
 * @param   address     The 16-bit address from which to read the byte, relative
 *                      to the start of the RAM area. Must be within the range
 *                      `0x0000` to `0x1FFF`.
 * @param   outValue    A pointer to a byte variable where the value read from
 *                      the cartridge will be stored. Must not be `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If reading fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 */
GB_API bool gbReadCartridgeRAM (const gbCartridge* cartridge, uint16_t address,
    uint8_t* outValue);

/**
 * @brief   Writes a byte to the specified address within the given Game Boy
 *          cartridge device's ROM area.
 * 
 * A Game Boy cartridge's ROM is typically read-only; however, certain cartridge
 * devices come with external hardware (such as memory bank controllers) that may
 * be affected by writes to specific ROM addresses and address ranges. This
 * function allows such writes to be emulated.
 * 
 * @param   cartridge   A pointer to the @a `gbCartridge` structure to which to
 *                      write data. Must not be `nullptr`.
 * @param   address     The 16-bit address to which to write the byte, relative
 *                      to the start of the ROM area. Must be within the range
 *                      `0x0000` to `0x7FFF`.
 * @param   value       The byte value to write to the specified address.
 * @param   outActual   A pointer to a byte variable where the actual value
 *                      written to the cartridge will be stored. Must not be
 *                      `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If writing fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 */
GB_API bool gbWriteCartridgeROM (gbCartridge* cartridge, uint16_t address,
    uint8_t value, uint8_t* outActual);

/**
 * @brief   Writes a byte to the specified address within the given Game Boy
 *          cartridge device's RAM area.
 * 
 * @param   cartridge   A pointer to the @a `gbCartridge` structure to which to
 *                      write data. Must not be `nullptr`.
 * @param   address     The 16-bit address to which to write the byte, relative
 *                      to the start of the RAM area. Must be within the range
 *                      `0x0000` to `0x1FFF`.
 * @param   value       The byte value to write to the specified address.
 * @param   outActual   A pointer to a byte variable where the actual value
 *                      written to the cartridge will be stored. Must not be
 *                      `nullptr`.
 * 
 * @return  If successful, returns `true`.
 *          If writing fails (e.g., invalid address, null pointers, etc.),
 *          returns `false`.
 */
GB_API bool gbWriteCartridgeRAM (gbCartridge* cartridge, uint16_t address,
    uint8_t value, uint8_t* outActual);
