#include "lfo.h"
#include "filter.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void LFO_Init(LFO *lfo) {
    lfo->frequency = 1.0f;
    lfo->waveform = WAVE_SINE;
    lfo->depth = 0.5f;
    lfo->amount = 0.0f;
    lfo->phase = 0.0f;
    lfo->target_type = LFO_TARGET_NONE;
    lfo->target_index = 0;
    lfo->enabled = 0;
}

void LFOs_InitAll(LFO *lfos, int count) {
    for (int i = 0; i < count; i++) {
        LFO_Init(&lfos[i]);
    }
}

static float GenerateLFOWaveform(WaveformType type, float phase) {
    float value = 0.0f;

    switch (type) {
        case WAVE_SINE:
            value = sinf(phase);
            break;

        case WAVE_SQUARE:
            value = (phase < M_PI) ? 1.0f : -1.0f;
            break;

        case WAVE_TRIANGLE: {
            float normalized = phase / (2.0f * M_PI);
            if (normalized < 0.25f)
                value = 4.0f * normalized;
            else if (normalized < 0.75f)
                value = 2.0f - 4.0f * normalized;
            else
                value = -4.0f + 4.0f * normalized;
            break;
        }

        case WAVE_SAWTOOTH:
            value = 2.0f * (phase / (2.0f * M_PI)) - 1.0f;
            break;

        case WAVE_NOISE:
            value = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            break;

        default:
            value = 0.0f;
    }

    return value;
}

float LFO_Generate(LFO *lfo) {
    if (!lfo->enabled) return 0.0f;

    float value = GenerateLFOWaveform(lfo->waveform, lfo->phase);
    value *= lfo->depth;

    float phase_inc = 2.0f * M_PI * lfo->frequency / SAMPLE_RATE;
    lfo->phase += phase_inc;
    if (lfo->phase >= 2.0f * M_PI) {
        lfo->phase -= 2.0f * M_PI;
    }

    return value;
}

void LFO_ParseTarget(LFO *lfo, const char *target_str) {
    char clean[64] = {0};
    int j = 0;
    for (int i = 0; target_str[i] && j < 63; i++) {
        if (!isspace(target_str[i])) {
            clean[j++] = tolower(target_str[i]);
        }
    }
    clean[j] = '\0';

    if (strncmp(clean, "osc", 3) == 0) {
        int osc_num = atoi(clean + 3);
        if (osc_num < 1 || osc_num > MAX_OSCILLATORS) {
            lfo->target_type = LFO_TARGET_NONE;
            lfo->enabled = 0;
            return;
        }

        lfo->target_index = osc_num - 1;

        if (strstr(clean, "frequency")) {
            lfo->target_type = LFO_TARGET_OSC_FREQUENCY;
            lfo->enabled = 1;
        } else if (strstr(clean, "amplitude")) {
            lfo->target_type = LFO_TARGET_OSC_AMPLITUDE;
            lfo->enabled = 1;
        } else {
            lfo->target_type = LFO_TARGET_NONE;
            lfo->enabled = 0;
        }
    } else if (strstr(clean, "filter.cutoff") || strstr(clean, "filtercutoff")) {
        lfo->target_type = LFO_TARGET_FILTER_CUTOFF;
        lfo->enabled = 1;
    } else if (strstr(clean, "filter.resonance") || strstr(clean, "filterresonance")) {
        lfo->target_type = LFO_TARGET_FILTER_RESONANCE;
        lfo->enabled = 1;
    } else {
        lfo->target_type = LFO_TARGET_NONE;
        lfo->enabled = 0;
    }
}

void LFOs_ApplyToOscillators(LFO *lfos, int lfo_count, Oscillator *oscillators, int osc_count) {
    for (int i = 0; i < lfo_count; i++) {
        LFO *lfo = &lfos[i];

        if (!lfo->enabled || lfo->target_type == LFO_TARGET_NONE) {
            continue;
        }

        float lfo_value = LFO_Generate(lfo);

        switch (lfo->target_type) {
            case LFO_TARGET_OSC_FREQUENCY:
                if (lfo->target_index >= 0 && lfo->target_index < osc_count) {
                    oscillators[lfo->target_index].frequency += lfo_value * lfo->amount;
                    if (oscillators[lfo->target_index].frequency < 20.0f) {
                        oscillators[lfo->target_index].frequency = 20.0f;
                    }
                }
                break;

            case LFO_TARGET_OSC_AMPLITUDE:
                if (lfo->target_index >= 0 && lfo->target_index < osc_count) {
                    float amp_mod = lfo_value * lfo->amount;
                    oscillators[lfo->target_index].amplitude += amp_mod;

                    if (oscillators[lfo->target_index].amplitude < 0.0f) {
                        oscillators[lfo->target_index].amplitude = 0.0f;
                    }
                    if (oscillators[lfo->target_index].amplitude > 1.0f) {
                        oscillators[lfo->target_index].amplitude = 1.0f;
                    }
                }
                break;

            default:
                break;
        }
    }
}

void LFOs_ApplyToFilter(LFO *lfos, int lfo_count, Filter *filter) {
    for (int i = 0; i < lfo_count; i++) {
        LFO *lfo = &lfos[i];

        if (!lfo->enabled) continue;

        float lfo_value = LFO_Generate(lfo);

        switch (lfo->target_type) {
            case LFO_TARGET_FILTER_CUTOFF:
                filter->cutoff += lfo_value * lfo->amount;
                // Limiter la fréquence de coupure
                if (filter->cutoff < 20.0f) {
                    filter->cutoff = 20.0f;
                }
                if (filter->cutoff > SAMPLE_RATE / 2.0f) {
                    filter->cutoff = SAMPLE_RATE / 2.0f;
                }
                break;

            case LFO_TARGET_FILTER_RESONANCE:
                filter->resonance += lfo_value * lfo->amount * 0.1f;
                // Limiter la résonance
                if (filter->resonance < 0.1f) {
                    filter->resonance = 0.1f;
                }
                if (filter->resonance > 10.0f) {
                    filter->resonance = 10.0f;
                }
                break;

            default:
                break;
        }
    }
}