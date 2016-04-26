#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include "scarestate.h"
#include "../game.h"

static struct // Data
{
    ALLEGRO_BITMAP* image;
    ALLEGRO_SAMPLE* noise;
}
data;

static int step_count = 0;

static void on_init(void* param)
{
    data.image = al_load_bitmap("zalgopie.png");

    data.noise = al_load_sample("noise.wav");

    if (data.noise != NULL)
    {
        al_play_sample(data.noise, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }

    set_bg_color(C_BLACK);
}

static void on_end()
{
    if (data.image != NULL)
    {
        al_destroy_bitmap(data.image);
    }

    if (data.noise != NULL)
    {
        al_destroy_sample(data.noise);
    }

    set_bg_color(al_map_rgb(30, 0, 0));
}

static void on_pause()
{
}

static void on_resume()
{
}

static void on_events(ALLEGRO_EVENT* event)
{
}

static void on_update()
{
    ++step_count;

    if (step_count > game_config->framerate)
    {
        // Back to the game
        pop_state();
    }
}

static void on_draw()
{
    al_draw_text(font, C_WHITE, SCREEN_W / 2, SCREEN_H / 2,
        ALLEGRO_ALIGN_CENTER, "zalgopie.png");

    if (data.image != NULL)
    {
        al_draw_bitmap(data.image, 0, 0, 0);
    }
}

struct State* get_scare_state()
{
    static struct State state =
    {
        on_init,
        on_end,
        on_pause,
        on_resume,
        on_events,
        on_update,
        on_draw
    };

    return &state;
}
