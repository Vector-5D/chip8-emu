#include "Chip8.hpp"

Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    // Program counter starts on 0x200 where program-specific
    // instructions start being stored
    pc = START_ADDRESS;

    // Load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Initialize RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

void Chip8::load_rom(char const* filename) {
    // Open as stream of binary and set file pointer to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        // Get the size of the file
        std::streampos size = file.tellg();
        // Allocate a buffer to hold the contents
        char* buffer = new char[size];

        // Go back to the start of the file
        file.seekg(0, std::ios::beg);
        // Fill the buffer with the contents of the file
        file.read(buffer, size);
        file.close(); // close

        // Load the ROM contents into memory
        // START_ADDRESS (0x200) is where program-specific
        // instructions start being written into memory
        for (size_t i = 0; i < size; ++i) {
            memory[START_ADDRESS + i] = buffer[i];
        }

        // Free the buffer
        delete[] buffer;
    }
}

void Chip8::cycle() {
    // Grab the 4-nibble opcode from memory
    opcode = (memory[pc] << 8u) | memory[pc + 1];
    pc += 2;

    // Decode and execute via the top nibble first
    switch (opcode & 0xF000u) {
        case 0x0000:
            switch (opcode & 0x00FFu) {
                case 0x00E0: OP_00E0(); break; // CLS
                case 0x00EE: OP_00EE(); break; // RET
                // 0nnn (SYS) intentionally ignored
            }
            break;

        case 0x1000: OP_1nnn(); break; // JP addr
        case 0x2000: OP_2nnn(); break; // CALL addr
        case 0x3000: OP_3xkk(); break; // SE Vx, byte
        case 0x4000: OP_4xkk(); break; // SNE Vx, byte
        case 0x5000: OP_5xy0(); break; // SE Vx, Vy
        case 0x6000: OP_6xkk(); break; // LD Vx, byte
        case 0x7000: OP_7xkk(); break; // ADD Vx, byte

        case 0x8000:
            // The last nibble distinguishes all 8xy_ instructions
            switch (opcode & 0x000Fu) {
                case 0x0: OP_8xy0(); break; // LD Vx, Vy
                case 0x1: OP_8xy1(); break; // OR
                case 0x2: OP_8xy2(); break; // AND
                case 0x3: OP_8xy3(); break; // XOR
                case 0x4: OP_8xy4(); break; // ADD (with carry)
                case 0x5: OP_8xy5(); break; // SUB
                case 0x6: OP_8xy6(); break; // SHR
                case 0x7: OP_8xy7(); break; // SUBN
                case 0xE: OP_8xyE(); break; // SHL
            }
            break;

        case 0x9000: OP_9xy0(); break; // SNE Vx, Vy
        case 0xA000: OP_Annn(); break; // LD I, addr
        case 0xB000: OP_Bnnn(); break; // JP V0, addr
        case 0xC000: OP_Cxkk(); break; // RND
        case 0xD000: OP_Dxyn(); break; // DRW

        case 0xE000:
            // Two key-input instructions differentiated by the low byte
            switch (opcode & 0x00FFu) {
                case 0x9E: OP_Ex9E(); break; // SKP Vx
                case 0xA1: OP_ExA1(); break; // SKNP Vx
            }
            break;

        case 0xF000:
            // For timer, memory, and misc instructions the low byte is the discriminator
            switch (opcode & 0x00FFu) {
                case 0x07: OP_Fx07(); break; // LD Vx, DT
                case 0x0A: OP_Fx0A(); break; // LD Vx, K
                case 0x15: OP_Fx15(); break; // LD DT, Vx
                case 0x18: OP_Fx18(); break; // LD ST, Vx
                case 0x1E: OP_Fx1E(); break; // ADD I, Vx
                case 0x29: OP_Fx29(); break; // LD F, Vx
                case 0x33: OP_Fx33(); break; // LD B, Vx
                case 0x55: OP_Fx55(); break; // LD [I], Vx
                case 0x65: OP_Fx65(); break; // LD Vx, [I]
            }
            break;
    }

    // Decrement timers
    if (delay_timer > 0) --delay_timer;
    if (sound_timer > 0) --sound_timer;
}

void Chip8::OP_00E0() {
    for (auto& v : video) {
        v = 0;
    }
}

void Chip8::OP_00EE() {
    --this->sp;
    pc = stack[sp];
}

void Chip8::OP_1nnn() {
    uint16_t nnn = opcode & 0x0FFFu;
    pc = nnn;
}

void Chip8::OP_2nnn() {
    uint16_t nnn = opcode & 0x0FFFu;
    stack[sp++] = pc;
    pc = nnn;
}

void Chip8::OP_3xkk() {
    uint8_t kk = opcode & 0x00FFu;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    if (kk == registers[Vx]) pc += 2;
}

void Chip8::OP_4xkk() {
    uint8_t kk = opcode & 0x00FFu;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    if (kk != registers[Vx]) pc += 2;
}

void Chip8::OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    if (registers[Vx] == registers[Vy]) pc += 2;
}

void Chip8::OP_6xkk() {
    uint8_t kk = opcode & 0x00FFu;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[Vx] = kk;
}

void Chip8::OP_7xkk() {
    uint8_t kk = opcode & 0x00FFu;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[Vx] += kk;
}

void Chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint16_t sum = registers[Vx] + registers[Vy];

    // Set Vf to 1 if the result is greater than 8 bits, else 0
    registers[0xF] = sum > 255u ? 1 : 0;
    // Put only the lowest 8 bits in Vx
    registers[Vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    // If Vx > Vy, Vf is set to 1, otherwise 0
    registers[0xF] = registers[Vx] > registers[Vy] ? 1 : 0;
    registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Put the least-significant bit of Vx in Vf
    registers[0xF] = registers[Vx] & 0x1u;
    // Divide Vx by two (identical to shift 1 to the right)
    registers[Vx] >>= 1;
}

void Chip8::OP_8xy7() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    // If Vx > Vy, Vf is set to 1, otherwise 0
    registers[0xF] = Vy > Vx ? 1 : 0;
    registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Put the most-significant bit of Vx in Vf
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
    // Multiply Vx by two (identical to shift 1 to the left)
    registers[Vx] <<= 1;
}

void Chip8::OP_9xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy]) pc += 2;
}

void Chip8::OP_Annn() {
    uint16_t nnn = opcode & 0x0FFFu;
    index = nnn;
}

void Chip8::OP_Bnnn() {
    uint16_t nnn = opcode & 0x0FFFu;
    pc = nnn + registers[0x0];
}

void Chip8::OP_Cxkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t kk = opcode & 0x00FFu;
	registers[Vx] = randByte(randGen) & kk;
}

void Chip8::OP_Dxyn() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t n = opcode & 0x000Fu;

    // Wrap around if Vx or Vy goes over display boundaries
    uint8_t x_pos = registers[Vx] % VIDEO_WIDTH;
    uint8_t y_pos = registers[Vy] % VIDEO_HEIGHT;

    // Reset collision flag
    registers[0xF] = 0;

    for (unsigned int row = 0; row < n; ++row) {
        // Each sprite row is one byte in memory
        uint8_t sprite_byte = memory[index + row];

        // A row is always 8 bits
        for (unsigned int col = 0; col < 8; ++col) {
            // Get the most significant bit (pixel) from the sprite row
            uint8_t sprite_pixel = sprite_byte & (0x80u >> col);
            uint32_t* screen_pixel = &video[(y_pos + row) * VIDEO_WIDTH + (x_pos + col)];

            if (sprite_pixel) {
                // If the pixel was already set, set VF to 1 (collision)
                if (*screen_pixel == 0xFFFFFFFF) registers[0xF] = 1;
                // XOR the pixel
                *screen_pixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];
    if (keypad[key]) pc += 2;
}

void Chip8::OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];
    if (!keypad[key]) pc += 2;
}

void Chip8::OP_Fx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[Vx] = delay_timer;
}

void Chip8::OP_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0]) {
		registers[Vx] = 0;
	}
	else if (keypad[1]) {
		registers[Vx] = 1;
	}
	else if (keypad[2]) {
		registers[Vx] = 2;
	}
	else if (keypad[3]) {
		registers[Vx] = 3;
	}
	else if (keypad[4]) {
		registers[Vx] = 4;
	}
	else if (keypad[5]) {
		registers[Vx] = 5;
	}
	else if (keypad[6]) {
		registers[Vx] = 6;
	}
	else if (keypad[7]) {
		registers[Vx] = 7;
	}
	else if (keypad[8]) {
		registers[Vx] = 8;
	}
	else if (keypad[9]) {
		registers[Vx] = 9;
	}
	else if (keypad[10]) {
		registers[Vx] = 10;
	}
	else if (keypad[11]) {
		registers[Vx] = 11;
	}
	else if (keypad[12]) {
		registers[Vx] = 12;
	}
	else if (keypad[13]) {
		registers[Vx] = 13;
	}
	else if (keypad[14]) {
		registers[Vx] = 14;
	}
	else if (keypad[15]) {
		registers[Vx] = 15;
	}
	else {
	    // Decrementing PC by 2 is effectively the same
	    // as running the same instruction repeatedly
		pc -= 2;
	}
}

void Chip8::OP_Fx15() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    delay_timer = registers[Vx];
}

void Chip8::OP_Fx18() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    sound_timer = registers[Vx];
}

void Chip8::OP_Fx1E() {
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    index += registers[Vx];
}

void Chip8::OP_Fx29() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    // All fonts are 5 bytes each
    index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}

void Chip8::OP_Fx55() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i) {
        memory[index + i] = registers[i];
    }
}

void Chip8::OP_Fx65() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i) {
        registers[i] = memory[index + i];
    }
}
