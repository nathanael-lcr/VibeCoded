#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "filter.h"
#include "lfo.h"
#include "oscillator.h"
#include "project.h"

// Parse toutes les commandes du texte
void ParseCommands(char *text, Oscillator *oscillators, LFO     *lfos,
                   Filter *filter, float *master_volume, ProjectSettings *project);

// Parse une seule ligne de commande
void ParseLine(const char *text, Oscillator *oscillators, float *master_volume, ProjectSettings *project);

#endif // COMMAND_PARSER_H