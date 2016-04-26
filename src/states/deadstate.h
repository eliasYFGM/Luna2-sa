#ifndef DEADSTATE_H_INCLUDED
#define DEADSTATE_H_INCLUDED

#include "../state.h"

struct State* get_dead_state();

#define DEAD_STATE  get_dead_state()

#endif // DEADSTATE_H_INCLUDED
