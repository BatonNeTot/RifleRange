//
// Created by KILLdon on 27.05.2019.
//

#ifndef RIFLERANGE_SPRITE_H
#define RIFLERANGE_SPRITE_H

#include "locator.h"

#include "window.h"
#include "object_pool.h"
#include "data.h"
#include "opengl_helper.h"
#include "lua_helper.h"

#include <map>
#include <string>

class Sprite {
public:

    inline Size<unsigned> getSize() { return size; }
    inline unsigned getSlides() { return slides; }
    inline Point<int> getOffset() { return offset; }

    // Lua

    inline static const char * const metatable = "metatable.sprite";
    static void setupMeta(lua_State *L);

    static Sprite &getFromLua(lua_State *L, int index);
    void pushToLua(lua_State *L);

private:
    typedef ObjectPool<Sprite, 16> Pool;

    friend Pool;
    friend class Sprites;
    friend class Render;

    const GLuint textureId;

    const Size<unsigned> size;
    const unsigned slides;
    const Point<int> offset;


    Sprite(GLuint textureId, Size<unsigned> size, unsigned slides, Point<int> offset);
    ~Sprite();

    // Lua

    static int L_getSize(lua_State *L);
    static int L_getSlides(lua_State *L);
    static int L_getOffset(lua_State *L);
};

class Sprites {
public:

    friend class Locator;
    friend class Game;

    Sprite defaultSprite;

    void load(const char *name, const char *source, unsigned count, Point<int> offset);
    Sprite &get(const char *name);

    // Lua

    static void setupLuaVar(lua_State *L);

private:
    Sprites();

    inline static const char * const configName = "name";
    inline static const char * const configSource = "source";
    inline static const char * const configOffset = "offset";
    inline static const char * const configSlides = "slides";

    bool setupConfig(const char *filename);

    Sprite::Pool pool;
    std::map<std::string, Sprite *> spriteMap;

    // Lua

    static int L_getSprite(lua_State *L);
};

#endif //RIFLERANGE_SPRITE_H
