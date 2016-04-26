// The main Game state that controls everything

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_memfile.h>
#include "../game.h"
#include "../player.h"
#include "gamestate.h"
#include "scarestate.h"
#include "deadstate.h"
#include "../data/main_gfx.h"
#include "../data/music.h"
#include "../data/level.h"

struct Tile* vtiles[500];
int vtile_count = 0;

float view_x = 0;
float view_y = 0;

// Structure holding default player keys
static struct Keys default_keys = { 0, 0, 0, 0 };

// Player
static struct Player* player;

// Full list of tiles
static struct Tile* tile_list;
static int tile_count = 0;

static struct // Data
{
    ALLEGRO_BITMAP* bg;
    ALLEGRO_BITMAP* tiles;
    ALLEGRO_BITMAP* tilesred;
    ALLEGRO_BITMAP* cracks;
    ALLEGRO_BITMAP* text;
    ALLEGRO_AUDIO_STREAM* music;
    ALLEGRO_AUDIO_STREAM* music2;
    ALLEGRO_FILE* fmusic;
}
data;

static int crack_level = 0;
static int step_count = 0;
static int rush = 0;

// Used to fade out
static float alpha = 0;

// Level width
static int max_width = 0;

// Whether we've reached 'creepy mode'
static int creepy = 0;

static void on_init(void* param)
{
    int i;

    ALLEGRO_FILE* file_level = al_open_memfile(level_txt_data, level_txt_length, "r");

    while (!al_feof(file_level))
    {
        int c = al_fgetc(file_level);

        if (c == '\n')
        {
            ++tile_count;
        }
    }

    tile_list = malloc(sizeof(struct Tile) * tile_count);
    al_fseek(file_level, 0, ALLEGRO_SEEK_SET);

    for (i=0; i<tile_count; ++i)
    {
        int v;
        char line[100];

        al_fgets(file_level, line, 100);

        sscanf(line, "%d %d %d %d %d %f %f\n", &v,
          &tile_list[i].left,
          &tile_list[i].top,
          &tile_list[i].w,
          &tile_list[i].h,
          &tile_list[i].x,
          &tile_list[i].y
        );

        if (tile_list[i].x > max_width)
        {
            max_width = tile_list[i].x + tile_list[i].w;
        }
    }

    al_fclose(file_level);

    data.bg = bitmap_from_data(bg_tga_data, bg_tga_length, ".tga");
    data.tiles = bitmap_from_data(tiles_tga_data, tiles_tga_length, ".tga");
    data.tilesred = bitmap_from_data(tilesred_tga_data, tilesred_tga_length, ".tga");
    data.cracks = bitmap_from_data(cracks_tga_data, cracks_tga_length, ".tga");
    data.text = bitmap_from_data(text_tga_data, text_tga_length, ".tga");

    data.fmusic = al_open_memfile(music1_ogg_data, music1_ogg_length, "r");
    data.music = al_load_audio_stream_f(data.fmusic, ".ogg", 2, 4096);

    al_attach_audio_stream_to_mixer(data.music, al_get_default_mixer());
    al_set_audio_stream_playmode(data.music, ALLEGRO_PLAYMODE_LOOP);

    // Loop points for the music
    al_set_audio_stream_loop_secs(data.music, 20.274,
      al_get_audio_stream_length_secs(data.music));

    srand(time(NULL));

    player = create_player(100, 100, &default_keys);
}

static void on_end()
{
    al_destroy_bitmap(data.bg);
    al_destroy_bitmap(data.tiles);
    al_destroy_bitmap(data.tilesred);
    al_destroy_bitmap(data.cracks);
    al_destroy_bitmap(data.text);

    if (data.music2 != NULL)
    {
        al_destroy_audio_stream(data.music2);
    }

    free(tile_list);

    destroy_player(player);
}

static void on_pause()
{
}

static void on_resume()
{
    al_set_audio_stream_playing(data.music, 0);
    al_fclose(data.fmusic);

    data.music2 = al_load_audio_stream("music2.ogg", 2, 4096);

    if (data.music2 != NULL)
    {
        al_attach_audio_stream_to_mixer(data.music2, al_get_default_mixer());
        al_set_audio_stream_playmode(data.music2, ALLEGRO_PLAYMODE_LOOP);
    }
}

static void on_events(ALLEGRO_EVENT* event)
{
    if (event->type == ALLEGRO_EVENT_KEY_DOWN)
    {
        if (event->keyboard.keycode == ALLEGRO_KEY_LEFT)
        {
            default_keys.left = 1;
        }

        if (event->keyboard.keycode == ALLEGRO_KEY_RIGHT)
        {
            default_keys.right = 1;
        }

        if (event->keyboard.keycode == ALLEGRO_KEY_UP)
        {
            default_keys.jump = 1;
        }

        if (event->keyboard.keycode == ALLEGRO_KEY_LSHIFT
        || event->keyboard.keycode == ALLEGRO_KEY_RSHIFT)
        {
            default_keys.run = 1;
        }
    }

    if (event->type == ALLEGRO_EVENT_KEY_UP)
    {
        if (event->keyboard.keycode == ALLEGRO_KEY_LEFT)
        {
            default_keys.left = 0;
        }

        if (event->keyboard.keycode == ALLEGRO_KEY_RIGHT)
        {
            default_keys.right = 0;
        }

        if (event->keyboard.keycode == ALLEGRO_KEY_UP)
        {
            default_keys.jump = 0;
        }

        if (event->keyboard.keycode == ALLEGRO_KEY_LSHIFT
        || event->keyboard.keycode == ALLEGRO_KEY_RSHIFT)
        {
            default_keys.run = 0;
        }
    }

    if (creepy)
    {
        default_keys.run = 0;
    }
}

static void on_update()
{
    int i;

    vtile_count = 0;

    for (i=0; i<tile_count; ++i)
    {
        if (tile_list[i].x < (view_x + (SCREEN_W + tile_list[i].w))
            && tile_list[i].x > (view_x - (tile_list[i].w * 2)))
        {
            vtiles[vtile_count++] = &tile_list[i];
        }
    }

    if ((default_keys.left || default_keys.right) && !creepy)
    {
        if (rush)
        {
            ++crack_level;
        }
        else
        {
            ++step_count;

            // Increment crack level each 75 steps
            if ((step_count % 75) < 1)
            {
                ++crack_level;

                // scarestate may appear quicker than normal...
                if (rand() % 20 == 1 && view_x > 1000)
                {
                    rush = 1;
                }
            }
        }

        if (crack_level >= 13)
        {
            if (data.music != NULL)
            {
                al_set_audio_stream_playing(data.music, 0);
            }

            push_state(SCARE_STATE, NULL);
            creepy = 1;
            crack_level = 14;
            default_keys.left = 0;
            default_keys.right = 0;
            default_keys.run = 0;
            default_keys.jump = 0;
            go_down = 1;
        }
    }

    int x, y;
    player_update(player);
    player_get_pos(player, &x, &y);

    // Update camera
    while (view_x > (max_width - SCREEN_W) || x < view_x + 270)
    {
        --view_x;
    }

    while (view_x < 0 || x > view_x + 320)
    {
        ++view_x;
    }

    if (go_down && view_x > 5555)
    {
        while (y > view_y + 222)
        {
            ++view_y;
        }
    }

    if (view_y > 3333)
    {
        // Fade out slowly
        alpha += 0.01;

        if (alpha >= 1.0)
        {
            change_state(DEAD_STATE, NULL);
        }
    }
}

static void on_draw()
{
    int i, j;

    if (!creepy)
    {
        int w = al_get_bitmap_width(data.bg);
        int h = al_get_bitmap_height(data.bg);

        for (j=0; j<SCREEN_H; j+=w)
        {
            for (i=0; i<max_width; i+=h)
            {
                al_draw_bitmap(data.bg, i - view_x * 0.4, j, 0);
            }
        }
    }

    for (i=0; i<vtile_count; ++i)
    {
        al_draw_bitmap_region(creepy ? data.tilesred : data.tiles,
            vtiles[i]->left,
            vtiles[i]->top,
            vtiles[i]->w,
            vtiles[i]->h,
            vtiles[i]->x - view_x,
            vtiles[i]->y - view_y,
        0);

        if (creepy)
        {
            al_draw_bitmap_region(data.cracks,
                416, 0, 32, 32,
                vtiles[i]->x - view_x,
                vtiles[i]->y - view_y,
            0);
        }
        else
        {
            al_draw_bitmap_region(data.cracks,
                crack_level * 32, 0, 32, 32,
                vtiles[i]->x - view_x,
                vtiles[i]->y - view_y,
            0);
        }
    }

    al_draw_bitmap(data.text, 257 - view_x, 289, 0);

    player_draw(player);

    al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H,
        al_map_rgba_f(0, 0, 0, alpha));
}

struct State* get_game_state()
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
