#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <random>
#include <chrono>
#include <fstream>

// Width of the display
constexpr unsigned int VIDEO_WIDTH = 64;
// Height of the display
constexpr unsigned int VIDEO_HEIGHT = 32;
// Starting point for program-specific instructions to be read
constexpr unsigned int START_ADDRESS = 0x200;
// Size of the fontset array
constexpr unsigned int FONTSET_SIZE = 80;
// Starting address for fontset storage
constexpr unsigned int FONTSET_START_ADDRESS = 0x50;

// Array for every character expected
// Each character is a sprite consisting of 5 bytes
constexpr uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class Chip8 {
    public:
        Chip8();
        // Loads ROM file into memory
        void load_rom(char const* filename);
        // Main loop
        void cycle();

        // Methods for CPU instructions
        // 0nnn: SYS addr - Jump to a machine code routine at nnn
        // since 0nnn is only used on the old computers on which Chip-8
        // was originally implemented, it is ignored my modern interpreters
        // 00E0: CLS - Clears the display
        void OP_00E0();
        // 00EE: RET - Returns from a subroutine
        void OP_00EE();
        // 1nnn: JP addr - Jumps to nnn
        void OP_1nnn();
        // 2nnn: CALL addr - Calls subroutine at nnn
        void OP_2nnn();
        // 3xkk: SE Vx, byte - Skip next instruction if Vx == kk
        void OP_3xkk();
        // 4xkk: SNE Vx, byte - Skip next instruction if Vx != kk
        void OP_4xkk();
        // 5xy0: SE Vx, Vy - Skip next instruction if Vx == Vy
        void OP_5xy0();
        // 6xkk: LD Vx, byte - Set Vx = kk
        void OP_6xkk();
        // 7xkk: ADD Vx, byte - Set Vx += kk
        void OP_7xkk();
        // 8xy0: LD Vx, Vy - Set Vx = Vy
        void OP_8xy0();
        // 8xy1: OR Vx, Vy - Set Vx = Vx OR Vy
        void OP_8xy1();
        // 8xy2: AND Vx, Vy - Set Vx = Vx AND Vy
        void OP_8xy2();
        // 8xy3: XOR Vx, Vy - Set Vx = Vx XOR Vy
        void OP_8xy3();
        // 8xy4: ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry
        void OP_8xy4();
        // 8xy5: SUB Vx, Vy - Set Vx = Vx - Vym set VF = NOT borrow
        void OP_8xy5();
        // 8xy6: SHR Vx {, Vy} - Set Vx = Vx SHR 1
        // If the least significant bit of Vx is 1, then VF is set to 1,
        // other wise 0. Then VX is divided by 2
        void OP_8xy6();
        // 8xy7: SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow.
        void OP_8xy7();
        // 8xyE: SHL Vx {, Vy} - Set Vx = Vx SHL 1
        // If the most significant bit of Vx is 1, then VF is set to 1,
        // otherwise to 0. Then VX is multiplied by 2
        void OP_8xyE();
        // 9xy0: SNE Vx, Vy - Skip next instruction if Vx != Vy
        void OP_9xy0();
        // Annn: LD I, addr - Set register I = nnn
        void OP_Annn();
        // Bnnn: JP V0, addr - Jump to location nnn + V0
        void OP_Bnnn();
        // Cxkk: RND Vx, byte - Set Vx = random byte AND kk
        void OP_Cxkk();
        // Dxyn: DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location I at (Vx, Vy),
        // set VF = collision (1 if XOR causes pixels to be erased, else 0)
        void OP_Dxyn();
        // Ex9E: SKP Vx - Skip next instruction if key with value of Vx is pressed.
        void OP_Ex9E();
        // ExA1: SKNP Vx - Skip next instruction if key with value of Vx is not pressed.
        void OP_ExA1();
        // Fx07: LD Vx, DT - Set Vx = delay timer value
        void OP_Fx07();
        // Fx0A: LD Vx, K - Wait for a key press, store the value of the key in Vx
        void OP_Fx0A();
        // Fx15: LD DT, Vx - Set delay timer = Vx
        void OP_Fx15();
        // Fx18: LD ST, Vx - Set sound timer = Vx
        void OP_Fx18();
        // Fx1E: Add I, Vx - Set I = I + Vx
        void OP_Fx1E();
        // Fx29: LD F, Vx - Set I = location of sprite for digit Vx
        void OP_Fx29();
        // Fx33: LD B, Vx - Store BCD representation of Vx in memory location I, I+1 and I+2
        void OP_Fx33();
        // Fx55: LD [I], Vx - Store registers V0 through Vx in memory starting at location I
        void OP_Fx55();
        // Fx65: LD Vx, [I] - Read registers V0 through Vx from memory starting at location I
        void OP_Fx65();

        /*
        16 8-bit Registers
        ------------------
        Registers are labeled V0 to VF,
        each register can hold a value
        from 0x00 to 0xFF

        Register VF is used as a flag
        to hold information about the
        result of operations
        */
        uint8_t registers[16]{};

        /*
        4K (4096) Bytes of Memory
        -------------------------
        Address space: 0x000 to 0xFFF

        Segmented into three sections:

            - 0x000-0x1FF: Originally reserved for
              the CHIP-8 interpreter, but a modern
              emulator will MOSTLY never write or
              read from this.

            - 0x050-0x0A0: Storage space for the
              built-in characters (0 through F),
              which will need to be manually put
              into memory. ROMs will look for these

            - 0x200-0xFFF: Instructions from the ROM
              will be stored starting at 0x200, and
              anything left after the ROM's space
              is free to use
        */
        uint8_t memory[4096]{};

        /*
        16-bit Index Register
        ---------------------
        Special register used to store
        memory addresses for use in operations.
        It's 16 bit because that's the max-
        memory address (0xFFF)
        */
        uint16_t index{};

        /*
        16-bit Program Counter (PC)
        ---------------------------
        Special register that holds the address
        of the next instruction to execute. It is
        16 bit for the same reason as Index register

        An instruction is two bytes but memory is
        addressed as a single byte, so when fetching
        an instruction from memory you need to fetch a
        byte from PC and from PC+1 and connect them
        into a single value
        */
        uint16_t pc{};

        /*
        16-level Stack
        --------------
        The CPU uses the stack to keep track of the
        order of execution when it calls into functions.
        16 levels means it can hold 16 PCs
        */
        uint16_t stack[16]{};

        /*
        8-bit Stack Pointer
        -------------------
        The SP keeps track of where in the stack the most
        recent value was placed (i.e, the top)

        It only needs 8 bits because it's an index into the
        stack which is represented as an array
        */
        uint8_t sp{};

        /*
        8-bit Delay timer
        -----------------
        If the timer value is zero, it stays zero
        If it is loaded with a value, it will decrement
        at a rate of 60Hz
        */
        uint8_t delay_timer{};

        /*
        8-bit Sound Timer
        -----------------
        The same behavior as the delay timer, but when
        the value is non-zero it will buzz with a single tone
        */
        uint8_t sound_timer{};

        /*
        16 Input Keys
        -------------
        Has keys that match the first 16 hex values: 0 through F,
        each key is either pressed or not pressed

        These keys will be mapped to something more useable on
        a modern keyboard
        */
        uint8_t keypad[16]{};

        /*
        64x32 Monochrome Display Memory
        -------------------------------
        An additional memory buffer used for storing the
        graphics to display which is 64x32 pixels, each
        pixel is either on or off, so there's only two colors

        A uint32 is used for each pixel for easy use with SDL
        (on is 0xFFFFFFFF, off is 0x00000000)
        */
        uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};

        uint16_t opcode;

        // Member vars for the RNG instruction Cxkk - RND
        std::default_random_engine randGen;
        std::uniform_int_distribution<uint8_t> randByte;
};
