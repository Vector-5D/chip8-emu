#include "Platform.hpp"
#include <SDL3/SDL_audio.h>

Platform::Platform(
    char const* title,
    int w_width,   int w_height,
    int tex_width, int tex_height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    if (!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow(title, w_width, w_height, 0);

    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        tex_width,
        tex_height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    spec.freq = SAMPLE_RATE;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;

    audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    stream = SDL_CreateAudioStream(&spec, &spec);

    SDL_BindAudioStream(audio_device, stream);

    freq = 440.0f;
    phase = 0.0f;

    for (int i = 0; i < SAMPLE_RATE; i++) {
        buffer[i] = sinf(phase * 2.0f * M_PI);
        phase += freq / SAMPLE_RATE;
    }

    SDL_PutAudioStreamData(stream, buffer, sizeof(buffer));
}

Platform::~Platform() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyAudioStream(stream);
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}

void Platform::update(void const* buffer, int pitch, uint8_t sound_timer) {
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    bool paused = SDL_AudioDevicePaused(audio_device);

    if (paused && sound_timer > 0) {
        SDL_ResumeAudioDevice(audio_device);
    } else if (!paused && sound_timer <= 0) {
        SDL_PauseAudioDevice(audio_device);
    }
}

bool Platform::processInput(uint8_t* keys) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                quit = true;
                break;

            case SDL_EVENT_KEY_DOWN:
                switch (event.key.key) {
                    case SDLK_ESCAPE: quit      = true; break;
                    case SDLK_X:      keys[0x0] = 1;    break;
                    case SDLK_1:      keys[0x1] = 1;    break;
                    case SDLK_2:      keys[0x2] = 1;    break;
                    case SDLK_3:      keys[0x3] = 1;    break;
                    case SDLK_Q:      keys[0x4] = 1;    break;
                    case SDLK_W:      keys[0x5] = 1;    break;
                    case SDLK_E:      keys[0x6] = 1;    break;
                    case SDLK_A:      keys[0x7] = 1;    break;
                    case SDLK_S:      keys[0x8] = 1;    break;
                    case SDLK_D:      keys[0x9] = 1;    break;
                    case SDLK_Z:      keys[0xA] = 1;    break;
                    case SDLK_C:      keys[0xB] = 1;    break;
                    case SDLK_4:      keys[0xC] = 1;    break;
                    case SDLK_R:      keys[0xD] = 1;    break;
                    case SDLK_F:      keys[0xE] = 1;    break;
                    case SDLK_V:      keys[0xF] = 1;    break;
                }
                break;

            case SDL_EVENT_KEY_UP:
                switch (event.key.key) {
                    case SDLK_X: keys[0x0] = 0; break;
                    case SDLK_1: keys[0x1] = 0; break;
                    case SDLK_2: keys[0x2] = 0; break;
                    case SDLK_3: keys[0x3] = 0; break;
                    case SDLK_Q: keys[0x4] = 0; break;
                    case SDLK_W: keys[0x5] = 0; break;
                    case SDLK_E: keys[0x6] = 0; break;
                    case SDLK_A: keys[0x7] = 0; break;
                    case SDLK_S: keys[0x8] = 0; break;
                    case SDLK_D: keys[0x9] = 0; break;
                    case SDLK_Z: keys[0xA] = 0; break;
                    case SDLK_C: keys[0xB] = 0; break;
                    case SDLK_4: keys[0xC] = 0; break;
                    case SDLK_R: keys[0xD] = 0; break;
                    case SDLK_F: keys[0xE] = 0; break;
                    case SDLK_V: keys[0xF] = 0; break;
                }
                break;
        }
    }

    return quit;
}
