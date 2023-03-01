//
// Created by KILLdon on 26.05.2019.
//

#include "window.h"

#include "game.h"
#include "logger.h"
#include "sprite.h"
#include "lua_engine.h"

static void windowErrorCallback(int code, const char* message) {
    Game::locator().logger().print("Error with OpenGL %d: %s\n", code, message);
}

Window::Window() : mode(Mode::WINDOWED) {}

bool Window::setupConfig(const char *filename) {
    if (!Game::locator().lua().loadObjectFile(filename)) {
        Game::locator().logger().print("Error loading window file!");
        Game::locator().logger().print(Game::locator().lua().getString());
        Game::locator().lua().pop();
        return false;
    }

    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Window file isn't a table!");
        Game::locator().lua().pop();
        return false;
    }

    const char* title = Game::locator().lua().getPathString({configTitle}, nullptr);
    if (title) {
        setupTitle(title);
    }

    Size<unsigned> size;
    long sizeInput = Game::locator().lua().getPathLong({configSize, Lua::configWidth}, 0);
    if (sizeInput < 0) {
        Game::locator().logger().print("Window width must be more than zero; width = %ld", sizeInput);
        Game::locator().lua().pop();
        return false;
    }
    size.width = sizeInput;
    sizeInput = Game::locator().lua().getPathLong({configSize, Lua::configHeight}, 0);
    if (sizeInput < 0) {
        Game::locator().logger().print("Window height must be more than zero; height = %ld", sizeInput);
        Game::locator().lua().pop();
        return false;
    }
    size.height = sizeInput;
    setupSize(size);

    const char *backgroundSource = Game::locator().lua().getPathString({configRoom, configBackground}, "");
    background = &Game::locator().sprites().get(backgroundSource);

    Size<unsigned> room;
    long roomInput = Game::locator().lua().getPathLong({configRoom, Lua::configWidth}, 0);
    if (roomInput < 0) {
        Game::locator().logger().print("Room width must be more than zero; width = %ld", roomInput);
        Game::locator().lua().pop();
        return false;
    }
    room.width = roomInput;
    roomInput = Game::locator().lua().getPathLong({configRoom, Lua::configHeight}, 0);
    if (roomInput < 0) {
        Game::locator().logger().print("Room height must be more than zero; height = %ld", roomInput);
        Game::locator().lua().pop();
        return false;
    }
    room.height = roomInput;
    setupRoom(room);


    Game::locator().lua().pop();

    return true;
}

void Window::init() {
    glfwSetErrorCallback(windowErrorCallback);

    if (!glfwInit()) {
        Game::locator().logger().print("Failed to init GLFW\n");
        exit(-1);
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    /* Test window for setting up GL version */
    GLFWwindow *windowTest = glfwCreateWindow(1, 1, "Tmp window", NULL, NULL);
    if (!windowTest) {
        Game::locator().logger().print("Failed to create test GLFW window\n");
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(windowTest);

    version.major = glfwGetWindowAttrib(windowTest, GLFW_CONTEXT_VERSION_MAJOR);
    version.minor = glfwGetWindowAttrib(windowTest, GLFW_CONTEXT_VERSION_MINOR );
    version.rev = glfwGetWindowAttrib(windowTest, GLFW_CONTEXT_REVISION);

    Game::locator().logger().print("OpenGL version: %d.%d.%d\n", version.major, version.minor, version.rev);

    if (version.major < 4 || version.minor < 1) {
        Game::locator().logger().print("Error: OpenGL version is too low, need 4.1.0 or higher\n");
        glfwTerminate();
        exit(-1);
    }

    glfwDestroyWindow(windowTest);

    /* Done */

    size = {1, 1};
    room = {1, 1};
    id = glfwCreateWindow(size.width, size.height, "", NULL, NULL);
    if (!id) {
        Game::locator().logger().print("Failed to create GLFW window\n");
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(id);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Game::locator().logger().print("Failed to initialize GLAD\n");
        glfwTerminate();
        exit(-1);
    }

    monitor.id = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor.id, &monitor.position.x, &monitor.position.y);
    const GLFWvidmode *vidmode = glfwGetVideoMode(monitor.id);
    monitor.resolution.width = vidmode->width;
    monitor.resolution.height = vidmode->height;

    posInCenter();
    setupViewport();

    gettimeofday(&fps.lastTime, nullptr);
    fps.lastDeltaIndex = fpsDeltaCount - 1;
    for (long &delta : fps.deltaHistory) {
        delta = 0;
    }
    fps.sum = 0;
    fps.delta = 0;
    fps.fps = 0;

    glfwShowWindow(id);
}

void Window::update() {
    updateFPS();
    glfwSwapBuffers(id);
}

static const unsigned nanoInSeconds = 1000000;

void Window::updateFPS() {
    timeval currentTime;
    gettimeofday(&currentTime, nullptr);

    ++fps.lastDeltaIndex %= fpsDeltaCount;

    long currentDelta = (currentTime.tv_sec - fps.lastTime.tv_sec) * nanoInSeconds + (currentTime.tv_usec - fps.lastTime.tv_usec);

    fps.lastTime = currentTime;

    fps.sum += currentDelta - fps.deltaHistory[fps.lastDeltaIndex];
    fps.deltaHistory[fps.lastDeltaIndex] = currentDelta;
    fps.delta = (float)currentDelta / nanoInSeconds;

    fps.fps = (float)fpsDeltaCount * nanoInSeconds / fps.sum;
}

float Window::getDelta() {
    return fps.delta;
}

float Window::getFPS() {
    return fps.fps;
}

bool Window::isShouldClose() {
    return glfwWindowShouldClose(id);
}

void Window::closeQuery() {
    glfwSetWindowShouldClose(id, true);
}

void Window::setupSize(Size<unsigned> size) {
    if (size.width > 0 && size.height > 0) {
        this->size = size;
        glfwSetWindowSize(id, size.width, size.height);
        glfwSetWindowAttrib(id, GLFW_DECORATED, GLFW_TRUE);
        setupViewport();
        posInCenter();
    }
}

void Window::posInCenter() {
    glfwSetWindowPos(id,
                     monitor.position.x + (monitor.resolution.width - size.width) / 2,
                     monitor.position.y + (monitor.resolution.height - size.height) / 2);
}

void Window::setupTitle(const char *title) {
    glfwSetWindowTitle(id, title);
}

void Window::setWindowMode(Window::Mode mode) {
    if (this->mode != mode) {
        switch (mode) {
            case Mode::WINDOWED: {
                glfwSetWindowMonitor(id, nullptr,
                                     monitor.position.x + (monitor.resolution.width - size.width) / 2,
                                     monitor.position.y + (monitor.resolution.height - size.height) / 2,
                                     size.width,
                                     size.height,
                                     GLFW_DONT_CARE);
                glfwSetWindowAttrib(id, GLFW_DECORATED, GLFW_TRUE);
                break;
            }
            case Mode::FULLSCREEN: {
                glfwSetWindowMonitor(id, nullptr,
                                     monitor.position.x,
                                     monitor.position.y,
                                     monitor.resolution.width,
                                     monitor.resolution.height,
                                     GLFW_DONT_CARE);
                glfwSetWindowAttrib(id, GLFW_DECORATED, GLFW_FALSE);
                break;
            }
        }
        this->mode = mode;
        setupViewport();
    }
}

Window::Mode Window::getWindowMode() {
    return mode;
}

void Window::setupRoom(Size<unsigned> size) {
    if (size.width > 0 && size.height > 0) {
        this->room = size;
        setupViewport();
    }
}

Size<unsigned> Window::getRoom() {
    return room;
}

void Window::setupViewport() {
    auto output = mode == Window::Mode::WINDOWED ? size : monitor.resolution;

    auto outputScale = (float)output.width / output.height;
    auto roomScale = (float)room.width / room.height;

    if (outputScale > roomScale) {
        auto scale = (float)output.height / room.height;
        view.height = output.height;
        view.width = (int)(room.width * scale);

        offset.x = ((output.width - view.width) / 2);
    } else {
        auto scale = (float)output.width / room.width;
        view.width = output.width;
        view.height = (int)(room.height * scale);

        offset.y = ((output.height - view.height) / 2);
    }

    glViewport(offset.x, offset.y, view.width, view.height);
}

Window::Mode operator !(const Window::Mode &&mode) {
    return (mode == Window::Mode::FULLSCREEN ? Window::Mode::WINDOWED : Window::Mode::FULLSCREEN);
}

void Window::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 0); // meta

    lua_createtable(L, 0, 9);

    luaL_Reg windowFunc[] = {
            {"isShouldClose", Window::L_isShouldClose},
            {"closeQuery", Window::L_closeQuery},
            {"setWindowMode", Window::L_setWindowMode},
            {"getWindowMode", Window::L_getWindowMode},
            {"getDelta", Window::L_getDelta},
            {"getFPS", Window::L_getFPS},
            {"getRoom", Window::L_getRoom},
            {NULL, NULL},
    };
    luaL_register(L, NULL, windowFunc);

    struct {
        const char *key;
        int value;
    } windowConsts[] = {
            {"MODE_WINDOWED", (int)Mode::WINDOWED},
            {"MODE_FULLSCREEN", (int)Mode::FULLSCREEN},
            {NULL, -1},
    };

    auto *windowConstsIter = windowConsts;
    while (windowConstsIter->key && windowConstsIter->value != -1) {
        lua_pushinteger(L, windowConstsIter->value);
        lua_setfield(L, -2, windowConstsIter->key);
        windowConstsIter++;
    }

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Window");
}

int Window::L_isShouldClose(lua_State *L) {
    lua_pushboolean(L, Game::locator().window().isShouldClose());
    return 1;
}
int Window::L_closeQuery(lua_State *L) {
    Game::locator().window().closeQuery();
    return 0;
}

int Window::L_setWindowMode(lua_State *L) {
    Game::locator().window().setWindowMode((Mode)luaL_checkinteger(L, 1));
    return 0;
}
int Window::L_getWindowMode(lua_State *L) {
    lua_pushinteger(L, (int)Game::locator().window().getWindowMode());
    return 1;
}

int Window::L_getDelta(lua_State *L) {
    lua_pushnumber(L, Game::locator().window().getDelta());
    return 1;
}
int Window::L_getFPS(lua_State *L) {
    lua_pushnumber(L, Game::locator().window().getFPS());
    return 1;

}
int Window::L_getRoom(lua_State *L) {
    auto &&room = Game::locator().window().getRoom();
    lua_pushnumber(L, room.width);
    lua_pushnumber(L, room.height);
    return 2;
}