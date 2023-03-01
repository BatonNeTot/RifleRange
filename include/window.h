//
// Created by KILLdon on 26.05.2019.
//

#ifndef RIFLERANGE_WINDOW_H
#define RIFLERANGE_WINDOW_H

#include "locator.h"

#include "data.h"

#include "opengl_helper.h"
#include "lua_helper.h"

#include <sys/time.h>

class Window {
public:
    friend class Locator;
    friend class Input;
    friend class Render;
    friend class Game;

    bool isShouldClose();
    void closeQuery();


    enum class Mode {
        WINDOWED,
        FULLSCREEN
    };

    void setWindowMode(Window::Mode mode);
    Window::Mode getWindowMode();

    float getDelta();
    float getFPS();
    Size<unsigned> getRoom();

    // Lua

    static void setupLuaVar(lua_State *L);

private:
    Window();

    inline static const char *const configTitle = "title";
    inline static const char *const configSize = "size";
    inline static const char *const configRoom = "room";
    inline static const char *const configBackground = "background";

    bool setupConfig(const char *filename);

    GLFWwindow *id;
    Size<unsigned> size;
    Size<unsigned> room;
    Size<unsigned> view;
    Point<unsigned> offset;
    class Sprite *background;

    Mode mode;

    struct {
        GLFWmonitor *id;
        Point<int> position;
        Size<unsigned> resolution;
    } monitor;

    OpenGLVersion version;

    inline static const int fpsDeltaCount = 128;

    struct {
        timeval lastTime;
        int lastDeltaIndex;
        long deltaHistory[fpsDeltaCount];
        long sum;
        float delta;
        float fps;
    } fps;

    void init();
    void update();
    void updateFPS();

    void setupTitle(const char *title);
    void setupSize(Size<unsigned> size);
    void setupRoom(Size<unsigned> size);

    void setupViewport();
    void posInCenter();

    // Lua

    static int L_isShouldClose(lua_State *L);
    static int L_closeQuery(lua_State *L);

    static int L_setWindowMode(lua_State *L);
    static int L_getWindowMode(lua_State *L);

    static int L_getDelta(lua_State *L);
    static int L_getFPS(lua_State *L);
    static int L_getRoom(lua_State *L);
};

Window::Mode operator !(const Window::Mode &&mode);

#endif //RIFLERANGE_WINDOW_H
