#include "filter.h"
#include <math.h>

#include "oscillator.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Filter_Init(Filter *filter) {
    filter->type = FILTER_LOWPASS;
    filter->cutoff = 1000.0f;
    filter->resonance = 1.0f;
    filter->z1 = 0.0f;
    filter->z2 = 0.0f;
    filter->enabled = 0;
}

void Filter_Process(Filter *filter, float *buffer, int len) {
    if (!filter->enabled) return;

    // Coefficients du filtre biquad
    float omega = 2.0f * M_PI * filter->cutoff / SAMPLE_RATE;
    float sin_omega = sinf(omega);
    float cos_omega = cosf(omega);
    float alpha = sin_omega / (2.0f * filter->resonance);

    float b0, b1, b2, a0, a1, a2;

    switch (filter->type) {
        case FILTER_LOWPASS:
            b0 = (1.0f - cos_omega) / 2.0f;
            b1 = 1.0f - cos_omega;
            b2 = (1.0f - cos_omega) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case FILTER_HIGHPASS:
            b0 = (1.0f + cos_omega) / 2.0f;
            b1 = -(1.0f + cos_omega);
            b2 = (1.0f + cos_omega) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case FILTER_BANDPASS:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case FILTER_NOTCH:
            b0 = 1.0f;
            b1 = -2.0f * cos_omega;
            b2 = 1.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        default:
            return;
    }

    // Normaliser
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    // Appliquer le filtre
    float z1 = filter->z1;
    float z2 = filter->z2;

    for (int i = 0; i < len; i++) {
        float input = buffer[i];
        float output = b0 * input + z1;
        z1 = b1 * input - a1 * output + z2;
        z2 = b2 * input - a2 * output;
        buffer[i] = output;
    }

    filter->z1 = z1;
    filter->z2 = z2;
}