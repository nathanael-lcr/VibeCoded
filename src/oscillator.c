#include "oscillator.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Oscillator_Init(Oscillator *osc) {
    osc->frequency = 0.0f;
    osc->waveform = WAVE_SINE;
    osc->amplitude = 0.05f;
    osc->phase = 0.0f;
    osc->phase_offset = 0.0f;
    osc->detune = 0.0f;
    osc->pan = 0.0f;
    osc->target_frequency = 0.0f;
}

void Oscillators_InitAll(Oscillator *oscillators, int count) {
    for (int i = 0; i < count; i++) {
        Oscillator_Init(&oscillators[i]);
    }
}

float Oscillator_GetRealFrequency(const Oscillator *osc) {
    // Conversion des cents en ratio de fréquence
    // 1 cent = 1/100 de demi-ton
    // 1 demi-ton = 2^(1/12)
    // donc 1 cent = 2^(1/1200)
    float detune_ratio = powf(2.0f, osc->detune / 1200.0f);
    return osc->frequency * detune_ratio;
}

static float GenerateWaveform(WaveformType type, float phase) {
    switch (type) {
        case WAVE_SINE:
            return sinf(phase);

        case WAVE_SQUARE:
            return (phase < M_PI) ? 1.0f : -1.0f;

        case WAVE_TRIANGLE: {
            float normalized = phase / (2.0f * M_PI);
            if (normalized < 0.25f)
                return 4.0f * normalized;
            else if (normalized < 0.75f)
                return 2.0f - 4.0f * normalized;
            else
                return -4.0f + 4.0f * normalized;
        }

        case WAVE_SAWTOOTH:
            return 2.0f * (phase / (2.0f * M_PI)) - 1.0f;

        case WAVE_NOISE:
            return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

        default:
            return 0.0f;
    }
}

void Oscillator_Generate(Oscillator *osc, float *buffer, int len) {
    float phase = osc->phase;
    float freq = osc->target_frequency;

    // Convertir phase_offset de degrés en radians
    float phase_offset_rad = osc->phase_offset * M_PI / 180.0f;

    for (int i = 0; i < len; i++) {
        // Lissage vers target frequency
        freq += (Oscillator_GetRealFrequency(osc) - freq) * 0.05f;

        // Calcul de l'incrément de phase
        float phase_inc = 2.0f * M_PI * freq / SAMPLE_RATE;

        // Génération de l'onde avec phase offset
        float sample = GenerateWaveform(osc->waveform, phase + phase_offset_rad);
        buffer[i] += sample * osc->amplitude;

        // Mise à jour de la phase
        phase += phase_inc;
        if (phase >= 2.0f * M_PI)
            phase -= 2.0f * M_PI;
    }

    osc->phase = phase;
    osc->target_frequency = freq;
}

void Oscillators_GenerateAll(Oscillator *oscillators, int count, float *buffer, int len) {
    // Initialiser le buffer à zéro
    for (int i = 0; i < len; i++) {
        buffer[i] = 0.0f;
    }

    // Additionner tous les oscillateurs
    for (int i = 0; i < count; i++) {
        if (oscillators[i].amplitude > 0.0f) {
            Oscillator_Generate(&oscillators[i], buffer, len);
        }
    }
}