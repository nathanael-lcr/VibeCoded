#include "waveform.h"
#include "lfo.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Waveform_Update(Waveform *wave, float *buffer) {
    SDL_LockSpinlock(&wave->lock);
    for (int i = 0; i < BUFFER_SIZE; i++)
        wave->data[i] = buffer[i];
    SDL_UnlockSpinlock(&wave->lock);
}

void Waveform_Draw(SDL_Renderer *ren, Waveform *wave, int w, int h) {
    // Dessiner la ligne centrale (référence 0)
    SDL_SetRenderDrawColor(ren, 60, 60, 70, 255);
    SDL_RenderLine(ren, 0, h / 2, w, h / 2);

    // Dessiner la waveform principale en blanc brillant
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_LockSpinlock(&wave->lock);
    for (int x = 0; x < w - 1; x++) {
        int i1 = (x * BUFFER_SIZE) / w;
        int i2 = ((x + 1) * BUFFER_SIZE) / w;
        float s1 = wave->data[i1];
        float s2 = wave->data[i2];
        int y1 = h / 2 - (int)(s1 * (h / 2 - 20));
        int y2 = h / 2 - (int)(s2 * (h / 2 - 20));
        SDL_RenderLine(ren, (float)x, (float)y1, (float)(x + 1), (float)y2);
    }
    SDL_UnlockSpinlock(&wave->lock);
}

static const char* GetWaveformName(WaveformType type) {
    switch (type) {
        case WAVE_SINE: return "SINE";
        case WAVE_SQUARE: return "SQUARE";
        case WAVE_TRIANGLE: return "TRI";
        case WAVE_SAWTOOTH: return "SAW";
        case WAVE_NOISE: return "NOISE";
        default: return "?";
    }
}

static const char* GetTargetName(LFOTargetType type, int index) {
    static char buffer[32];
    switch (type) {
        case LFO_TARGET_OSC_FREQUENCY:
            snprintf(buffer, sizeof(buffer), "OSC%d.FREQ", index + 1);
            return buffer;
        case LFO_TARGET_OSC_AMPLITUDE:
            snprintf(buffer, sizeof(buffer), "OSC%d.AMP", index + 1);
            return buffer;
        case LFO_TARGET_FILTER_CUTOFF:
            return "FILTER.CUTOFF";
        case LFO_TARGET_FILTER_RESONANCE:
            return "FILTER.RES";
        default:
            return "NONE";
    }
}

void Waveform_DrawLFO(SDL_Renderer *ren, LFO *lfo, int w, int h, SDL_Color color) {
    if (!lfo || !lfo->enabled) return;

    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);

    float saved_phase = lfo->phase;

    // Dessiner le LFO sur toute la largeur (plusieurs périodes selon la fréquence)
    int num_periods = (lfo->frequency < 1.0f) ? 1 : (int)(lfo->frequency * 2);
    if (num_periods > 8) num_periods = 8;

    for (int x = 0; x < w - 1; x++) {
        float t1 = (float)x / w;
        float t2 = (float)(x + 1) / w;

        float phase1 = t1 * 2.0f * M_PI * num_periods;
        float phase2 = t2 * 2.0f * M_PI * num_periods;

        float value1 = 0.0f, value2 = 0.0f;

        switch (lfo->waveform) {
            case WAVE_SINE:
                value1 = sinf(phase1);
                value2 = sinf(phase2);
                break;

            case WAVE_SQUARE:
                value1 = (fmodf(phase1, 2.0f * M_PI) < M_PI) ? 1.0f : -1.0f;
                value2 = (fmodf(phase2, 2.0f * M_PI) < M_PI) ? 1.0f : -1.0f;
                break;

            case WAVE_TRIANGLE: {
                float norm1 = fmodf(phase1 / (2.0f * M_PI), 1.0f);
                float norm2 = fmodf(phase2 / (2.0f * M_PI), 1.0f);

                if (norm1 < 0.25f)
                    value1 = 4.0f * norm1;
                else if (norm1 < 0.75f)
                    value1 = 2.0f - 4.0f * norm1;
                else
                    value1 = -4.0f + 4.0f * norm1;

                if (norm2 < 0.25f)
                    value2 = 4.0f * norm2;
                else if (norm2 < 0.75f)
                    value2 = 2.0f - 4.0f * norm2;
                else
                    value2 = -4.0f + 4.0f * norm2;
                break;
            }

            case WAVE_SAWTOOTH:
                value1 = 2.0f * fmodf(phase1 / (2.0f * M_PI), 1.0f) - 1.0f;
                value2 = 2.0f * fmodf(phase2 / (2.0f * M_PI), 1.0f) - 1.0f;
                break;

            default:
                break;
        }

        value1 *= lfo->depth;
        value2 *= lfo->depth;

        // Réduire l'amplitude pour distinction
        int y1 = h / 2 - (int)(value1 * (h / 5));
        int y2 = h / 2 - (int)(value2 * (h / 5));

        SDL_RenderLine(ren, (float)x, (float)y1, (float)(x + 1), (float)y2);
    }

    lfo->phase = saved_phase;
}

void Waveform_DrawLFOsWithLegend(SDL_Renderer *ren, LFO *lfos, int lfo_count,
                                  int w, int h, TTF_Font *font) {
    SDL_Color colors[] = {
        {255, 100, 100, 200},  // Rouge
        {100, 255, 100, 200},  // Vert
        {100, 150, 255, 200},  // Bleu
        {255, 255, 100, 200},  // Jaune
        {255, 100, 255, 200},  // Magenta
        {100, 255, 255, 200}   // Cyan
    };

    int num_colors = sizeof(colors) / sizeof(colors[0]);
    int legend_y = 5;

    for (int i = 0; i < lfo_count; i++) {
        if (lfos[i].enabled) {
            SDL_Color color = colors[i % num_colors];

            // Dessiner le LFO
            Waveform_DrawLFO(ren, &lfos[i], w, h, color);

            // Dessiner la légende (si font disponible)
            if (font) {
                char legend[128];
                snprintf(legend, sizeof(legend), "LFO%d: %.1fHz %s -> %s (%.0f)",
                        i + 1,
                        lfos[i].frequency,
                        GetWaveformName(lfos[i].waveform),
                        GetTargetName(lfos[i].target_type, lfos[i].target_index),
                        lfos[i].amount);

                SDL_Surface *surf = TTF_RenderText_Blended(font, legend, 0, color);
                if (surf) {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
                    SDL_FRect dest = {5, legend_y, surf->w, surf->h};
                    SDL_RenderTexture(ren, tex, NULL, &dest);
                    SDL_DestroyTexture(tex);
                    SDL_DestroySurface(surf);
                    legend_y += surf->h + 2;
                }
            }
        }
    }
}

void Waveform_DrawWithLFOs(SDL_Renderer *ren, Waveform *wave, LFO *lfos,
                           int lfo_count, int w, int h) {
    // Dessiner d'abord la waveform principale
    Waveform_Draw(ren, wave, w, h);

    // Puis superposer les LFOs
    SDL_Color colors[] = {
        {255, 100, 100, 200},
        {100, 255, 100, 200},
        {100, 150, 255, 200},
        {255, 255, 100, 200},
        {255, 100, 255, 200},
        {100, 255, 255, 200}
    };

    int num_colors = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < lfo_count; i++) {
        if (lfos[i].enabled) {
            SDL_Color color = colors[i % num_colors];
            Waveform_DrawLFO(ren, &lfos[i], w, h, color);
        }
    }
}