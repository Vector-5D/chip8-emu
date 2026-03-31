#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_audio.h>
#include <cstdint>
#include <cstdlib>
#include <math.h>

#define SAMPLE_RATE 48000

class Platform {
    public:
        Platform(
            char const* title,
            int w_width,   int w_height,
            int tex_width, int tex_height);
        ~Platform();

        void update(void const* buffer, int pitch, uint8_t sound_timer);
        bool processInput(uint8_t* keys);

    private:
        SDL_Window*       window{};
        SDL_Renderer*     renderer{};
        SDL_Texture*      texture{};
        SDL_AudioSpec     spec{};
        SDL_AudioDeviceID audio_device{};
        SDL_AudioStream*  stream{};
        float             buffer[SAMPLE_RATE]{};
        float             freq{};
        float             phase{};
};
