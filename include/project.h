#ifndef PROJECT_H
#define PROJECT_H

#define PROJECT_NAME_MAX 64

typedef struct {
    char name[PROJECT_NAME_MAX];
    float tempo;       // en BPM
    int sample_rate;   // en Hz
} ProjectSettings;

#endif
