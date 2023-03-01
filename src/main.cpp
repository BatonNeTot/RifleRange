#include <iostream>
#include <vector>

#include "game.h"

int main(int argc, char **argv) {
    const char *gameFile = "res/game.lua";
    if (argc >= 2) {
        gameFile = argv[1];
    }
    if (Game::instance().init(gameFile)) {
        Game::instance().run();
        return 0;
    } else {
        return -1;
    }
}