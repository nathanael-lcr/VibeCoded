#include "command_parser.h"
#include "lfo.h"
#include "filter.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

// Fonction pour parser les commandes LFO
// Syntaxe: lfo1.freq = 2.0
//          lfo1.wave = sine
//          lfo1.depth = 0.8
//          lfo1.amount = 100
//          lfo1.target = osc1.frequency
//          lfo1.enabled = 1

static void ParseLFOCommand(char *line, LFO *lfos) {
    char *lfo_str = strstr(line, "lfo");
    if (!lfo_str) return;

    // Extraire le numéro du LFO
    int lfo_num = atoi(lfo_str + 3);
    if (lfo_num < 1 || lfo_num > MAX_LFOS) return;
    
    LFO *lfo = &lfos[lfo_num - 1];

    // Parser les différentes propriétés
    if (strstr(line, ".frequency")) {
        char *eq = strchr(line, '=');
        if (eq) {
            lfo->frequency = atof(eq + 1);
        }
    }
    else if (strstr(line, ".waveform")) {
        char *eq = strchr(line, '=');
        if (eq) {
            char wave[32];
            sscanf(eq + 1, "%31s", wave);
            
            // Convertir en minuscules
            for (int i = 0; wave[i]; i++) {
                wave[i] = tolower(wave[i]);
            }
            
            if (strcmp(wave, "sine") == 0) lfo->waveform = WAVE_SINE;
            else if (strcmp(wave, "square") == 0) lfo->waveform = WAVE_SQUARE;
            else if (strcmp(wave, "triangle") == 0) lfo->waveform = WAVE_TRIANGLE;
            else if (strcmp(wave, "sawtooth") == 0 || strcmp(wave, "saw") == 0) 
                lfo->waveform = WAVE_SAWTOOTH;
            else if (strcmp(wave, "noise") == 0) lfo->waveform = WAVE_NOISE;
        }
    }
    else if (strstr(line, ".depth")) {
        char *eq = strchr(line, '=');
        if (eq) {
            lfo->depth = atof(eq + 1);
            if (lfo->depth < 0.0f) lfo->depth = 0.0f;
            if (lfo->depth > 1.0f) lfo->depth = 1.0f;
        }
    }
    else if (strstr(line, ".amount")) {
        char *eq = strchr(line, '=');
        if (eq) {
            lfo->amount = atof(eq + 1);
        }
    }
    else if (strstr(line, ".target")) {
        char *eq = strchr(line, '=');
        if (eq) {
            char target[64];
            sscanf(eq + 1, "%63s", target);
            LFO_ParseTarget(lfo, target);
        }
    }
    else if (strstr(line, ".enabled")) {
        char *eq = strchr(line, '=');
        if (eq) {
            lfo->enabled = atoi(eq + 1);
        }
    }
}

// Fonction pour parser les commandes de filtre
// Syntaxe: filter.type = lowpass
//          filter.cutoff = 1000
//          filter.resonance = 2.0
//          filter.enabled = 1

static void ParseFilterCommand(char *line, Filter *filter) {
    if (!strstr(line, "filter")) return;

    if (strstr(line, ".type")) {
        char *eq = strchr(line, '=');
        if (eq) {
            char type[32];
            sscanf(eq + 1, "%31s", type);
            
            for (int i = 0; type[i]; i++) {
                type[i] = tolower(type[i]);
            }
            
            if (strcmp(type, "lowpass") == 0 || strcmp(type, "lp") == 0) 
                filter->type = FILTER_LOWPASS;
            else if (strcmp(type, "highpass") == 0 || strcmp(type, "hp") == 0) 
                filter->type = FILTER_HIGHPASS;
            else if (strcmp(type, "bandpass") == 0 || strcmp(type, "bp") == 0) 
                filter->type = FILTER_BANDPASS;
            else if (strcmp(type, "notch") == 0) 
                filter->type = FILTER_NOTCH;
        }
    }
    else if (strstr(line, ".cutoff")) {
        char *eq = strchr(line, '=');
        if (eq) {
            filter->cutoff = atof(eq + 1);
            if (filter->cutoff < 20.0f) filter->cutoff = 20.0f;
            if (filter->cutoff > SAMPLE_RATE / 2.0f) 
                filter->cutoff = SAMPLE_RATE / 2.0f;
        }
    }
    else if (strstr(line, ".resonance") || strstr(line, ".q")) {
        char *eq = strchr(line, '=');
        if (eq) {
            filter->resonance = atof(eq + 1);
            if (filter->resonance < 0.1f) filter->resonance = 0.1f;
            if (filter->resonance > 10.0f) filter->resonance = 10.0f;
        }
    }
    else if (strstr(line, ".enabled")) {
        char *eq = strchr(line, '=');
        if (eq) {
            filter->enabled = atoi(eq + 1);
        }
    }
}

// Parser pour les oscillateurs
static void ParseOscCommand(char *line, Oscillator *oscillators) {
    if (!strstr(line, "osc")) return;

    int osc_num = atoi(strstr(line, "osc") + 3);
    if (osc_num < 1 || osc_num > MAX_OSCILLATORS) return;
    
    Oscillator *osc = &oscillators[osc_num - 1];

    if (strstr(line, ".freq")) {
        char *eq = strchr(line, '=');
        if (eq) {
            osc->frequency = atof(eq + 1);
        }
    }
    else if (strstr(line, ".wave")) {
        char *eq = strchr(line, '=');
        if (eq) {
            char wave[32];
            sscanf(eq + 1, "%31s", wave);
            
            for (int i = 0; wave[i]; i++) {
                wave[i] = tolower(wave[i]);
            }
            
            if (strcmp(wave, "sine") == 0) osc->waveform = WAVE_SINE;
            else if (strcmp(wave, "square") == 0) osc->waveform = WAVE_SQUARE;
            else if (strcmp(wave, "triangle") == 0) osc->waveform = WAVE_TRIANGLE;
            else if (strcmp(wave, "sawtooth") == 0 || strcmp(wave, "saw") == 0) 
                osc->waveform = WAVE_SAWTOOTH;
            else if (strcmp(wave, "noise") == 0) osc->waveform = WAVE_NOISE;
        }
    }
    else if (strstr(line, ".amp")) {
        char *eq = strchr(line, '=');
        if (eq) {
            osc->amplitude = atof(eq + 1);
            if (osc->amplitude < 0.0f) osc->amplitude = 0.0f;
            if (osc->amplitude > 1.0f) osc->amplitude = 1.0f;
        }
    }
    else if (strstr(line, ".detune")) {
        char *eq = strchr(line, '=');
        if (eq) {
            osc->detune = atof(eq + 1);
        }
    }
    else if (strstr(line, ".phase")) {
        char *eq = strchr(line, '=');
        if (eq) {
            osc->phase_offset = atof(eq + 1);
        }
    }
    else if (strstr(line, ".pan")) {
        char *eq = strchr(line, '=');
        if (eq) {
            osc->pan = atof(eq + 1);
            if (osc->pan < -1.0f) osc->pan = -1.0f;
            if (osc->pan > 1.0f) osc->pan = 1.0f;
        }
    }
}

// Fonction principale ParseCommands mise à jour
void ParseCommands(char *text, Oscillator *oscillators, LFO *lfos, 
                   Filter *filter, float *master_volume, ProjectSettings *project) {
    char text_copy[4096];
    strncpy(text_copy, text, sizeof(text_copy) - 1);
    text_copy[sizeof(text_copy) - 1] = '\0';
    
    char *line = strtok(text_copy, "\n");
    
    while (line != NULL) {
        // Ignorer les lignes vides et les commentaires
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '\0' || *line == '#' || *line == '/') {
            line = strtok(NULL, "\n");
            continue;
        }

        // Parser les commandes d'oscillateurs
        if (strstr(line, "osc") && !strstr(line, "lfo")) {
            ParseOscCommand(line, oscillators);
        }
        // Parser les commandes LFO
        else if (strstr(line, "lfo")) {
            ParseLFOCommand(line, lfos);
        }
        // Parser les commandes de filtre
        else if (strstr(line, "filter")) {
            ParseFilterCommand(line, filter);
        }
        // Parser le volume master
        else if (strstr(line, "volume")) {
            char *eq = strchr(line, '=');
            if (eq) {
                *master_volume = atof(eq + 1);
                if (*master_volume < 0.0f) *master_volume = 0.0f;
                if (*master_volume > 2.0f) *master_volume = 2.0f;
            }
        }

        line = strtok(NULL, "\n");
    }
}