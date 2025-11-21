# GABLE - GAme Boy-Like Engine

The GABLE project is an experimental, open-source project exploring the idea of
a game engine for developing games on modern hardware, but is backed by a
cycle-accurate, classic game console emulator core. The goal of this engine is
to build games on modern systems which mimic, as accurately as possible, the
look, feel, sound, behavior, limitations, quirks and cycle-accracy of that
classic game console, while still being able to leverage modern hardware and
APIs for rendering, audio, input, etc. The classic game console being emulated
in the case of GABLE is the Nintendo Game Boy and Game Boy Color.

**Disclaimer**: This project involves emulation of proprietary hardware and
software, and is provided for educational purposes only. The author does not
endorse or support piracy or the unauthorized use of copyrighted materials, and
will **not** provide ROMs of any kind (commercial, homebrew, testing, etc.), any
source code therefor, nor any means of building such ROMs - NO EXCEPTIONS. Users
of this project are expected to obtain or create their own ROMs in a legal manner.
The author of this project is not responsible for any legal issues or damages
which may arise from any misuse of this project, or from any violation of
copyright or intellectual property laws.

## Projects

Below is a brief overview of the main projects within the GABLE solution. All 
projects listed are C++23-compliant. The names of the projects correspond to 
their folders within the `projects/` folder:

- **`GB`** - Game Boy Emulation Core Library.
    - This is a library which implements a cycle-accurate emulator
      core for the Nintendo Game Boy and Game Boy Color consoles. This library is
      designed to be portable and easily integrated into other applications, and
      serves as the emulation core backing the GABLE engine.
- **`GBMU`** - Game Boy Emulator Frontend
    - This is a console application which serves as a frontend
      for the `GB` emulation core library. This application provides a user
      interface for loading and running Game Boy (Color) software, and
      demonstrates the capabilities of the `GB` library. This application is
      primarily intended for testing cycle-accuracy and correctness of the
      `GB` library.

## Resources

The following resources may be useful for understanding the GABLE project and its
goals:

- [Pan Docs](https://gbdev.io/pandocs/) - A community-driven specification of
    the Game Boy hardware and software.
- [GBZ80 Opcode Reference](https://rgbds.gbdev.io/docs/v0.6.1/gbz80.7) - A
    detailed reference of the instruction set used by the Sharp LR35902, the
    Game Boy's system-on-a-chip (SoC) CPU.
- [Gameboy CPU (LR35902) instruction set](https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html) -
    An opcode table showing each instruction, its hex code, length, and timing.
    