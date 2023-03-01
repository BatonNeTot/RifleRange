//
// Created by KILLdon on 26.05.2019.
//

#include "game.h"

#include "logger.h"
#include "window.h"
#include "input.h"
#include "sprite.h"
#include "particles.h"
#include "entity.h"
#include "render.h"
#include "sound.h"
#include "lua_engine.h"

#include "object_pool.h"

bool Game::init(const char *name) {
    locator().window().init();
    locator().input().init();
    locator().render().init();
    locator().lua().setupGlobal();

    if (!locator().lua().loadObjectFile(name)) {
        Game::locator().logger().print("Error loading game file!");
        Game::locator().logger().print(locator().lua().getString());
        return false;
    }

    if (!locator().lua().isTable()) {
        Game::locator().logger().print("Game file isn't a table!");
        locator().lua().pop();
        return false;
    }

    auto spritesPath = locator().lua().getPathString({configSprites}, nullptr);
    if (spritesPath) {
        if (!locator().sprites().setupConfig(spritesPath)) {
            return false;
        }
    } else {
        Game::locator().logger().print("Can't get sprites config file path!");
        locator().lua().pop();
        return false;
    }

    auto windowPath = locator().lua().getPathString({configWindow}, nullptr);
    if (windowPath) {
        if (!locator().window().setupConfig(windowPath)) {
            return false;
        }
    } else {
        Game::locator().logger().print("Can't get window config file path!");
        locator().lua().pop();
        return false;
    }

    auto particlesPath = locator().lua().getPathString({configParticles}, nullptr);
    if (particlesPath) {
        if (!locator().particleMaster().setupConfig(particlesPath)) {
            return false;
        }
    } else {
        Game::locator().logger().print("Can't get particles config file path!");
        locator().lua().pop();
        return false;
    }

    auto soundsPath = locator().lua().getPathString({configSounds}, nullptr);
    if (soundsPath) {
        if (!locator().soundMaster().setupConfig(soundsPath)) {
            return false;
        }
    } else {
        Game::locator().logger().print("Can't get sounds config file path!");
        locator().lua().pop();
        return false;
    }

    auto fontsPath = locator().lua().getPathString({configFonts}, nullptr);
    if (fontsPath) {
        if (!locator().fonts().setupConfig(fontsPath)) {
            return false;
        }
    } else {
        Game::locator().logger().print("Can't get fonts config file path!");
        locator().lua().pop();
        return false;
    }

    auto entitiesPath = locator().lua().getPathString({configEntities}, nullptr);
    if (entitiesPath) {
        if (!locator().entities().setupConfig(entitiesPath)) {
            return false;
        }
    } else {
        Game::locator().logger().print("Can't get entities config file path!");
        locator().lua().pop();
        return false;
    }

    locator().lua().pop();

    return true;
}

void Game::run() {
    while(!locator().window().isShouldClose()) {
        locator().render().renderWorld();

        locator().input().update();
        locator().entities().foreach([this](Entity &entity){
            locator().lua().cleanEntityEnvironment(entity);
            locator().lua().evalOnStep(entity);
        });
        locator().entities().clearDestroying();

        locator().particleMaster().foreach([this](ParticleGenerator &generator){
            generator.update(locator().window().getDelta());
            locator().render().renderParticles(generator);
        });
        locator().particleMaster().collectGarbage();

        locator().soundMaster().collectGarbage();

        locator().window().update();
    }
}
