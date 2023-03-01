//
// Created by KILLdon on 01.06.2019.
//

#include "ini.h"

#include "game.h"
#include "lua_engine.h"

#include "minIni.h"

Ini::Ini() : iniFile(nullptr) {};

void Ini::setupIniFile(const char *filename) {
    iniFile = filename;
}

long Ini::getLong(const char *section, const char *key, long def) {
    if (iniFile && key && def) {
        return ini_getl(section, key, def, iniFile);
    }
    return def;
}

void Ini::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 0); // meta

    lua_createtable(L, 0, 2);

    luaL_Reg iniFunc[] = {
            {"setIniFile", Ini::L_setIniFile},
            {"getInteger", Ini::L_getInteger},
            {NULL, NULL},
    };
    luaL_register(L, NULL, iniFunc);

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Ini");
}

int Ini::L_setIniFile(lua_State *L) {
    Game::locator().ini().setupIniFile(luaL_checkstring(L, 1));
    return 0;
}

int Ini::L_getInteger(lua_State *L) {
    const char *section = nullptr;
    if (lua_isstring(L, 1)) {
        lua_tostring(L, 1);
    }
    lua_pushinteger(L, Game::locator().ini().getLong(
            section,
            luaL_checkstring(L, 2),
            luaL_checkinteger(L, 3)));
    return 1;
}