//
// Created by KILLdon on 26.05.2019.
//

#ifndef RIFLERANGE_INPUT_H
#define RIFLERANGE_INPUT_H

#include "locator.h"

#include "opengl_helper.h"
#include "lua_helper.h"
#include "data.h"

#include <set>

class Input {
public:

    inline bool isKeyTouched(int key) { return keysTouched.find(key) != keysTouched.end(); }
    inline bool isKeyTyped(int key) { return keysTyped.find(key) != keysTyped.end(); }
    inline bool isKeyPressed(int key) { return keysPressed.find(key) != keysPressed.end(); }
    inline bool isKeyReleased(int key) { return keysReleased.find(key) != keysReleased.end(); }

    inline bool isAnyKeyTouched() { return !keysTouched.empty(); }
    inline bool isAnyKeyTyped() { return !keysTyped.empty(); }
    inline bool isAnyKeyPressed() { return !keysPressed.empty(); }
    inline bool isAnyKeyReleased() { return !keysReleased.empty(); }

    inline int getLastKeyTouched() { return lastKeyTouched; }
    inline int getLastKeyTyped() { return lastKeyTyped; }
    inline int getLastKeyReleased() { return lastKeyReleased; }

    Point<float> getMousePos();

    inline bool isMouseButtonTouched(int key) { return buttonsTouched.find(key) != buttonsTouched.end(); }
    inline bool isMouseButtonPressed(int key) { return buttonsPressed.find(key) != buttonsPressed.end(); }
    inline bool isMouseButtonReleased(int key) { return buttonsReleased.find(key) != buttonsReleased.end(); }

    inline bool isAnyMouseButtonTouched() { return !buttonsTouched.empty(); }
    inline bool isAnyMouseButtonPressed() { return !buttonsPressed.empty(); }
    inline bool isAnyMouseButtonReleased() { return !buttonsReleased.empty(); }

    inline int getLastMouseButtonTouched() { return lastButtonTouched; }
    inline int getLastMouseButtonReleased() { return lastButtonReleased; }

    // Lua

    static void setupLuaVar(lua_State *L);

    static int L_isKeyTouched(lua_State *L);
    static int L_isKeyTyped(lua_State *L);
    static int L_isKeyPressed(lua_State *L);
    static int L_isKeyReleased(lua_State *L);

    static int L_isAnyKeyTouched(lua_State *L);
    static int L_isAnyKeyTyped(lua_State *L);
    static int L_isAnyKeyPressed(lua_State *L);
    static int L_isAnyKeyReleased(lua_State *L);

    static int L_getLastKeyTouched(lua_State *L);
    static int L_getLastKeyTyped(lua_State *L);
    static int L_getLastKeyReleased(lua_State *L);

    static int L_getMousePos(lua_State *L);

    static int L_isMouseButtonTouched(lua_State *L);
    static int L_isMouseButtonPressed(lua_State *L);
    static int L_isMouseButtonReleased(lua_State *L);

    static int L_isAnyMouseButtonTouched(lua_State *L);
    static int L_isAnyMouseButtonPressed(lua_State *L);
    static int L_isAnyMouseButtonReleased(lua_State *L);

    static int L_getLastMouseButtonTouched(lua_State *L);
    static int L_getLastMouseButtonReleased(lua_State *L);

private:
    Input() = default;

    friend class Locator;
    friend class Game;

    void init();
    void update();

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void mouseCursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void mouseCursorEnterCallback(GLFWwindow *window, int entered);

    std::set<int> keysTouched;
    std::set<int> keysTyped;
    std::set<int> keysPressed;
    std::set<int> keysReleased;

    int lastKeyTouched;
    int lastKeyTyped;
    int lastKeyReleased;

    float mouseX;
    float mouseY;

    std::set<int> buttonsTouched;
    std::set<int> buttonsPressed;
    std::set<int> buttonsReleased;

    int lastButtonTouched;
    int lastButtonReleased;
};

#endif //RIFLERANGE_INPUT_H
