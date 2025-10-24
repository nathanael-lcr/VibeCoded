#include "textarea.h"
#include <string.h>
#include <stdio.h>

// Liste complète des suggestions
static const char *all_suggestions[] = {
    // Oscillateurs
    "osc1.frequency",
    "osc1.freq",
    "osc1.waveform",
    "osc1.wave",
    "osc1.amplitude",
    "osc1.amp",
    "osc1.phase",
    "osc1.detune",
    "osc1.pan",
    "osc2.frequency",
    "osc2.freq",
    "osc2.waveform",
    "osc2.wave",
    "osc2.amplitude",
    "osc2.amp",
    "osc2.phase",
    "osc2.detune",
    "osc2.pan",
    "osc3.frequency",
    "osc3.freq",
    "osc3.waveform",
    "osc3.wave",
    "osc3.amplitude",
    "osc3.amp",
    "osc3.phase",
    "osc3.detune",
    "osc3.pan",
    "osc4.frequency",
    "osc4.freq",
    "osc4.waveform",
    "osc4.wave",
    "osc4.amplitude",
    "osc4.amp",
    "osc4.phase",
    "osc4.detune",
    "osc4.pan",
    // LFOs
    "lfo1.frequency",
    "lfo1.waveform",
    "lfo1.depth",
    "lfo1.amount",
    "lfo1.target",
    "lfo1.enabled",
    "lfo2.frequency",
    "lfo2.waveform",
    "lfo2.depth",
    "lfo2.amount",
    "lfo2.target",
    "lfo2.enabled",
    "lfo3.frequency",
    "lfo3.waveform",
    "lfo3.depth",
    "lfo3.amount",
    "lfo3.target",
    "lfo3.enabled",
    "lfo4.frequency",
    "lfo4.waveform",
    "lfo4.depth",
    "lfo4.amount",
    "lfo4.target",
    "lfo4.enabled",
    "lfo5.frequency",
    "lfo5.waveform",
    "lfo5.depth",
    "lfo5.amount",
    "lfo5.target",
    "lfo5.enabled",
    "lfo6.frequency",
    "lfo6.waveform",
    "lfo6.depth",
    "lfo6.amount",
    "lfo6.target",
    "lfo6.enabled",
    // Filtre
    "filter.type",
    "filter.cutoff",
    "filter.resonance",
    "filter.q",
    "filter.enabled",
    // Master
    "master.volume",
    "volume",
    NULL
};

static const char *waveform_suggestions[] = {
    "sine",
    "square",
    "triangle",
    "sawtooth",
    "saw",
    "noise",
    NULL
};

static const char *filter_type_suggestions[] = {
    "lowpass",
    "lp",
    "highpass",
    "hp",
    "bandpass",
    "bp",
    "notch",
    NULL
};

static const char *target_suggestions[] = {
    "osc1.frequency",
    "osc1.amplitude",
    "osc2.frequency",
    "osc2.amplitude",
    "osc3.frequency",
    "osc3.amplitude",
    "osc4.frequency",
    "osc4.amplitude",
    "filter.cutoff",
    "filter.resonance",
    NULL
};

int TextArea_Init(TextArea *ta, const char *font_path, int font_size) {
    memset(ta->text, 0, sizeof(ta->text));
    ta->length = 0;
    ta->cursor_pos = 0;
    ta->last_blink = SDL_GetTicks();
    ta->cursor_visible = 1;
    ta->suggestion_count = 0;
    ta->selected_suggestion = 0;
    ta->show_suggestions = 0;

    ta->font = TTF_OpenFont(font_path, font_size);
    if (!ta->font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return -1;
    }
    return 0;
}

void TextArea_Cleanup(TextArea *ta) {
    if (ta->font) {
        TTF_CloseFont(ta->font);
        ta->font = NULL;
    }
}

static int FindWordStart(TextArea *ta) {
    int pos = ta->cursor_pos - 1;
    while (pos >= 0 && ta->text[pos] != ' ' && ta->text[pos] != '\n' && ta->text[pos] != '\t') {
        pos--;
    }
    return pos + 1;
}

static void UpdateSuggestions(TextArea *ta) {
    ta->suggestion_count = 0;
    ta->selected_suggestion = 0;

    int word_start = FindWordStart(ta);
    int word_len = ta->cursor_pos - word_start;

    if (word_len == 0) {
        ta->show_suggestions = 0;
        return;
    }

    char current_word[64];
    if (word_len >= 64) word_len = 63;
    memcpy(current_word, &ta->text[word_start], word_len);
    current_word[word_len] = '\0';

    // Déterminer quel type de suggestions afficher
    const char **source = all_suggestions;

    // Vérifier si on cherche une waveform
    if (word_start >= 10) {
        char before[15];
        int check_len = word_start > 14 ? 14 : word_start;
        memcpy(before, &ta->text[word_start - check_len], check_len);
        before[check_len] = '\0';

        if (strstr(before, "wave") || strstr(before, "waveform")) {
            source = waveform_suggestions;
        } else if (strstr(before, "filter.type") || strstr(before, "type")) {
            source = filter_type_suggestions;
        } else if (strstr(before, "target")) {
            source = target_suggestions;
        }
    }

    // Chercher les suggestions qui correspondent
    for (int i = 0; source[i] != NULL && ta->suggestion_count < MAX_SUGGESTIONS; i++) {
        if (strncmp(source[i], current_word, word_len) == 0) {
            strncpy(ta->suggestions[ta->suggestion_count], source[i], 63);
            ta->suggestions[ta->suggestion_count][63] = '\0';
            ta->suggestion_count++;
        }
    }

    ta->show_suggestions = (ta->suggestion_count > 0);
}

static void InsertSuggestion(TextArea *ta) {
    if (!ta->show_suggestions || ta->suggestion_count == 0) return;

    int word_start = FindWordStart(ta);
    int word_len = ta->cursor_pos - word_start;

    if (word_len > 0) {
        memmove(&ta->text[word_start], &ta->text[ta->cursor_pos],
                ta->length - ta->cursor_pos + 1);
        ta->length -= word_len;
        ta->cursor_pos = word_start;
    }

    const char *suggestion = ta->suggestions[ta->selected_suggestion];
    int sugg_len = strlen(suggestion);

    if (ta->length + sugg_len < TEXT_MAX_LENGTH) {
        memmove(&ta->text[ta->cursor_pos + sugg_len], &ta->text[ta->cursor_pos],
                ta->length - ta->cursor_pos + 1);
        memcpy(&ta->text[ta->cursor_pos], suggestion, sugg_len);
        ta->length += sugg_len;
        ta->cursor_pos += sugg_len;

        // Ajouter " = " après le nom de propriété (sauf pour les valeurs simples)
        if (strchr(suggestion, '.') != NULL && ta->length + 3 < TEXT_MAX_LENGTH) {
            memmove(&ta->text[ta->cursor_pos + 3], &ta->text[ta->cursor_pos],
                    ta->length - ta->cursor_pos + 1);
            ta->text[ta->cursor_pos] = ' ';
            ta->text[ta->cursor_pos + 1] = '=';
            ta->text[ta->cursor_pos + 2] = ' ';
            ta->length += 3;
            ta->cursor_pos += 3;
        }
    }

    ta->show_suggestions = 0;
}

void TextArea_HandleEvent(TextArea *ta, SDL_Event *e) {
    if (e->type == SDL_EVENT_KEY_DOWN) {
        if (ta->show_suggestions) {
            if (e->key.key == SDLK_DOWN) {
                ta->selected_suggestion = (ta->selected_suggestion + 1) % ta->suggestion_count;
                return;
            } else if (e->key.key == SDLK_UP) {
                ta->selected_suggestion = (ta->selected_suggestion - 1 + ta->suggestion_count) % ta->suggestion_count;
                return;
            } else if (e->key.key == SDLK_TAB || e->key.key == SDLK_RETURN) {
                InsertSuggestion(ta);
                return;
            } else if (e->key.key == SDLK_ESCAPE) {
                ta->show_suggestions = 0;
                return;
            }
        }

        if (e->key.key == SDLK_LEFT) {
            if (ta->cursor_pos > 0) {
                ta->cursor_pos--;
                UpdateSuggestions(ta);
            }
        } else if (e->key.key == SDLK_RIGHT) {
            if (ta->cursor_pos < ta->length) {
                ta->cursor_pos++;
                UpdateSuggestions(ta);
            }
        } else if (e->key.key == SDLK_UP) {
            int line_start = ta->cursor_pos - 1;
            while (line_start > 0 && ta->text[line_start - 1] != '\n') {
                line_start--;
            }

            if (line_start > 0) {
                int prev_line_start = line_start - 2;
                while (prev_line_start > 0 && ta->text[prev_line_start - 1] != '\n') {
                    prev_line_start--;
                }

                int offset = ta->cursor_pos - line_start;
                int prev_line_len = line_start - 1 - prev_line_start;

                ta->cursor_pos = prev_line_start + (offset < prev_line_len ? offset : prev_line_len);
                UpdateSuggestions(ta);
            }
        } else if (e->key.key == SDLK_DOWN) {
            int line_start = ta->cursor_pos;
            while (line_start > 0 && ta->text[line_start - 1] != '\n') {
                line_start--;
            }

            int next_line_start = ta->cursor_pos;
            while (next_line_start < ta->length && ta->text[next_line_start] != '\n') {
                next_line_start++;
            }

            if (next_line_start < ta->length) {
                next_line_start++;

                int offset = ta->cursor_pos - line_start;
                int next_line_end = next_line_start;
                while (next_line_end < ta->length && ta->text[next_line_end] != '\n') {
                    next_line_end++;
                }

                int next_line_len = next_line_end - next_line_start;
                ta->cursor_pos = next_line_start + (offset < next_line_len ? offset : next_line_len);
                UpdateSuggestions(ta);
            }
        } else if (e->key.key == SDLK_HOME) {
            while (ta->cursor_pos > 0 && ta->text[ta->cursor_pos - 1] != '\n') {
                ta->cursor_pos--;
            }
            UpdateSuggestions(ta);
        } else if (e->key.key == SDLK_END) {
            while (ta->cursor_pos < ta->length && ta->text[ta->cursor_pos] != '\n') {
                ta->cursor_pos++;
            }
            UpdateSuggestions(ta);
        } else if (e->key.key == SDLK_BACKSPACE) {
            if (ta->cursor_pos > 0) {
                memmove(&ta->text[ta->cursor_pos - 1], &ta->text[ta->cursor_pos],
                        ta->length - ta->cursor_pos + 1);
                ta->cursor_pos--;
                ta->length--;
                UpdateSuggestions(ta);
            }
        } else if (e->key.key == SDLK_DELETE) {
            if (ta->cursor_pos < ta->length) {
                memmove(&ta->text[ta->cursor_pos], &ta->text[ta->cursor_pos + 1],
                        ta->length - ta->cursor_pos);
                ta->length--;
                UpdateSuggestions(ta);
            }
        } else if (e->key.key == SDLK_RETURN) {
            if (ta->length < TEXT_MAX_LENGTH - 1) {
                memmove(&ta->text[ta->cursor_pos + 1], &ta->text[ta->cursor_pos],
                        ta->length - ta->cursor_pos + 1);
                ta->text[ta->cursor_pos] = '\n';
                ta->cursor_pos++;
                ta->length++;
                ta->show_suggestions = 0;
            }
        }
    } else if (e->type == SDL_EVENT_TEXT_INPUT) {
        if (ta->length < TEXT_MAX_LENGTH - 1) {
            char c = e->text.text[0];
            memmove(&ta->text[ta->cursor_pos + 1], &ta->text[ta->cursor_pos],
                    ta->length - ta->cursor_pos + 1);
            ta->text[ta->cursor_pos] = c;
            ta->cursor_pos++;
            ta->length++;
            UpdateSuggestions(ta);
        }
    }
}

void TextArea_GetText(TextArea *ta, char *out, int max_len) {
    if (!ta || !out) return;

    int copy_len = ta->length < max_len - 1 ? ta->length : max_len - 1;
    memcpy(out, ta->text, copy_len);
    out[copy_len] = '\0';
}

void TextArea_Draw(SDL_Renderer *ren, TextArea *ta, int w, int h) {
    int area_height = h / 2;
    int area_y = (h - area_height) / 2;

    SDL_FRect bg = {0.0f, (float)area_y, (float)w, (float)area_height};
    SDL_SetRenderDrawColor(ren, 30, 30, 38, 255);
    SDL_RenderFillRect(ren, &bg);

    SDL_SetRenderDrawColor(ren, 80, 80, 100, 255);
    SDL_RenderRect(ren, &bg);

    if (!ta->font) {
        return;
    }

    Uint64 now = SDL_GetTicks();
    if (now - ta->last_blink > 500) {
        ta->cursor_visible = !ta->cursor_visible;
        ta->last_blink = now;
    }

    int line_height = TTF_GetFontLineSkip(ta->font);
    int gutter_width = 50;

    SDL_FRect gutter = {0.0f, (float)area_y, (float)gutter_width, (float)area_height};
    SDL_SetRenderDrawColor(ren, 30, 30, 38, 255);
    SDL_RenderFillRect(ren, &gutter);

    SDL_SetRenderDrawColor(ren, 20, 20, 30, 255);
    SDL_RenderLine(ren, (float)gutter_width, (float)area_y,
                   (float)gutter_width, (float)(area_y + area_height));

    int line_count = 1;
    for (int i = 0; i < ta->length; i++) {
        if (ta->text[i] == '\n') line_count++;
    }

    float y = (float)(area_y + 20);
    SDL_Color line_num_color = {100, 100, 120, 255};

    for (int line_num = 1; line_num <= line_count; line_num++) {
        char num_str[16];
        snprintf(num_str, sizeof(num_str), "%d", line_num);

        SDL_Surface *num_surf = TTF_RenderText_Blended(ta->font, num_str, 0, line_num_color);
        if (num_surf) {
            SDL_Texture *num_tex = SDL_CreateTextureFromSurface(ren, num_surf);
            if (num_tex) {
                float num_x = gutter_width - num_surf->w - 10.0f;
                SDL_FRect num_dst = {num_x, y, (float)num_surf->w, (float)num_surf->h};
                SDL_RenderTexture(ren, num_tex, NULL, &num_dst);
                SDL_DestroyTexture(num_tex);
            }
            SDL_DestroySurface(num_surf);
        }

        y += line_height;
    }

    char line_buffer[256];
    int line_start = 0;
    y = (float)(area_y + 20);
    float text_x = (float)(gutter_width + 10);

    SDL_Color text_color = {220, 220, 220, 255};

    int cursor_line = 0;
    int cursor_col = 0;
    int current_line = 0;
    int line_pos = 0;

    for (int i = 0; i < ta->cursor_pos; i++) {
        if (ta->text[i] == '\n') {
            current_line++;
            line_pos = 0;
        } else {
            line_pos++;
        }
    }
    cursor_line = current_line;
    cursor_col = line_pos;

    current_line = 0;
    for (int i = 0; i <= ta->length; i++) {
        if (i == ta->length || ta->text[i] == '\n') {
            int line_len = i - line_start;
            if (line_len > 0 && line_len < 256) {
                memcpy(line_buffer, &ta->text[line_start], line_len);
                line_buffer[line_len] = '\0';

                SDL_Surface *surf = TTF_RenderText_Blended(ta->font, line_buffer, 0, text_color);
                if (surf) {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
                    if (tex) {
                        SDL_FRect dst = {text_x, y, (float)surf->w, (float)surf->h};
                        SDL_RenderTexture(ren, tex, NULL, &dst);
                        SDL_DestroyTexture(tex);
                    }
                    SDL_DestroySurface(surf);
                }
            }

            if (current_line == cursor_line && ta->cursor_visible) {
                float cursor_x = text_x;

                if (cursor_col > 0 && line_len > 0) {
                    char before_cursor[256];
                    int before_len = cursor_col < line_len ? cursor_col : line_len;
                    memcpy(before_cursor, &ta->text[line_start], before_len);
                    before_cursor[before_len] = '\0';

                    int text_w = 0, text_h = 0;
                    TTF_GetStringSize(ta->font, before_cursor, 0, &text_w, &text_h);
                    cursor_x += text_w;
                }

                SDL_FRect cursor = {cursor_x, y, 2.0f, (float)line_height};
                SDL_SetRenderDrawColor(ren, 255, 255, 100, 255);
                SDL_RenderFillRect(ren, &cursor);
            }

            y += line_height;
            line_start = i + 1;
            current_line++;
        }
    }

    if (ta->show_suggestions && ta->suggestion_count > 0) {
        float sugg_x = text_x;
        float sugg_y = (float)(area_y + 20 + (cursor_line + 1) * line_height);

        int max_width = 0;
        for (int i = 0; i < ta->suggestion_count; i++) {
            int w_calc = 0, h_calc = 0;
            TTF_GetStringSize(ta->font, ta->suggestions[i], 0, &w_calc, &h_calc);
            if (w_calc > max_width) max_width = w_calc;
        }

        SDL_FRect sugg_bg = {
            sugg_x - 5,
            sugg_y,
            (float)(max_width + 10),
            (float)(ta->suggestion_count * line_height)
        };
        SDL_SetRenderDrawColor(ren, 40, 40, 48, 255);
        SDL_RenderFillRect(ren, &sugg_bg);

        SDL_SetRenderDrawColor(ren, 100, 100, 120, 255);
        SDL_RenderRect(ren, &sugg_bg);

        SDL_Color sugg_color = {180, 180, 200, 255};
        SDL_Color selected_color = {255, 255, 100, 255};

        for (int i = 0; i < ta->suggestion_count; i++) {
            if (i == ta->selected_suggestion) {
                SDL_FRect sel_bg = {
                    sugg_x - 5,
                    sugg_y + i * line_height,
                    (float)(max_width + 10),
                    (float)line_height
                };
                SDL_SetRenderDrawColor(ren, 60, 60, 80, 255);
                SDL_RenderFillRect(ren, &sel_bg);
            }

            SDL_Color color = (i == ta->selected_suggestion) ? selected_color : sugg_color;
            SDL_Surface *surf = TTF_RenderText_Blended(ta->font, ta->suggestions[i], 0, color);
            if (surf) {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
                if (tex) {
                    SDL_FRect dst = {
                        sugg_x,
                        sugg_y + i * line_height,
                        (float)surf->w,
                        (float)surf->h
                    };
                    SDL_RenderTexture(ren, tex, NULL, &dst);
                    SDL_DestroyTexture(tex);
                }
                SDL_DestroySurface(surf);
            }
        }
    }
}