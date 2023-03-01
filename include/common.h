//
// Created by KILLdon on 26.05.2019.
//

#ifndef RIFLERANGE_COMMON_H
#define RIFLERANGE_COMMON_H

#include <cmath>

#include "lua_helper.h"

namespace Common {

    inline double toRad(double deg) {
        return deg * M_PI / 180;
    }

    inline double toDeg(double rad) {
        return rad * 180 / M_PI;
    }

    float cropRad(float rad);
    float cropDeg(float deg);

    float generateRad(float angle, float dispersion);
    float generateDeg(float angle, float dispersion);

    int generateInt(int min, int max);
    float generateFloat(float min, float max);

    int L_cropRad(lua_State *L);
    int L_cropDeg(lua_State *L);

    void setupLuaVar(lua_State *L);

}

#endif //RIFLERANGE_COMMON_H
