#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

struct Player;

struct Keys
{
    int left;
    int right;
    int run;
    int jump;
};

struct Player* create_player(float x, float y, struct Keys*);
void destroy_player(struct Player*);
void player_update(struct Player*);
void player_draw(struct Player*);
void player_get_pos(struct Player*, int* x, int* y);

extern int go_down;

#endif // PLAYER_H_INCLUDED
