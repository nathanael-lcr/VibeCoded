#include "input.h"

float Note_FromKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_Q: return 261.63f;
        case SDLK_S: return 293.66f;
        case SDLK_D: return 329.63f;
        case SDLK_F: return 349.23f;
        case SDLK_G: return 392.00f;
        case SDLK_H: return 440.00f;
        case SDLK_J: return 493.88f;
        case SDLK_K: return 523.25f;
        default: return 0.0f;
    }
}
