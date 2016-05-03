#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_memfile.h>
#include "game.h"
#include "state.h"

static struct // Game data
{
    ALLEGRO_DISPLAY* display;
    ALLEGRO_BITMAP* buffer;
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* event_queue;
    int initialized;
    int is_running;
    ALLEGRO_COLOR bg_color;
}
game =
{
    NULL, NULL, NULL, NULL,
    0, 0,
    { 0, 0, 0, 0 }
};

struct Game_Config* game_config;
ALLEGRO_FONT* font;

#define MAX_STATES  8
static struct State* states[MAX_STATES];
static int current_state = 0;

// Updates the aspect ratio when going full-screen or windowed
static void aspect_ratio_transform()
{
    int window_w = al_get_display_width(game.display);
    int window_h = al_get_display_height(game.display);

    float sw = (window_w / (float) SCREEN_W);
    float sh = (window_h / (float) SCREEN_H);
    float scale = (sw < sh ? sw : sh);

    float scale_w = ((float) SCREEN_W * scale);
    float scale_h = ((float) SCREEN_H * scale);
    int scale_x_pos = (window_w - scale_w) / 2;
    int scale_y_pos = (window_h - scale_h) / 2;

    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_build_transform(&trans, scale_x_pos, scale_y_pos, scale, scale, 0);
    al_use_transform(&trans);
}

int game_init(struct Game_Config* config, int argc, char** argv)
{
    int i;

    if (game.initialized)
    {
        puts("WARNING: Game already initialized");
        return 1;
    }

    for (i=0; i<MAX_STATES; ++i)
    {
        states[current_state] = NULL;
    }

    // Initialize Allegro and stuff
    al_init();

    if (!al_install_keyboard())
    {
        puts("ERROR: Could not initialize the keyboard...");
        return 0;
    }

    if (!al_install_mouse())
    {
        puts("ERROR: Could not initialize the mouse...");
        return 0;
    }

    if (config->audio)
    {
        if (!al_install_audio())
        {
            puts("ERROR: Could not initialize audio...");
            return 0;
        }

        if (!al_init_acodec_addon())
        {
            puts("ERROR: Could not initialize acodec addon...");
            return 0;
        }

        al_reserve_samples(1);
    }

    // Add-ons
    if (!al_init_image_addon())
    {
        puts("ERROR: Could not initialize image addon...");
        return 0;
    }

    if (!al_init_primitives_addon())
    {
        puts("ERROR: Could not initialize primitives addon...");
        return 0;
    }

    al_init_font_addon();

    if (config->fullscreen)
    {
        al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    }

    // Create our display
    game.display = al_create_display(config->width, config->height);

    if (!game.display)
    {
        puts("ERROR: Could not create a display window...");
        return 0;
    }

    al_set_window_title(game.display, config->title);

    // Use built-in Allegro font
    font = al_create_builtin_font();

    // Use linear filtering for scaling game screen
    al_add_new_bitmap_flag(ALLEGRO_MAG_LINEAR);
    game.buffer = al_create_bitmap(config->width, config->height);

    game_config = config;
    aspect_ratio_transform();

    game.timer = al_create_timer(1.0 / config->framerate);
    game.event_queue = al_create_event_queue();

    game.bg_color = al_map_rgb(192, 192, 192);
    game.initialized = 1;
    game.is_running = 1;

    return 1;
}

void game_run()
{
    int i, redraw = 0;

    // Register event sources
    al_register_event_source(game.event_queue,
        al_get_display_event_source(game.display));

    al_register_event_source(game.event_queue,
        al_get_keyboard_event_source());

    al_register_event_source(game.event_queue,
        al_get_mouse_event_source());

    al_register_event_source(game.event_queue,
        al_get_timer_event_source(game.timer));

    al_start_timer(game.timer);

    // Main game loop
    while (game.is_running)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(game.event_queue, &event);

        // Event processing
        states[current_state]->events(&event);

        // If the close button was pressed...
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            game.is_running = 0;
            break;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            // Escape key will end the game
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                game.is_running = 0;
                break;
            }

            // The F4 key will switch between screen modes (mantaining aspect ratio)
            // Inspired by Game Maker.
            if (event.keyboard.keycode == ALLEGRO_KEY_F4)
            {
                if (al_get_display_flags(game.display) & ALLEGRO_FULLSCREEN_WINDOW)
                {
                    al_toggle_display_flag(game.display, ALLEGRO_FULLSCREEN_WINDOW, 0);
                }
                else
                {
                    al_toggle_display_flag(game.display, ALLEGRO_FULLSCREEN_WINDOW, 1);
                }

                aspect_ratio_transform();
            }
        }
        else if (event.type == ALLEGRO_EVENT_TIMER)
        {
            states[current_state]->update();
            redraw = 1;
        }

        if (redraw && al_event_queue_is_empty(game.event_queue))
        {
            redraw = 0;

            al_set_target_bitmap(game.buffer);

            al_clear_to_color(game.bg_color);

            states[current_state]->draw();

            al_set_target_backbuffer(game.display);

            al_clear_to_color(C_BLACK);

            al_draw_bitmap(game.buffer, 0, 0, 0);

            al_flip_display();
        }
    }

    for (i=0; i<MAX_STATES; ++i)
    {
        if (states[i] != NULL)
        {
            states[i]->end();
        }
    }

    al_destroy_display(game.display);
    al_destroy_bitmap(game.buffer);
    al_destroy_timer(game.timer);
    al_destroy_event_queue(game.event_queue);
    al_destroy_font(font);
}

void game_over()
{
    game.is_running = 0;
}

void set_bg_color(ALLEGRO_COLOR color)
{
    game.bg_color = color;
}

ALLEGRO_BITMAP* bitmap_from_data(void* data, unsigned int length,
  const char* type)
{
    ALLEGRO_BITMAP* bmp = NULL;

    ALLEGRO_FILE* f = al_open_memfile(data, length, "r");

    if (f != NULL)
    {
        bmp = al_load_bitmap_f(f, type);
        al_fclose(f);
    }

    return bmp;
}

void change_state(struct State* state, void* param)
{
    if (states[current_state] != NULL)
    {
        states[current_state]->end();
    }

    states[current_state] = state;
    state->init(param);
}

void push_state(struct State* state, void* param)
{
    if (current_state < (MAX_STATES - 1))
    {
        if (states[current_state] != NULL)
        {
            states[current_state]->pause();
        }

        states[++current_state] = state;
        state->init(param);
    }
    else
    {
        puts("WARNING: Can't add new state (current_state = MAX_STATES)");
    }
}

void pop_state()
{
    if (current_state > 0)
    {
        states[current_state]->end();
        states[current_state] = NULL;
        states[--current_state]->resume();
    }
    else
    {
        puts("WARNING: Can't remove any more states (current_state = 0)");
    }
}
