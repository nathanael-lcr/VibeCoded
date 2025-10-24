#ifndef TEXTAREA_H
#define TEXTAREA_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define TEXT_MAX_LENGTH 4096
#define MAX_SUGGESTIONS 10

typedef struct {
    char text[TEXT_MAX_LENGTH];
    int length;
    int cursor_pos;  // Position du curseur dans le texte
    Uint64 last_blink;
    int cursor_visible;
    TTF_Font *font;

    // Autocomplétion
    char suggestions[MAX_SUGGESTIONS][64];
    int suggestion_count;
    int selected_suggestion;
    int show_suggestions;
} TextArea;

int TextArea_Init(TextArea *ta, const char *font_path, int font_size);
void TextArea_Cleanup(TextArea *ta);
void TextArea_HandleEvent(TextArea *ta, SDL_Event *e);
void TextArea_GetText(TextArea *ta, char *out, int max_len);
void TextArea_Draw(SDL_Renderer *ren, TextArea *ta, int w, int h);

#endif