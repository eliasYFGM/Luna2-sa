#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <allegro5/allegro.h>

// Main state structure
struct State
{
    void (*init)(void*);
    void (*end)();

    void (*pause)();
    void (*resume)();

    void (*events)(ALLEGRO_EVENT*);
    void (*update)();
    void (*draw)();
};

#endif // STATE_H_INCLUDED
