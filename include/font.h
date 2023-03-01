//
// Created by KILLdon on 02.06.2019.
//

#ifndef RIFLERANGE_FONT_H
#define RIFLERANGE_FONT_H

#include "locator.h"

#include "object_pool.h"
#include "opengl_helper.h"
#include "data.h"
#include "lua_helper.h"

#include <string>
#include <map>

class Font {
public:

    enum class ALIGNMENT {
        LEFT = 0u,
        H_CENTER = 1u,
        RIGHT = 2u,
        UP = 0u,
        V_CENTER = 4u,
        DOWN = 8u,
        CENTER = 5u,
    };

    void draw(const char *text, Point<float> pos, ALIGNMENT alignment, short depth);

    GLuint getTexture();
    const Size<unsigned> &getLetterSize();
    const Point<unsigned> &getSlices();


    // Lua

    inline static const char * const metatable = "metatable.font";
    static void setupMeta(lua_State *L);

    static Font &getFromLua(lua_State *L, int index);
    void pushToLua(lua_State *L);

private:
    Font(GLuint textureId, Size<unsigned> size, Point<unsigned> slices, std::map<char, unsigned> &chars);
    Font(GLuint textureId, Size<unsigned> size, Point<unsigned> slices, std::map<char, unsigned> &&chars);

    typedef ObjectPool<Font, 4> Pool;

    friend Pool;
    friend class Fonts;

    const GLuint textureId;
    const Size<unsigned> size;
    const Point<unsigned> slices;
    const std::map<char, unsigned> chars;

    // Lua

    static int L_draw(lua_State *L);
    static int L_getSize(lua_State *L);

};

class Fonts {
public:

    void load(const char *name, const char *source, const char *letters, Point<unsigned> slices);
    Font &get(const char *name);

    friend class Locator;
    friend class Game;
    friend Font;

    // Lua

    static void setupLuaVar(lua_State *L);

private:
    Fonts();

    inline static const char * const configName = "name";
    inline static const char * const configSource = "source";
    inline static const char * const configSlices = "slices";
    inline static const char * const configLetters = "letters";

    bool setupConfig(const char *filename);

    Font defaultFont;

    Font::Pool pool;
    std::map<std::string, Font *> fonts;

    // Lua

    static int L_getFont(lua_State *L);
};

#endif //RIFLERANGE_FONT_H
