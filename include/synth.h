#ifndef SYNTH_H
#define SYNTH_H

#include <SDL3/SDL.h>

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 2048

typedef struct {
    float phase;
    float freq;
    float target_freq;
    float amplitude;
} Synth;

void Synth_Init(Synth *synth, float freq);
void Synth_Generate(Synth *synth, float *buffer, int len);
void Synth_SetFrequency(Synth *synth, float freq);

#endif
