//
// Created by KILLdon on 29.05.2019.
//

#include "lua_engine.h"

#include "logger.h"
#include "game.h"
#include "font.h"
#include "sound.h"
#include "particles.h"
#include "entity.h"
#include "input.h"
#include "common.h"
#include "ini.h"

#include <iostream>

Lua::Lua() : L(luaL_newstate()) {
    luaL_openlibs(L);

    Sprite::setupMeta(L);
    Sound::setupMeta(L);
    Player::setupMeta(L);
    ParticleGenerator::setupMeta(L);
    Font::setupMeta(L);
    Entity::setupMeta(L);
}

void Lua::setupGlobal() {
    luaL_newmetatable(L, metaGlobal);
    luaL_newmetatable(L, metaGlobalIndex);

    Input::setupLuaVar(L);
    Window::setupLuaVar(L);
    Sprites::setupLuaVar(L);
    SoundMaster::setupLuaVar(L);
    ParticleMaster::setupLuaVar(L);
    Fonts::setupLuaVar(L);
    Entities::setupLuaVar(L);
    Ini::setupLuaVar(L);
    Common::setupLuaVar(L);

    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, LUA_GLOBALSINDEX);
}

void Lua::cleanEntityEnvironment(Entity &entity) {
    previousEnvironmentEntities = std::stack<Entity *>();

    luaL_newmetatable(L, metaGlobalIndex);

    entity.pushToLua(L);
    lua_setfield(L, -2, Entity::varEntity);

    lua_pushnil(L);
    lua_setfield(L, -2, Entity::varThat);

    luaL_newmetatable(L, Entity::storage);
    lua_pushlightuserdata(L, &entity);
    lua_gettable(L, -2);
    lua_setfield(L, -3, Entity::varStorage);

    lua_pop(L, 2);

    previousEnvironmentEntities.push(&entity);
}

void Lua::prepareEntityEnvironment(Entity &entity) {
    luaL_newmetatable(L, metaGlobalIndex);

    entity.pushToLua(L);
    lua_setfield(L, -2, Entity::varEntity);

    previousEnvironmentEntities.top()->pushToLua(L);
    lua_setfield(L, -2, Entity::varThat);

    luaL_newmetatable(L, Entity::storage);
    lua_pushlightuserdata(L, &entity);
    lua_gettable(L, -2);
    lua_setfield(L, -3, Entity::varStorage);

    lua_pop(L, 2);

    previousEnvironmentEntities.push(&entity);
}

void Lua::restoreEntityEnvironment() {
    previousEnvironmentEntities.pop();

    Entity *entity = previousEnvironmentEntities.top();
    previousEnvironmentEntities.pop();

    luaL_newmetatable(L, metaGlobalIndex);

    entity->pushToLua(L);
    lua_setfield(L, -2, Entity::varEntity);

    if (previousEnvironmentEntities.size() > 0) {
        previousEnvironmentEntities.top()->pushToLua(L);
    } else {
        lua_pushnil(L);
    }
    lua_setfield(L, -2, Entity::varThat);

    luaL_newmetatable(L, Entity::storage);
    lua_pushlightuserdata(L, entity);
    lua_gettable(L, -2);
    lua_setfield(L, -3, Entity::varStorage);

    lua_pop(L, 2);

    previousEnvironmentEntities.push(entity);
}


void Lua::evalOnCreate(Entity &entity) {
    luaL_newmetatable(L, Entity::onCreate);
    lua_pushlightuserdata(L, &entity);
    lua_gettable(L, -2);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0)) {
            Game::locator().logger().print("Error in onStart!");
            Game::locator().logger().print(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

void Lua::evalOnStep(Entity &entity) {
    luaL_newmetatable(L, Entity::onStep);
    lua_pushlightuserdata(L, &entity);
    lua_gettable(L, -2);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0)) {
            Game::locator().logger().print("Error in onStep!");
            Game::locator().logger().print(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

int Lua::emptyFunction(lua_State *L) {
    return 0;
}

bool Lua::loadObjectFile(const char *filename) {
    int result = luaL_loadfile(L, filename);

    if (result) {
        return false;
    }

    result = lua_pcall(L, 0, 1, 0);

    return !result;
}

long Lua::getPathLong(std::vector<const char *> fields, long defaultValue) {
    int pop = 0;
    for (auto field : fields) {
        if (!lua_istable(L, -1)) {
            lua_pop(L, pop);
            return defaultValue;
        }

        pop++;
        lua_getfield(L, -1, field);
    }

    long result;
    if (!lua_isnumber(L, -1)) {
        result = defaultValue;
    } else {
        result = lua_tointeger(L, -1);
    }

    lua_pop(L, pop);

    return result;
}

double Lua::getPathDouble(std::vector<const char *> fields, double defaultValue) {
    int pop = 0;
    for (auto field : fields) {
        if (!lua_istable(L, -1)) {
            lua_pop(L, pop);
            return defaultValue;
        }

        pop++;
        lua_getfield(L, -1, field);
    }

    double result;
    if (!lua_isnumber(L, -1)) {
        result = defaultValue;
    } else {
        result = lua_tonumber(L, -1);
    }

    lua_pop(L, pop);

    return result;
}

const char *Lua::getPathString(std::vector<const char *> fields, const char *defaultValue) {
    int pop = 0;
    for (auto field : fields) {
        if (!lua_istable(L, -1)) {
            lua_pop(L, pop);
            return defaultValue;
        }

        pop++;
        lua_getfield(L, -1, field);
    }

    const char *result;
    if (!lua_isstring(L, -1)) {
        result = defaultValue;
    } else {
        result = lua_tostring(L, -1);
    }

    lua_pop(L, pop);

    return result;
}

void Lua::printTable(lua_State *L) {
    Game::locator().logger().print("Printing table\n");
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_isstring(L, -2)) {
            Game::locator().logger().print("'%s'", lua_tostring(L, -2));
        } else if (lua_islightuserdata(L, -2)) {
            Game::locator().logger().print("#%p", lua_touserdata(L, -2));
        }  else {
            Game::locator().logger().print("[<%s>]", lua_typename(L, lua_type(L, -2)));
        }
        Game::locator().logger().print(" -> ");
        if (lua_isfunction(L, -1)) {
            Game::locator().logger().print("function#%p", lua_topointer(L, -1));
        } else {
            Game::locator().logger().print("<%s>", lua_typename(L, lua_type(L, -1)));
        }
        Game::locator().logger().print("\n");
        lua_pop(L, 1);
    }
}

void Lua::printStack(lua_State *L) {
    Game::locator().logger().print("Printing stack\n");
    auto top = lua_gettop(L);
    while (top > 0) {
        if (lua_isstring(L, -top)) {
            Game::locator().logger().print("'%s'\n", lua_tostring(L, -top));
        } else {
            Game::locator().logger().print("<%s>\n", lua_typename(L, lua_type(L, -top)));
        }
        top--;
    }
    Game::locator().logger().print("------");
}

const char *Lua::getTypename() {
    return lua_typename(L, lua_type(L, -1));
}

void Lua::getField(const char *field) {
    lua_getfield(L, -1, field);
}

int Lua::stackSize() {
    return lua_gettop(L);
}

void Lua::pop() {
    lua_pop(L, 1);
}

bool Lua::isTable() {
    return lua_istable(L, -1);
}

const char *Lua::getString() {
    const char *str = nullptr;
    if (lua_gettop(L) > 0 && lua_isstring(L, -1)) {
        str = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    return str;
}

void Lua::foreachInTable(std::function<void(unsigned)> action) {
    unsigned index = 1;

    while(true) {
        lua_pushinteger(L, index);
        lua_gettable(L, -2);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            return;
        }
        action(index++);
        lua_pop(L, 1);
    }
}
void Lua::createStorage(Entity &entity) {
    luaL_newmetatable(L, Entity::storage);
    lua_pushlightuserdata(L, &entity);
    lua_createtable(L, 0,0);
    lua_settable(L, -3);
    pop();
}
void Lua::attachToStorage(Entity &entity, const char *storageField, const char *configField) {
    luaL_newmetatable(L, storageField);
    lua_pushlightuserdata(L, &entity);
    lua_getfield(L, -3, configField);
    lua_settable(L, -3);
    pop();
}
void Lua::reattachInStorage(Entity &prototype, Entity &entity, const char *storageField) {
    luaL_newmetatable(L, storageField);
    lua_pushlightuserdata(L, &entity);
    lua_pushlightuserdata(L, &prototype);
    lua_gettable(L, -3);
    lua_settable(L, -3);
    pop();
}
void Lua::deattachInStorage(Entity &entity, const char *storageField) {
    luaL_newmetatable(L, storageField);
    lua_pushlightuserdata(L, &entity);
    lua_pushnil(L);
    lua_settable(L, -3);
    pop();
}
