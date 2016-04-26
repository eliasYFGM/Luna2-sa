#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include "deadstate.h"
#include "../game.h"
#include "../data/dead.h"

static struct // Data
{
    ALLEGRO_BITMAP* dead;
    ALLEGRO_AUDIO_STREAM* music;
}
data;

static float alpha = 0;

static void on_init(void* param)
{
    data.dead = bitmap_from_data(dead_tga_data, dead_tga_length, ".tga");

    data.music = al_load_audio_stream("youdied.ogg", 2, 4086);
    if (data.music != NULL)
    {
        al_attach_audio_stream_to_mixer(data.music, al_get_default_mixer());
        al_set_audio_stream_playing(data.music, 1);
    }

    set_bg_color(C_BLACK);
}

static void on_end()
{
    al_destroy_bitmap(data.dead);

    if (data.music != NULL)
    {
        al_destroy_audio_stream(data.music);
    }
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
    // Fade-in animation
    if (alpha < 1.0)
    {
        alpha += 0.05;
    }
    else
    {
        al_rest(25.0);
        game_over();
    }
}

static void on_draw()
{
    al_draw_tinted_bitmap(data.dead,
        al_map_rgba_f(1.0 * alpha, 1.0 * alpha, 1.0 * alpha, alpha),
        0, 0, 0);
}

struct State* get_dead_state()
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
