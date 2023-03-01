//
// Created by KILLdon on 26.05.2019.
//

#ifndef RIFLERANGE_GAME_H
#define RIFLERANGE_GAME_H

#include "locator.h"

class Game {
public:
    static Game &instance() {
        static Game game;
        return game;
    }

    inline static Locator &locator() { return instance().mLocator; };
    bool init(const char *name);
    void run();

private:
    Game() = default;

    Locator mLocator;

    inline static const char *const configWindow = "Window";
    inline static const char *const configSprites = "Sprites";
    inline static const char *const configParticles = "Particles";
    inline static const char *const configSounds = "Sounds";
    inline static const char *const configFonts = "Fonts";
    inline static const char *const configEntities = "Entities";
};

#endif //RIFLERANGE_GAME_H
