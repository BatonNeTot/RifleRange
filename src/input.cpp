//
// Created by KILLdon on 26.05.2019.
//

#include "input.h"

#include "game.h"
#include "window.h"
#include "lua_engine.h"

#include <iostream>
#include <cmath>

void Input::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    switch (action) {
        case GLFW_PRESS: {
            Game::locator().input().lastKeyTouched = key;
            Game::locator().input().lastKeyTyped = key;

            Game::locator().input().keysTouched.insert(key);
            Game::locator().input().keysTyped.insert(key);
            Game::locator().input().keysPressed.insert(key);
            break;
        }
        case GLFW_REPEAT: {
            Game::locator().input().lastKeyTyped = key;

            Game::locator().input().keysTyped.insert(key);
            break;
        }
        case GLFW_RELEASE: {
            Game::locator().input().lastKeyReleased = key;

            Game::locator().input().keysPressed.erase(key);
            Game::locator().input().keysReleased.insert(key);
            break;
        }
    }
}

void Input::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    switch (action) {
        case GLFW_PRESS: {
            Game::locator().input().lastButtonTouched = button;

            Game::locator().input().buttonsTouched.insert(button);
            Game::locator().input().buttonsPressed.insert(button);
            break;
        }
        case GLFW_RELEASE: {
            Game::locator().input().lastButtonReleased = button;

            Game::locator().input().buttonsPressed.erase(button);
            Game::locator().input().buttonsReleased.insert(button);
            break;
        }
    }
}

void Input::mouseCursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    Game::locator().input().mouseX = xpos;
    Game::locator().input().mouseY = ypos;
}

void Input::mouseCursorEnterCallback(GLFWwindow *window, int entered) {
    if (!entered) {
        Game::locator().input().mouseX = NAN;
        Game::locator().input().mouseY = NAN;
    }
}

Point<float> Input::getMousePos() {
    if (mouseX != mouseX || mouseY != mouseY) {
        return {NAN, NAN};
    } else {
        return {
            (mouseX - Game::locator().window().offset.x) * Game::locator().window().room.width / Game::locator().window().view.width,
            (mouseY - Game::locator().window().offset.y) * Game::locator().window().room.height / Game::locator().window().view.height
        };
    }
}

void Input::init() {
    glfwSetKeyCallback(Game::locator().window().id, Input::keyCallback);
    glfwSetMouseButtonCallback(Game::locator().window().id, Input::mouseButtonCallback);
    glfwSetCursorPosCallback(Game::locator().window().id, Input::mouseCursorPosCallback);
    glfwSetCursorEnterCallback(Game::locator().window().id, Input::mouseCursorEnterCallback);
}

void Input::update() {
    lastKeyTouched = GLFW_KEY_UNKNOWN;
    lastKeyTyped = GLFW_KEY_UNKNOWN;
    lastKeyReleased = GLFW_KEY_UNKNOWN;

    keysTouched.clear();
    keysTyped.clear();
    keysReleased.clear();

    lastButtonTouched = GLFW_KEY_UNKNOWN;
    lastButtonReleased = GLFW_KEY_UNKNOWN;

    buttonsTouched.clear();
    buttonsReleased.clear();

    glfwPollEvents();
}

void Input::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 0); // meta

    lua_createtable(L, 0, 80);

    luaL_Reg inputFunc[] = {
            {"isKeyTouched", Input::L_isKeyTouched},
            {"isKeyTyped", Input::L_isKeyTyped},
            {"isKeyPressed", Input::L_isKeyPressed},
            {"isKeyReleased", Input::L_isKeyReleased},
            {"isAnyKeyTouched", Input::L_isAnyKeyTouched},
            {"isAnyKeyTyped", Input::L_isAnyKeyTyped},
            {"isAnyKeyPressed", Input::L_isAnyKeyPressed},
            {"isAnyKeyReleased", Input::L_isAnyKeyReleased},
            {"getLastKeyTouched", Input::L_getLastKeyTouched},
            {"getLastKeyTyped", Input::L_getLastKeyTyped},
            {"getLastKeyReleased", Input::L_getLastKeyReleased},

            {"getMousePos", Input::L_getMousePos},
            {"isMouseButtonTouched", Input::L_isMouseButtonTouched},
            {"isMouseButtonPressed", Input::L_isMouseButtonPressed},
            {"isMouseButtonReleased", Input::L_isMouseButtonReleased},
            {"isAnyMouseButtonTouched", Input::L_isAnyMouseButtonTouched},
            {"isAnyMouseButtonPressed", Input::L_isAnyMouseButtonPressed},
            {"isAnyMouseButtonReleased", Input::L_isAnyMouseButtonReleased},
            {"getLastMouseButtonTouched", Input::L_getLastMouseButtonTouched},
            {"getLastMouseButtonReleased", Input::L_getLastMouseButtonReleased},
            {NULL, NULL},
    };
    luaL_register(L, NULL, inputFunc);

    struct {
        const char *key;
        int value;
    } inputConsts[] = {
            {"KEY_ESCAPE", GLFW_KEY_ESCAPE},

            {"KEY_F1", GLFW_KEY_F1},
            {"KEY_F2", GLFW_KEY_F2},
            {"KEY_F3", GLFW_KEY_F3},
            {"KEY_F4", GLFW_KEY_F4},
            {"KEY_F5", GLFW_KEY_F5},
            {"KEY_F6", GLFW_KEY_F6},
            {"KEY_F7", GLFW_KEY_F7},
            {"KEY_F8", GLFW_KEY_F8},
            {"KEY_F9", GLFW_KEY_F9},
            {"KEY_F10", GLFW_KEY_F10},
            {"KEY_F11", GLFW_KEY_F11},
            {"KEY_F12", GLFW_KEY_F12},

            {"KEY_1", GLFW_KEY_1},
            {"KEY_2", GLFW_KEY_2},
            {"KEY_3", GLFW_KEY_3},
            {"KEY_4", GLFW_KEY_4},
            {"KEY_5", GLFW_KEY_5},
            {"KEY_6", GLFW_KEY_6},
            {"KEY_7", GLFW_KEY_7},
            {"KEY_8", GLFW_KEY_8},
            {"KEY_9", GLFW_KEY_9},
            {"KEY_0", GLFW_KEY_0},

            {"KEY_Q", GLFW_KEY_Q},
            {"KEY_W", GLFW_KEY_W},
            {"KEY_E", GLFW_KEY_E},
            {"KEY_R", GLFW_KEY_R},
            {"KEY_T", GLFW_KEY_T},
            {"KEY_Y", GLFW_KEY_Y},
            {"KEY_U", GLFW_KEY_U},
            {"KEY_I", GLFW_KEY_I},
            {"KEY_O", GLFW_KEY_O},
            {"KEY_P", GLFW_KEY_P},
            {"KEY_A", GLFW_KEY_A},
            {"KEY_S", GLFW_KEY_S},
            {"KEY_D", GLFW_KEY_D},
            {"KEY_F", GLFW_KEY_F},
            {"KEY_G", GLFW_KEY_G},
            {"KEY_H", GLFW_KEY_H},
            {"KEY_J", GLFW_KEY_J},
            {"KEY_K", GLFW_KEY_K},
            {"KEY_L", GLFW_KEY_L},
            {"KEY_Z", GLFW_KEY_Z},
            {"KEY_X", GLFW_KEY_X},
            {"KEY_C", GLFW_KEY_C},
            {"KEY_V", GLFW_KEY_V},
            {"KEY_B", GLFW_KEY_B},
            {"KEY_N", GLFW_KEY_N},
            {"KEY_M", GLFW_KEY_M},

            {"MOUSE_BUTTON_1", GLFW_MOUSE_BUTTON_1},
            {"MOUSE_BUTTON_2", GLFW_MOUSE_BUTTON_2},
            {"MOUSE_BUTTON_3", GLFW_MOUSE_BUTTON_3},
            {"MOUSE_BUTTON_4", GLFW_MOUSE_BUTTON_4},
            {"MOUSE_BUTTON_5", GLFW_MOUSE_BUTTON_5},
            {"MOUSE_BUTTON_6", GLFW_MOUSE_BUTTON_6},
            {"MOUSE_BUTTON_7", GLFW_MOUSE_BUTTON_7},
            {"MOUSE_BUTTON_8", GLFW_MOUSE_BUTTON_8},
            {"MOUSE_BUTTON_LEFT", GLFW_MOUSE_BUTTON_LEFT},
            {"MOUSE_BUTTON_RIGHT", GLFW_MOUSE_BUTTON_RIGHT},
            {"MOUSE_BUTTON_MIDDLE", GLFW_MOUSE_BUTTON_MIDDLE},

            {NULL, -1},
    };

    auto *inputConstsIter = inputConsts;
    while (inputConstsIter->key && inputConstsIter->value != -1) {
        lua_pushinteger(L, inputConstsIter->value);
        lua_setfield(L, -2, inputConstsIter->key);
        inputConstsIter++;
    }

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Input");
}

int Input::L_isKeyTouched(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isKeyTouched(luaL_checkinteger(L, 1)));
    return 1;
}
int Input::L_isKeyTyped(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isKeyTyped(luaL_checkinteger(L, 1)));
    return 1;
}
int Input::L_isKeyPressed(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isKeyPressed(luaL_checkinteger(L, 1)));
    return 1;
}
int Input::L_isKeyReleased(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isKeyReleased(luaL_checkinteger(L, 1)));
    return 1;
}

int Input::L_isAnyKeyTouched(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isAnyKeyTouched());
    return 1;
}
int Input::L_isAnyKeyTyped(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isAnyKeyTyped());
    return 1;
}
int Input::L_isAnyKeyPressed(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isAnyKeyPressed());
    return 1;
}
int Input::L_isAnyKeyReleased(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isAnyKeyReleased());
    return 1;
}

int Input::L_getLastKeyTouched(lua_State *L) {
    int key = Game::locator().input().getLastKeyTouched();
    if (key != GLFW_KEY_UNKNOWN) {
        lua_pushinteger(L, key);
    } else {
        lua_pushnil(L);
    }
    return 1;
}
int Input::L_getLastKeyTyped(lua_State *L) {
    int key = Game::locator().input().getLastKeyTyped();
    if (key != GLFW_KEY_UNKNOWN) {
        lua_pushinteger(L, key);
    } else {
        lua_pushnil(L);
    }
    return 1;
}
int Input::L_getLastKeyReleased(lua_State *L) {
    int key = Game::locator().input().getLastKeyReleased();
    if (key != GLFW_KEY_UNKNOWN) {
        lua_pushinteger(L, key);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

int Input::L_getMousePos(lua_State *L) {
    auto &&pos = Game::locator().input().getMousePos();
    if (pos.x != pos.x || pos.y != pos.y) {
        lua_pushnil(L);
        lua_pushnil(L);
    } else {
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
    }
    return 2;
}

int Input::L_isMouseButtonTouched(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isMouseButtonTouched(luaL_checkinteger(L, 1)));
    return 1;
}
int Input::L_isMouseButtonPressed(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isMouseButtonPressed(luaL_checkinteger(L, 1)));
    return 1;
}
int Input::L_isMouseButtonReleased(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isMouseButtonReleased(luaL_checkinteger(L, 1)));
    return 1;
}

int Input::L_isAnyMouseButtonTouched(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isAnyMouseButtonTouched());
    return 1;
}
int Input::L_isAnyMouseButtonPressed(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isAnyMouseButtonPressed());
    return 1;
}
int Input::L_isAnyMouseButtonReleased(lua_State *L) {
    lua_pushboolean(L, Game::locator().input().isAnyMouseButtonReleased());
    return 1;
}

int Input::L_getLastMouseButtonTouched(lua_State *L) {
    int key = Game::locator().input().getLastMouseButtonTouched();
    if (key != GLFW_KEY_UNKNOWN) {
        lua_pushinteger(L, key);
    } else {
        lua_pushnil(L);
    }
    return 1;
}
int Input::L_getLastMouseButtonReleased(lua_State *L) {
    int key = Game::locator().input().getLastMouseButtonReleased();
    if (key != GLFW_KEY_UNKNOWN) {
        lua_pushinteger(L, key);
    } else {
        lua_pushnil(L);
    }
    return 1;
}
