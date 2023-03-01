//
// Created by KILLdon on 28.05.2019.
//

#include "sprite.h"

#include "game.h"
#include "logger.h"
#include "window.h"
#include "image_loader.h"
#include "lua_engine.h"

#include <iostream>

Sprite::Sprite(GLuint textureId, Size<unsigned> size, unsigned slides, Point<int> offset) :
    textureId(textureId),
    size(size),
    slides(slides),
    offset(offset){}

Sprite::~Sprite() {
    glDeleteTextures(1, &textureId);
}

void Sprite::setupMeta(lua_State *L) {
    luaL_newmetatable(L, Sprite::metatable);
    lua_createtable(L, 0, 3);

    luaL_Reg spriteFunc[] = {
            {"getSize", Sprite::L_getSize},
            {"getSlides", Sprite::L_getSlides},
            {"getOffset", Sprite::L_getOffset},
            {NULL, NULL},
    };
    luaL_register(L, NULL, &spriteFunc[0]);

    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

int Sprite::L_getSize(lua_State *L) {
    auto &sprite = getFromLua(L, 1);
    auto &&size = sprite.getSize();
    lua_pushinteger(L, size.width);
    lua_pushinteger(L, size.height);
    return 2;
}
int Sprite::L_getSlides(lua_State *L) {
    auto &sprite = getFromLua(L, 1);
    lua_pushinteger(L, sprite.getSlides());
    return 1;
}
int Sprite::L_getOffset(lua_State *L) {
    auto &sprite = getFromLua(L, 1);
    auto &&offset = sprite.getOffset();
    lua_pushinteger(L, offset.x);
    lua_pushinteger(L, offset.y);
    return 2;
}

Sprite &Sprite::getFromLua(lua_State *L, int index) {
    return **(Sprite **)luaL_checkudata(L, index, Sprite::metatable);
}

void Sprite::pushToLua(lua_State *L) {
    auto ptr = (Sprite **)lua_newuserdata(L, sizeof(Sprite *));
    *ptr = this;
    luaL_newmetatable(L, Sprite::metatable);
    lua_setmetatable(L, -2);
}

Sprites::Sprites() : defaultSprite(0, {0,0}, 1, {0,0}){}

void Sprites::load(const char *name, const char *source, unsigned slides, Point<int> offset) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

// set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

// load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(source, &width, &height, &nrChannels, STBI_rgb_alpha);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        auto *sprite = pool.create(texture, Size<unsigned>(width / slides, height), slides, offset);
        spriteMap.insert(std::make_pair(name, sprite));
    }

    stbi_image_free(data);
}

Sprite &Sprites::get(const char *name) {
    auto iter = spriteMap.find(name);
    if (iter == spriteMap.end()) {
        return defaultSprite;
    }
    return *iter->second;
}

bool Sprites::setupConfig(const char *filename) {
    if (!Game::locator().lua().loadObjectFile(filename)) {
        Game::locator().logger().print("Error loading sprites file!");
        Game::locator().logger().print(Game::locator().lua().getString());
        Game::locator().lua().pop();
        return false;
    }

    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Sprites file isn't a table!");
        Game::locator().lua().pop();
        return false;
    }

    Game::locator().lua().foreachInTable([this](unsigned index){
        const char* name = Game::locator().lua().getPathString({configName}, nullptr);
        if (!name) {
            Game::locator().logger().print("Sprite in %ud index doesn't have name", index);
            return;
        }

        const char* source = Game::locator().lua().getPathString({configSource}, nullptr);
        if (!source) {
            Game::locator().logger().print("Sprite in %ud index doesn't have source", index);
            return;
        }

        Point<int> offset = {
                Game::locator().lua().getPathLong({configOffset, Lua::configX}, 0),
                Game::locator().lua().getPathLong({configOffset, Lua::configY}, 0)};

        long inputSlides = Game::locator().lua().getPathLong({configSlides}, 1);
        unsigned slides;
        if (inputSlides < 1) {
            Game::locator().logger().print("Sprite in %ud index:", index);
            Game::locator().logger().print("Slides must be more than zero: slides = %ld", inputSlides);
            slides = 1;
        } else {
            slides = inputSlides;
        }

        load(name, source, slides, offset);
    });

    Game::locator().lua().pop();

    return true;
}

void Sprites::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 2); // meta

    lua_createtable(L, 0, 1);

    luaL_Reg spritesFunc[] = {
            {"getSprite", Sprites::L_getSprite},
            {NULL, NULL},
    };
    luaL_register(L, NULL, spritesFunc);

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Sprites");
}

int Sprites::L_getSprite(lua_State *L) {
    Game::locator().sprites().get(luaL_checkstring(L, 1)).pushToLua(L);
    return 1;
}