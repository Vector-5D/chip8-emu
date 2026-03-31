#include <SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#include <chrono>
#include <iostream>
#include "Chip8.hpp"
#include "Platform.hpp"

/*
An emulator (interpreter) for the Chip-8 programming language.

Used sources:
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#9xy0
https://austinmorlan.com/posts/chip8_emulator/
*/

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    int video_scale = std::stoi(argv[1]);
    int cycle_delay = std::stoi(argv[2]);
    char const* rom_filename = argv[3];

    Platform platform(
        "CHIP-8 Emulator",
        VIDEO_WIDTH * video_scale,
        VIDEO_HEIGHT * video_scale,
        VIDEO_WIDTH, VIDEO_HEIGHT
    );

    Chip8 chip8;
    chip8.load_rom(rom_filename);

    int video_pitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    auto last_cycle_time = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit) {
        quit = platform.processInput(chip8.keypad);
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(
            current_time - last_cycle_time
        ).count();

        if (dt > cycle_delay) {
            last_cycle_time = current_time;
            chip8.cycle();
            platform.update(chip8.video, video_pitch, chip8.sound_timer);
        }
    }

    return 0;
}
