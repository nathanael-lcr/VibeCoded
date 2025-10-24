#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "lfo.h"
#include "synth.h"

typedef struct {
    float data[BUFFER_SIZE];
    SDL_SpinLock lock;
} Waveform;

// Fonctions de base
void Waveform_Update(Waveform *wave, float *buffer);
void Waveform_Draw(SDL_Renderer *ren, Waveform *wave, int w, int h);

// Fonctions avec LFO
void Waveform_DrawLFO(SDL_Renderer *ren, LFO *lfo, int w, int h, SDL_Color color);
void Waveform_DrawLFOsWithLegend(SDL_Renderer *ren, LFO *lfos, int lfo_count,
                                  int w, int h, TTF_Font *font);
void Waveform_DrawWithLFOs(SDL_Renderer *ren, Waveform *wave, LFO *lfos,
                           int lfo_count, int w, int h);

#endif // WAVEFORM_H