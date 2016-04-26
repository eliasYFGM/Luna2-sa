#include <allegro5/allegro.h>
#include "player.h"
#include "game.h"
#include "states/gamestate.h"
#include "states/deadstate.h"
#include "data/sprites.h"

struct Player
{
    float x, y;
    float yspeed;
    int dir;

    struct // Sprite data
    {
        ALLEGRO_BITMAP* stand;
        ALLEGRO_BITMAP* walk;
        ALLEGRO_BITMAP* flying;
        int frame;
    }
    sprite;

    struct Keys* keys;
};

int go_down = 0;

// Checks for a tile using the player's position and width/height
static struct Tile* check_tile(struct Player* p, float x, float y)
{
    int i;
    struct Tile* t = NULL;

    for (i=0; i<vtile_count; ++i)
    {
        if (check_bb_collision((p->x + 15) + x, (p->y + 20) + y,
            22, 23, vtiles[i]->x, vtiles[i]->y, vtiles[i]->w, vtiles[i]->h))
        {
            t = vtiles[i];
        }
    }

    return t;
}

struct Player* create_player(float x, float y, struct Keys* keys)
{
    struct Player* p = malloc(sizeof(struct Player));

    p->sprite.stand = bitmap_from_data(stand_tga_data, stand_tga_length, ".tga");
    p->sprite.walk = bitmap_from_data(trotting_tga_data, trotting_tga_length, ".tga");
    p->sprite.flying = bitmap_from_data(flying_tga_data, flying_tga_length, ".tga");
    p->sprite.frame = 0;

    p->keys = keys;

    p->x = x;
    p->y = y;
    p->yspeed = 0;
    p->dir = 1;

    return p;
}

void destroy_player(struct Player* p)
{
    al_destroy_bitmap(p->sprite.stand);
    al_destroy_bitmap(p->sprite.walk);
    al_destroy_bitmap(p->sprite.flying);

    free(p);
}

void player_update(struct Player* p)
{
    // Moving...
    if (p->keys->left)
    {
        p->dir = 0;

        if (p->y < 480)
        {
            p->x -= p->keys->run ? 6 : 3;
        }

        while (check_tile(p, 0, 0))
        {
            ++p->x;
        }
    }
    else if (p->keys->right)
    {
        p->dir = 1;

        if (p->y < 480)
        {
            p->x += p->keys->run ? 6 : 3;
        }

        while (check_tile(p, 0, 0))
        {
            --p->x;
        }
    }

    // Only jump if Luna is standing on ground
    if (p->keys->jump && check_tile(p, 0, 1))
    {
        p->yspeed = -12;
    }

    // Update vertical speed
    p->yspeed += 0.5;
    if (p->yspeed > 12)
    {
        p->yspeed = 12;
    }

    p->y += p->yspeed;

    // Stop Luna when overlapping with tiles
    struct Tile* t = check_tile(p, 0, 0);
    if (t)
    {
        if (p->yspeed > 0)
        {
            p->y = t->y - 43;
        }
        else if (p->yspeed < 0)
        {
            p->y = t->y + t->h - 20;
        }

        p->yspeed = 0;
    }

    // Update frame of animation
    if (p->keys->run)
    {
        p->sprite.frame += 2;
    }
    else
    {
        ++p->sprite.frame;
    }

    if (p->sprite.frame >= 16)
    {
        p->sprite.frame = 0;
    }
}

void player_draw(struct Player* p)
{
    // On ground
    if (check_tile(p, 0, 1))
    {
        if (p->keys->left || p->keys->right)
        {
            al_draw_bitmap_region(p->sprite.walk, p->sprite.frame * 48,
                0, 48, 47, p->x - view_x, p->y - 4 - view_y,
                p->dir == 1 ? 0 : ALLEGRO_FLIP_HORIZONTAL);
        }
        else
        {
            al_draw_bitmap(p->sprite.stand, p->x - view_x, p->y - view_y,
            p->dir == 1 ? 0 : ALLEGRO_FLIP_HORIZONTAL);
        }
    }
    else // Flying or falling
    {
        al_draw_bitmap_region(p->sprite.flying, p->sprite.frame * 48,
            0, 48, 56, p->x - view_x, p->y - 4 - view_y,
            p->dir == 1 ? 0 : ALLEGRO_FLIP_HORIZONTAL);
    }
}

void player_get_pos(struct Player* p, int* x, int* y)
{
    *x = p->x;
    *y = p->y;
}
