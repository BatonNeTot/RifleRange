//
// Created by KILLdon on 01.06.2019.
//

#ifndef RIFLERANGE_INI_H
#define RIFLERANGE_INI_H

#include "locator.h"

#include "lua_helper.h"

class Ini {
public:

    void setupIniFile(const char *filename);

    long getLong(const char *section, const char *key, long def);

    // Lua

    static void setupLuaVar(lua_State *L);

private:

    Ini();

    friend class Locator;

    const char *iniFile;

    // Lua

    static int L_setIniFile(lua_State *L);
    static int L_getInteger(lua_State *L);
};

#endif //RIFLERANGE_INI_H
