#include "synth.h"
#include <math.h>

void Synth_Init(Synth *synth, float freq) {
    synth->phase = 0.0f;
    synth->freq = freq;
    synth->target_freq = freq;
}

void Synth_SetFrequency(Synth *synth, float freq) {
    synth->target_freq = freq;
}

void Synth_Generate(Synth *synth, float *buffer, int len) {
    float phase = synth->phase;
    float freq = synth->freq;

    for (int i = 0; i < len; i++) {
        // lissage vers target
        freq += (synth->target_freq - freq) * 0.05f;
        float phase_inc = 2.0f * (float)M_PI * freq / SAMPLE_RATE;
        buffer[i] = sinf(phase) * synth->amplitude;
        phase += phase_inc;
        if (phase >= 2.0f * (float)M_PI)
            phase -= 2.0f * (float)M_PI;
    }

    synth->phase = phase;
    synth->freq = freq;
}
