//
// Created by KILLdon on 02.06.2019.
//

#include "font.h"

#include "game.h"
#include "logger.h"
#include "window.h"
#include "image_loader.h"
#include "lua_engine.h"
#include "render.h"

Font::Font(GLuint textureId, Size<unsigned> size, Point<unsigned> slices, std::map<char, unsigned> &chars) :
        textureId(textureId),
        size(size),
        slices(slices),
        chars(chars) {}

Font::Font(GLuint textureId, Size<unsigned> size, Point<unsigned> slices, std::map<char, unsigned> &&chars) :
        textureId(textureId),
        size(size),
        slices(slices),
        chars(chars) {}

void Font::draw(const char *text, Point<float> pos, ALIGNMENT alignment = (ALIGNMENT)0, short depth = 0) {
    unsigned length = strlen(text);
    std::vector<unsigned> textCodes;
    textCodes.reserve(length);
    while (*text != '\0') {
        auto iter = chars.find(*text++);
        textCodes.push_back(iter != chars.end() ? iter->second : 0);
    }
    Game::locator().render().renderText(*this, textCodes, pos, alignment, depth);
}

GLuint Font::getTexture() {
    return textureId;
}
const Size<unsigned> &Font::getLetterSize() {
    return size;
}
const Point<unsigned> &Font::getSlices() {
    return slices;
}

void Font::setupMeta(lua_State *L) {
    luaL_newmetatable(L, Font::metatable);
    lua_createtable(L, 0, 1);

    luaL_Reg fontFunc[] = {
            {"draw", Font::L_draw},
            {"getSize", Font::L_getSize},
            {NULL, NULL},
    };
    luaL_register(L, NULL, &fontFunc[0]);

    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

int Font::L_draw(lua_State *L) {
    auto &font = getFromLua(L, 1);
    const char *text = luaL_checkstring(L, 2);
    Point<float> pos;
    pos.x = luaL_checknumber(L, 3);
    pos.y = luaL_checknumber(L, 4);

    unsigned top = lua_gettop(L);
    if (top == 4) {
        font.draw(text, pos);
    } else {
        unsigned alignment = luaL_checkinteger(L, 5);
        if (top == 5) {
            font.draw(text, pos, (ALIGNMENT) alignment);
        } else {
            short depth = luaL_checkinteger(L, 6);
            font.draw(text, pos, (ALIGNMENT) alignment, depth);
        }
    }
    return 0;
}

int Font::L_getSize(lua_State *L) {
    auto &font = getFromLua(L, 1);
    auto &&size = font.size;
    lua_pushinteger(L, size.width);
    lua_pushinteger(L, size.height);
    return 2;
}

Font &Font::getFromLua(lua_State *L, int index) {
    return **(Font **)luaL_checkudata(L, index, Font::metatable);
}
void Font::pushToLua(lua_State *L) {
    auto ptr = (Font **)lua_newuserdata(L, sizeof(Font *));
    *ptr = this;
    luaL_newmetatable(L, Font::metatable);
    lua_setmetatable(L, -2);
}

Fonts::Fonts() : defaultFont(0, {0,0}, {0,0}, {}) {}

bool Fonts::setupConfig(const char *filename) {
    if (!Game::locator().lua().loadObjectFile(filename)) {
        Game::locator().logger().print("Error loading fonts file!");
        Game::locator().logger().print(Game::locator().lua().getString());
        Game::locator().lua().pop();
        return false;
    }

    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Fonts file isn't a table!");
        Game::locator().lua().pop();
        return false;
    }
    Game::locator().lua().foreachInTable([this](unsigned index){
        const char* name = Game::locator().lua().getPathString({configName}, nullptr);
        if (!name) {
            Game::locator().logger().print("Font in %ud index doesn't have name", index);
            Game::locator().lua().pop();
            return;
        }

        const char* source = Game::locator().lua().getPathString({configSource}, nullptr);
        if (!source) {
            Game::locator().logger().print("Font in %ud index doesn't have source", index);
            Game::locator().lua().pop();
            return;
        }

        Point<unsigned> slices;
        long inputSlice = Game::locator().lua().getPathLong({configSlices, Lua::configX}, 1);
        if (inputSlice < 1) {
            Game::locator().logger().print("Font in %ud index:", index);
            Game::locator().logger().print("Slices must be more than zero: x = %ld", inputSlice);
            slices.x = 1;
        } else {
            slices.x = inputSlice;
        }
        inputSlice = Game::locator().lua().getPathLong({configSlices, Lua::configY}, 1);
        if (inputSlice < 1) {
            Game::locator().logger().print("Sprite in %ud index:", index);
            Game::locator().logger().print("Slices must be more than zero: y = %ld", inputSlice);
            slices.y = 1;
        } else {
            slices.y = inputSlice;
        }

        const char* letters = Game::locator().lua().getPathString({configLetters}, nullptr);
        if (!letters) {
            Game::locator().logger().print("Font in %ud index doesn't have letters", index);
            Game::locator().lua().pop();
            return;
        }

        load(name, source, letters, slices);
    });

    Game::locator().lua().pop();

    return true;
}

void Fonts::load(const char *name, const char *source, const char *letters, Point<unsigned> slices) {
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

        std::map<char, unsigned> lettersMap;
        for (unsigned i = 0; letters[i] != '\0'; i++) {
            lettersMap.insert(std::make_pair(letters[i], i + 1));
        }

        auto *font = pool.create(texture, Size<unsigned>(width / slices.x, height / slices.y), slices, lettersMap);
        fonts.insert(std::make_pair(name, font));
    }

    stbi_image_free(data);
}

Font &Fonts::get(const char *name) {
    auto iter = fonts.find(name);
    if (iter == fonts.end()) {
        return defaultFont;
    }
    return *iter->second;
}

void Fonts::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 0); // meta

    lua_createtable(L, 0, 8);

    luaL_Reg fontsFunc[] = {
            {"getFont", Fonts::L_getFont},
            {NULL, NULL},
    };
    luaL_register(L, NULL, fontsFunc);

    struct {
        const char *key;
        int value;
    } fonstConsts[] = {
            {"LEFT", (int)Font::ALIGNMENT ::LEFT},
            {"H_CENTER", (int)Font::ALIGNMENT ::H_CENTER},
            {"RIGHT", (int)Font::ALIGNMENT ::RIGHT},
            {"UP", (int)Font::ALIGNMENT ::UP},
            {"V_CENTER", (int)Font::ALIGNMENT ::V_CENTER},
            {"DOWN", (int)Font::ALIGNMENT ::DOWN},
            {"CENTER", (int)Font::ALIGNMENT ::CENTER},
            {NULL, -1},
    };

    auto *fonstConstsIter = fonstConsts;
    while (fonstConstsIter->key && fonstConstsIter->value != -1) {
        lua_pushinteger(L, fonstConstsIter->value);
        lua_setfield(L, -2, fonstConstsIter->key);
        fonstConstsIter++;
    }

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Fonts");
}

int Fonts::L_getFont(lua_State *L) {
    Game::locator().fonts().get(luaL_checkstring(L, 1)).pushToLua(L);
    return 1;
}