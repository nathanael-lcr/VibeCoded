#ifndef FILTER_H
#define FILTER_H


typedef enum {
    FILTER_LOWPASS,
    FILTER_HIGHPASS,
    FILTER_BANDPASS,
    FILTER_NOTCH
} FilterType;

typedef struct {
    FilterType type;
    float cutoff;           // Fréquence de coupure (Hz)
    float resonance;        // Q factor (0.1 - 10.0)
    float z1, z2;           // États du filtre (biquad)
    int enabled;
} Filter;

void Filter_Init(Filter *filter);
void Filter_Process(Filter *filter, float *buffer, int len);
void Filter_UpdateCoefficients(Filter *filter);

#endif // FILTER_H