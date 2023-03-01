//
// Created by KILLdon on 28.05.2019.
//

#ifndef RIFLERANGE_ENTITY_H
#define RIFLERANGE_ENTITY_H

#include "locator.h"
#include "sprite.h"

#include "glm/matrix.hpp"
#include "lua_helper.h"

#include <map>
#include <string>
#include <set>

class Entity {
public:
    friend class Game;

    void setName(const char *name);
    const char *getName() const;

    void setType(const char *type);
    const char *getType() const;

    void setSprite(Sprite &sprite);
    Sprite &getSprite() const;

    void setIndex(unsigned spriteIndex);
    unsigned int getIndex() const;

    void setDepth(short depth);
    short getDepth() const;

    void setScale(Point<float> scale);
    Point<float> getScale() const;

    void setPosition(Point<float> position);
    Point<float> getPosition() const;

    void setRotation(float rotation);
    float getRotation() const;

    const glm::mat3 &getTransform();

    // Lua

    inline static const char * const metatable = "metatable.entity";
    inline static const char * const storage = "metatable.entity.storage";
    inline static const char * const onCreate = "metatable.entity.onCreate";
    inline static const char * const onStep = "metatable.entity.onStep";

    inline static const char * const varEntity = "entity";
    inline static const char * const varThat = "that";
    inline static const char * const varStorage = "storage";
    static void setupMeta(lua_State *L);

    static Entity &getFromLua(lua_State *L, int index);
    void pushToLua(lua_State *L);

private:
    typedef ObjectPool<Entity, 64> Pool;

    friend Pool;
    friend class Entities;
    friend class Lua;

    Entity();
    Entity(const Entity &entity);

    std::string name;
    const char *type;

    Sprite *sprite;
    unsigned spriteIndex;
    short depth;

    Point<float> scale;
    Point<float> position;
    float rotation;

    bool dirtyMatrix;
    glm::mat3 transform;

    // Lua

    static int L_setName(lua_State *L);
    static int L_getName(lua_State *L);

    static int L_setType(lua_State *L);
    static int L_getType(lua_State *L);

    static int L_setSprite(lua_State *L);
    static int L_getSprite(lua_State *L);

    static int L_setIndex(lua_State *L);
    static int L_getIndex(lua_State *L);

    static int L_setDepth(lua_State *L);
    static int L_getDepth(lua_State *L);

    static int L_setScale(lua_State *L);
    static int L_getScale(lua_State *L);

    static int L_setPosition(lua_State *L);
    static int L_getPosition(lua_State *L);

    static int L_setRotation(lua_State *L);
    static int L_getRotation(lua_State *L);

    static int L_getStorage(lua_State *L);
    static int L_equal(lua_State *L);
};

class Entities {
public:

    friend class Locator;
    friend class Entity;
    friend class Game;
    friend class Render;

    Entity &createEmpty();
    Entity &create(Entity &prototype);
    Entity &create(const char *name);

    void destroy(Entity &entity);
    void clearDestroying();

    void prototype(const char *name, Entity &entity);

    // Lua

    static void setupLuaVar(lua_State *L);

private:
    Entities() = default;

    inline static const char * const configPrototypes = "prototypes";

    inline static const char * const configName = "name";
    inline static const char * const configSprite = "sprite";
    inline static const char * const configDepth = "depth";
    inline static const char * const configType = "type";
    inline static const char * const configOnCreate = "onCreate";
    inline static const char * const configOnStep = "onStep";

    inline static const char * const configRoom = "room";

    inline static const char * const configPrototype = "prototype";
    inline static const char * const configPosition = "position";
    inline static const char * const configRotation = "rotation";
    inline static const char * const configScale = "scale";

    bool setupConfig(const char *filename);

    Entity::Pool pool;
    Entity::Pool prototypePool;
    std::map<std::string, Entity *> prototypes;

    std::map<std::string, std::vector<Entity *>> types;
    std::set<Entity *> destroying;

    void foreach(std::function<void (Entity &)> action);
    void foreach(const char *type, std::function<bool (Entity &)> action);
    Entity &createPrototype(const char *name);

    // Lua

    static int L_createEmpty(lua_State *L);
    static int L_createCopy(lua_State *L);
    static int L_create(lua_State *L);

    static int L_destroy(lua_State *L);
    static int L_prototype(lua_State *L);

    static int L_foreachType(lua_State *L);
};

#endif //RIFLERANGE_ENTITY_H
