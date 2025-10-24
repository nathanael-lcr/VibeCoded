#ifndef LFO_H
#define LFO_H

#include "filter.h"
#include "oscillator.h"

#define MAX_LFOS 6

typedef enum {
    LFO_TARGET_NONE,
    LFO_TARGET_OSC_FREQUENCY,
    LFO_TARGET_OSC_AMPLITUDE,
    LFO_TARGET_FILTER_CUTOFF,
    LFO_TARGET_FILTER_RESONANCE
} LFOTargetType;

typedef struct {
    float frequency;        // Fréquence du LFO en Hz
    WaveformType waveform;  // Type d'onde
    float depth;           // Profondeur (0.0 - 1.0)
    float amount;          // Quantité de modulation
    float phase;           // Phase actuelle
    LFOTargetType target_type;
    int target_index;      // Index de l'oscillateur cible (si applicable)
    int enabled;
} LFO;

void LFO_Init(LFO *lfo);
void LFOs_InitAll(LFO *lfos, int count);
float LFO_Generate(LFO *lfo);
void LFO_ParseTarget(LFO *lfo, const char *target_str);
void LFOs_ApplyToOscillators(LFO *lfos, int lfo_count, Oscillator *oscillators, int osc_count);
void LFOs_ApplyToFilter(LFO *lfos, int lfo_count, Filter *filter);

#endif // LFO_H