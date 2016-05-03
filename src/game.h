#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <allegro5/allegro_font.h>

// Color defines
#define C_BLACK     al_map_rgb(0, 0, 0)
#define C_WHITE     al_map_rgb(255, 255, 255)

struct Game_Config
{
    char *title;
    int width;
    int height;
    int framerate;
    int fullscreen;
    int audio;
};

// Pointer to the original game settings (main.c)
extern struct Game_Config* game_config;

#define SCREEN_W    game_config->width
#define SCREEN_H    game_config->height

// Default fixed-width font
extern ALLEGRO_FONT* font;

// Main game engine routines
int game_init(struct Game_Config* config, int argc, char** argv);
void game_run();
void game_over();
void set_bg_color(ALLEGRO_COLOR);
ALLEGRO_BITMAP* bitmap_from_data(void*, unsigned int length, const char* type);

struct State;

// State routines
void change_state(struct State* state, void* param);
void push_state(struct State* state, void* param);
void pop_state();

// Simple bounding box collision checking (taken from Alex4 source)
#define check_bb_collision(x1,y1,w1,h1,x2,y2,w2,h2) \
    (!( ((x1)>=(x2)+(w2)) || ((x2)>=(x1)+(w1)) || \
        ((y1)>=(y2)+(h2)) || ((y2)>=(y1)+(h1)) ))

#endif // GAME_H_INCLUDED
