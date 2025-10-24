#include <stdio.h>
#include "textarea.h"
#include "waveform.h"
#include "oscillator.h"
#include "lfo.h"
#include "filter.h"
#include "command_parser.h"
#include "project.h"

int main(void) {
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *win = SDL_CreateWindow("Modular SDL3 Synth", 800, 450, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *ren = SDL_CreateRenderer(win, nullptr);

    SDL_AudioSpec spec = {.freq = SAMPLE_RATE, .format = SDL_AUDIO_F32, .channels = 1};
    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, NULL);
    SDL_ResumeAudioStreamDevice(stream);

    Oscillator oscillators[MAX_OSCILLATORS];
    Oscillators_InitAll(oscillators, MAX_OSCILLATORS);

    LFO lfos[MAX_LFOS];
    LFOs_InitAll(lfos, MAX_LFOS);

    Filter filter;
    Filter_Init(&filter);

    Waveform wave = {0};
    float master_volume = 1.0f;

    ProjectSettings project = {"Untitled", 120.0f, SAMPLE_RATE};

    TextArea textarea;
    if (TextArea_Init(&textarea, "C:/Windows/Fonts/consola.ttf", 18) < 0) {
        SDL_Log("Failed to init textarea");
        return 1;
    }

    SDL_StartTextInput(win);
    char text[4096];

    SDL_Event e;
    int running = 1;
    int debug_counter = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT ||
                (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_DOLLAR))
                running = 0;

            TextArea_HandleEvent(&textarea, &e);
        }

        // Parser les commandes en continu
        TextArea_GetText(&textarea, text, sizeof(text));
        ParseCommands(text, oscillators, lfos, &filter, &master_volume, &project);

        // Debug périodique
        if (debug_counter++ % 300 == 0) {
            for (int i = 0; i < MAX_LFOS; i++) {
                if (lfos[i].enabled) {
                    SDL_Log("LFO%d actif: freq=%.1fHz, target=%d, amount=%.1f",
                            i+1, lfos[i].frequency, lfos[i].target_type, lfos[i].amount);
                }
            }
        }

        int queued = SDL_GetAudioStreamQueued(stream);
        int wanted = SAMPLE_RATE / 4;
        while (queued < wanted * sizeof(float)) {
            float buffer[BUFFER_SIZE];

            // Sauvegarder les valeurs originales
            float osc_frequencies[MAX_OSCILLATORS];
            float osc_amplitudes[MAX_OSCILLATORS];
            for (int i = 0; i < MAX_OSCILLATORS; i++) {
                osc_frequencies[i] = oscillators[i].frequency;
                osc_amplitudes[i] = oscillators[i].amplitude;
            }
            float filter_cutoff = filter.cutoff;
            float filter_resonance = filter.resonance;

            // Appliquer les LFOs
            for (int lfo_idx = 0; lfo_idx < MAX_LFOS; lfo_idx++) {
                LFO *lfo = &lfos[lfo_idx];
                if (!lfo->enabled) continue;

                float lfo_value = LFO_Generate(lfo);

                switch (lfo->target_type) {
                    case LFO_TARGET_OSC_FREQUENCY:
                        if (lfo->target_index >= 0 && lfo->target_index < MAX_OSCILLATORS) {
                            oscillators[lfo->target_index].frequency += lfo_value * lfo->amount;
                            if (oscillators[lfo->target_index].frequency < 20.0f) {
                                oscillators[lfo->target_index].frequency = 20.0f;
                            }
                        }
                        break;

                    case LFO_TARGET_OSC_AMPLITUDE:
                        if (lfo->target_index >= 0 && lfo->target_index < MAX_OSCILLATORS) {
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

                    case LFO_TARGET_FILTER_CUTOFF:
                        filter.cutoff += lfo_value * lfo->amount;
                        if (filter.cutoff < 20.0f) filter.cutoff = 20.0f;
                        if (filter.cutoff > SAMPLE_RATE / 2.0f) filter.cutoff = SAMPLE_RATE / 2.0f;
                        break;

                    case LFO_TARGET_FILTER_RESONANCE:
                        filter.resonance += lfo_value * lfo->amount * 0.1f;
                        if (filter.resonance < 0.1f) filter.resonance = 0.1f;
                        if (filter.resonance > 10.0f) filter.resonance = 10.0f;
                        break;

                    default:
                        break;
                }
            }

            // Générer l'audio
            Oscillators_GenerateAll(oscillators, MAX_OSCILLATORS, buffer, BUFFER_SIZE);

            // Appliquer le filtre
            if (filter.enabled) {
                Filter_Process(&filter, buffer, BUFFER_SIZE);
            }

            // Appliquer le volume master
            for (int i = 0; i < BUFFER_SIZE; i++)
                buffer[i] *= master_volume;

            // Restaurer les valeurs originales
            for (int i = 0; i < MAX_OSCILLATORS; i++) {
                oscillators[i].frequency = osc_frequencies[i];
                oscillators[i].amplitude = osc_amplitudes[i];
            }
            filter.cutoff = filter_cutoff;
            filter.resonance = filter_resonance;

            Waveform_Update(&wave, buffer);
            SDL_PutAudioStreamData(stream, buffer, BUFFER_SIZE * sizeof(float));
            queued += BUFFER_SIZE * sizeof(float);
        }

        int w, h;
        SDL_GetWindowSize(win, &w, &h);
        SDL_SetRenderDrawColor(ren, 20, 20, 30, 255);
        SDL_RenderClear(ren);

        // Zone de visualisation (haut de l'écran)
        int viz_height = h * 0.25f;

        // Dessiner la waveform avec les LFOs superposés
        Waveform_DrawWithLFOs(ren, &wave, lfos, MAX_LFOS, w, viz_height);

        // Dessiner le textarea (bas de l'écran)
        TextArea_Draw(ren, &textarea, w, h*1.5);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_StopTextInput(win);
    TextArea_Cleanup(&textarea);
    SDL_DestroyAudioStream(stream);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}