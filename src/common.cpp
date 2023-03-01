//
// Created by KILLdon on 01.06.2019.
//

#include "common.h"

#include "lua_engine.h"

#include <cstdlib>

namespace Common {

    float cropRad(float rad) {
        while (rad >= 2 * M_PI) rad -= 2 * M_PI;
        while (rad < 0) rad += 2 * M_PI;
        return rad;
    }

    float cropDeg(float deg) {
        while (deg >= 360) deg -= 360;
        while (deg < 0) deg += 360;
        return deg;
    }

    float generateRad(float angle, float dispersion) {
        return cropRad(angle - dispersion / 2 + rand() / ((float)RAND_MAX / dispersion));
    }

    float generateDeg(float angle, float dispersion) {
        return cropDeg(angle - dispersion / 2 + rand() / ((float)RAND_MAX / dispersion));
    }

    int generateInt(int min, int max) {
        return min + (rand() % (1 + max - min));
    }

    float generateFloat(float min, float max) {
        return min + rand() / ((float)RAND_MAX / (max - min));
    }

    int L_cropRad(lua_State *L) {
        lua_pushnumber(L, cropRad(luaL_checknumber(L, 1)));
        return 1;
    }

    int L_cropDeg(lua_State *L) {
        lua_pushnumber(L, cropDeg(luaL_checknumber(L, 1)));
        return 1;
    }

    void setupLuaVar(lua_State *L) {
        lua_createtable(L, 0, 0);
        lua_createtable(L, 0, 0); // meta

        lua_createtable(L, 0, 2);

        luaL_Reg commonFunc[] = {
                {"cropRad", L_cropRad},
                {"cropDeg", L_cropDeg},
                {NULL, NULL},
        };
        luaL_register(L, NULL, commonFunc);

        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, Lua::emptyFunction);
        lua_setfield(L, -2, "__newindex");

        lua_setmetatable(L, -2);

        lua_setfield(L, -2, "Common");
    }

}