#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#include <cstdint>

class Platform {
    public:
        Platform(
            char const* title,
            int w_width,   int w_height,
            int tex_width, int tex_height);
        ~Platform();

        void update(void const* buffer, int pitch);
        bool processInput(uint8_t* keys);

    private:
        SDL_Window*   window{};
        SDL_Renderer* renderer{};
        SDL_Texture*  texture{};
};
