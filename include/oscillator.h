#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <stdint.h>

#define MAX_OSCILLATORS 8
#define SAMPLE_RATE 44100

typedef enum {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH,
    WAVE_NOISE
} WaveformType;

typedef struct {
    float frequency;
    WaveformType waveform;
    float amplitude;
    float phase;
    float phase_offset;
    float detune;
    float pan;
    float target_frequency;
    float base_frequency;    // ← Ajouter
    float base_amplitude;    // ← Ajouter
} Oscillator;


// Initialisation
void Oscillator_Init(Oscillator *osc);
void Oscillators_InitAll(Oscillator *oscillators, int count);

// Génération audio
void Oscillator_Generate(Oscillator *osc, float *buffer, int len);
void Oscillators_GenerateAll(Oscillator *oscillators, int count, float *buffer, int len);

// Calcul de la fréquence réelle avec detune
float Oscillator_GetRealFrequency(const Oscillator *osc);

#endif // OSCILLATOR_H