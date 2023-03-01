//
// Created by KILLdon on 29.05.2019.
//

#ifndef RIFLERANGE_LUA_ENGINE_H
#define RIFLERANGE_LUA_ENGINE_H

#include "locator.h"

#include "lua_helper.h"
#include "entity.h"

#include <functional>
#include <vector>
#include <list>
#include <iostream>
#include <stack>

class Lua {
public:
    friend class Locator;
    friend class Game;

    inline static const char *configWidth = "width";
    inline static const char *configHeight = "height";

    inline static const char *configX = "x";
    inline static const char *configY = "y";

    static int emptyFunction(lua_State *L);

    bool loadObjectFile(const char *filename);

    long getPathLong(std::vector<const char *> fields, long defaultValue);
    double getPathDouble(std::vector<const char *> fields, double defaultValue);
    const char *getPathString(std::vector<const char *> fields, const char *defaultValue);

    static void printTable(lua_State *L);
    static void printStack(lua_State *L);

    const char *getTypename();
    int stackSize();
    void pop();

    bool isTable();

    void getField(const char* field);
    const char *getString();

    void foreachInTable(std::function<void(unsigned)> action);

    // Entity

    void createStorage(Entity &entity);
    void attachToStorage(Entity &entity, const char *storageField, const char *configField);
    void reattachInStorage(Entity &prototype, Entity &entity, const char *storageField);
    void deattachInStorage(Entity &entity, const char *storageField);

    void cleanEntityEnvironment(Entity &entity);
    void prepareEntityEnvironment(Entity &entity);
    void restoreEntityEnvironment();

    void evalOnCreate(Entity &entity);
    void evalOnStep(Entity &entity);

private:
    Lua();

    lua_State *L;

    void setupGlobal();

    std::stack<Entity *> previousEnvironmentEntities;

    inline static const char * const metaGlobal = "metatable.global";
    inline static const char * const metaGlobalIndex = "metatable.global.index";
};

#endif //RIFLERANGE_LUA_ENGINE_H
