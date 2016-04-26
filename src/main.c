#include "game.h"
#include "states/gamestate.h"

int main(int argc, char* argv[])
{
    // Default configuration that the game will make use of
    struct Game_Config config =
    {
        // Window title
        "Luna",
        // Display resolution
        640, 480,
        // Framerate (FPS)
        30,
        // Want full-screen?
        1,
        // Want audio module?
        1
    };

    if (game_init(&config, argc, argv))
    {
        // Starting state
        // Second argument is a pointer that will be passed to the init()
        // function of the state (a void*)
        change_state(GAME_STATE, NULL);

        // Run the game until it's done
        game_run();
    }

    return 0;
}
