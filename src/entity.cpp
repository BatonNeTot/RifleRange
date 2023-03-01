//
// Created by KILLdon on 28.05.2019.
//

#include "game.h"
#include "logger.h"
#include "entity.h"

#include "lua_engine.h"
#include "common.h"

Entity::Entity() :
        name(),
        type(nullptr),
        sprite(&Game::locator().sprites().defaultSprite),
        spriteIndex(0),
        depth(0),
    
        scale(1, 1),
        position(0, 0),
        rotation(0),
    
        dirtyMatrix(true),
        transform(){}

Entity::Entity(const Entity &entity) :
        name(entity.name),
        type(nullptr),
        sprite(entity.sprite),
        spriteIndex(entity.spriteIndex),
        depth(entity.depth),

        scale(entity.scale),
        position(entity.position),
        rotation(entity.rotation),

        dirtyMatrix(true),
        transform(){
    setType(entity.type);
}

void Entity::setName(const char *name) {
    this->name = name;
}
const char *Entity::getName() const {
    return name.c_str();
}

void Entity::setType(const char *type) {
    if (this->type) {
        auto mapIter = Game::locator().entities().types.find(this->type);
        mapIter->second.erase(std::remove(mapIter->second.begin(), mapIter->second.end(), this), mapIter->second.end());
    }

    this->type = type;

    if (type) {
        auto mapIter = Game::locator().entities().types.find(type);
        if (mapIter == Game::Game::locator().entities().types.end()) {
            mapIter = Game::Game::locator().entities().types.insert(std::make_pair(type, std::vector<Entity *>())).first;
        }
        mapIter->second.push_back(this);

        this->type = mapIter->first.c_str();
    }
}
const char *Entity::getType() const {
    return type;
}

void Entity::setSprite(Sprite &sprite)  {
    this->sprite = &sprite;
    dirtyMatrix = true;
}

Sprite &Entity::getSprite() const {
    return *sprite;
}

void Entity::setIndex(unsigned int spriteIndex) {
    this->spriteIndex = spriteIndex;
}

unsigned int Entity::getIndex() const {
    return spriteIndex;
}

void Entity::setDepth(short depth) {
    this->depth = depth;
}

short Entity::getDepth() const {
    return depth;
}

void Entity::setScale(Point<float> scale) {
    this->scale = scale;
    dirtyMatrix = true;
}

Point<float> Entity::getScale() const {
    return scale;
}

void Entity::setPosition(Point<float> position) {
    this->position = position;
    dirtyMatrix = true;
}

Point<float> Entity::getPosition() const {
    return position;
}

void Entity::setRotation(float rotation) {
    this->rotation = Common::cropDeg(rotation);
    dirtyMatrix = true;
}

float Entity::getRotation() const {
    return rotation;
}

const glm::mat3 &Entity::getTransform() {
    if (dirtyMatrix) {
        const auto &room = Game::locator().window().getRoom();
        auto spriteSize = sprite->getSize();

        transform = glm::mat3(1.0);

        glm::mat3 scaleMat(1.0);
        scaleMat[0][0] = (float)spriteSize.width * scale.x / room.width;
        scaleMat[1][1] = (float)spriteSize.height * scale.y / room.height;

        transform = scaleMat * transform;

        glm::mat3 offsetMat(1.0);
        offsetMat[2][0] = (spriteSize.width - sprite->getOffset().x * 2.0f) * scale.x / room.width;
        offsetMat[2][1] = -(spriteSize.height - sprite->getOffset().y * 2.0f) * scale.y / room.height;

        transform =  offsetMat * transform;

        glm::mat3 rotationMat(1.0);
        auto angle = Common::toRad(rotation);
        auto m_sin = sin(angle);
        auto m_cos = cos(angle);
        rotationMat[0][0] = m_cos;
        rotationMat[0][1] = m_sin * (float)room.width / room.height;
        rotationMat[1][0] = -m_sin * (float)room.height / room.width;
        rotationMat[1][1] = m_cos;

        transform = rotationMat * transform;

        glm::mat3 translateMat(1.0);
        translateMat[2][0] = position.x * 2.0f / room.width - 1;
        translateMat[2][1] = -position.y * 2.0f / room.height + 1;

        transform = translateMat * transform;

        dirtyMatrix = false;
    }
    return transform;
}

void Entity::setupMeta(lua_State *L) {
    luaL_newmetatable(L, Entity::metatable);
    lua_createtable(L, 0, 17);

    luaL_Reg entityFunc[] = {
            {"setName", Entity::L_setName},
            {"getName", Entity::L_getName},
            {"setType", Entity::L_getType},
            {"getType", Entity::L_setType},
            {"setSprite", Entity::L_setSprite},
            {"getSprite", Entity::L_getSprite},
            {"setDepth", Entity::L_setDepth},
            {"getDepth", Entity::L_getDepth},
            {"setIndex", Entity::L_setIndex},
            {"getIndex", Entity::L_getIndex},
            {"setScale", Entity::L_setScale},
            {"getScale", Entity::L_getScale},
            {"setPosition", Entity::L_setPosition},
            {"getPosition", Entity::L_getPosition},
            {"setRotation", Entity::L_setRotation},
            {"getRotation", Entity::L_getRotation},
            {"getStorage", Entity::L_getStorage},
            {NULL, NULL},
    };
    luaL_register(L, NULL, entityFunc);

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, L_equal);
    lua_setfield(L, -2, "__eq");

    lua_pop(L, 1);
}


int Entity::L_setName(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setName(luaL_checkstring(L, 2));
    return 0;
}
int Entity::L_getName(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    lua_pushstring(L, entity.getName());
    return 1;
}

int Entity::L_setType(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setType(luaL_checkstring(L, 2));
    return 0;
}
int Entity::L_getType(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    lua_pushstring(L, entity.getType());
    return 1;
}

int Entity::L_setSprite(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setSprite(Sprite::getFromLua(L, 2));
    return 0;
}
int Entity::L_getSprite(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.sprite->pushToLua(L);
    return 1;
}

int Entity::L_setIndex(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setIndex(luaL_checkinteger(L, 2));
    return 0;
}
int Entity::L_getIndex(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    lua_pushinteger(L, entity.getIndex());
    return 1;
}

int Entity::L_setDepth(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setDepth(luaL_checkinteger(L, 2));
    return 0;
}

int Entity::L_getDepth(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    lua_pushinteger(L, entity.getDepth());
    return 1;
}

int Entity::L_setScale(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setScale({(float)luaL_checknumber(L, 2),
                     (float)luaL_checknumber(L, 3)});
    return 0;
}
int Entity::L_getScale(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    auto &&scale = entity.getScale();
    lua_pushnumber(L, scale.x);
    lua_pushnumber(L, scale.y);
    return 2;
}

int Entity::L_setPosition(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setPosition({(float)luaL_checknumber(L, 2),
                        (float)luaL_checknumber(L, 3)});
    return 0;
}
int Entity::L_getPosition(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    auto &&position = entity.getPosition();
    lua_pushnumber(L, position.x);
    lua_pushnumber(L, position.y);
    return 2;
}

int Entity::L_setRotation(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    entity.setRotation(luaL_checknumber(L, 2));
    return 0;
}
int Entity::L_getRotation(lua_State *L) {
    auto &entity = getFromLua(L, 1);
    lua_pushnumber(L, entity.getRotation());
    return 1;
}

int Entity::L_getStorage(lua_State *L) {
    auto &entity = getFromLua(L, 1);

    luaL_newmetatable(L, Entity::storage);
    lua_pushlightuserdata(L, &entity);
    lua_gettable(L, -2);

    return 1;
}

int Entity::L_equal(lua_State *L) {
    auto &first = getFromLua(L, 1);
    auto &second = getFromLua(L, 2);
    lua_pushboolean(L, &first == &second);
    return 1;
}

Entity &Entity::getFromLua(lua_State *L, int index) {
    return **(Entity **)luaL_checkudata(L, index, Entity::metatable);
}

void Entity::pushToLua(lua_State *L) {
    auto ptr = (Entity **)lua_newuserdata(L, sizeof(Entity *));
    *ptr = this;
    luaL_newmetatable(L, Entity::metatable);
    lua_setmetatable(L, -2);
}

bool Entities::setupConfig(const char *filename) {
    if (!Game::locator().lua().loadObjectFile(filename)) {
        Game::locator().logger().print("Error loading entities file!");
        Game::locator().logger().print(Game::locator().lua().getString());
        Game::locator().lua().pop();
        return false;
    }

    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Entities file isn't a table!");
        Game::locator().lua().pop();
        return false;
    }

    Game::locator().lua().getField(configPrototypes);
    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Can't found table with prototypes!");
        Game::locator().lua().pop(); // field configPrototypes
        Game::locator().lua().pop(); // config file
        return false;
    }

    Game::locator().lua().foreachInTable([this](unsigned index){
        const char* name = Game::locator().lua().getPathString({configName}, nullptr);
        if (!name) {
            Game::locator().logger().print("Entity prototype in %ud index doesn't have name", index);
            return;
        }

        Entity &prototype = createPrototype(name);

        prototype.name = name;
        prototype.type = Game::locator().lua().getPathString({configType}, nullptr);
        prototype.setSprite(Game::locator().sprites().get(Game::locator().lua().getPathString({configSprite}, "")));
        prototype.setDepth(Game::locator().lua().getPathLong({configDepth}, 0));

        Game::locator().lua().attachToStorage(prototype, Entity::onCreate, configOnCreate);
        Game::locator().lua().attachToStorage(prototype, Entity::onStep, configOnStep);
    });

    Game::locator().lua().pop(); // field configPrototypes

    Game::locator().lua().getField(configRoom);
    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Can't found table with room contents!");
        Game::locator().lua().pop(); // field configRoom
        Game::locator().lua().pop(); // config file
        return false;
    }

    Game::locator().lua().foreachInTable([this](unsigned index){
        const char* prototypeName = Game::locator().lua().getPathString({configPrototype}, nullptr);
        if (!prototypeName) {
            Game::locator().logger().print("Entity in %ud index in room doesn't have prototype", index);
            return;
        }

        auto iter = prototypes.find(prototypeName);
        if (iter == prototypes.end()) {
            Game::locator().logger().print("Entity in %ud index in room have undefined prototype", index);
            return;
        }
        auto &prototype = *(iter->second);

        Point<float> position = {
                (float)Game::locator().lua().getPathDouble({configPosition, Lua::configX}, 0),
                (float)Game::locator().lua().getPathDouble({configPosition, Lua::configY}, 0)
        };

        float rotation = (float)Game::locator().lua().getPathDouble({configRotation}, 0);

        Point<float> scale = {
                (float)Game::locator().lua().getPathDouble({configScale, Lua::configX}, 1),
                (float)Game::locator().lua().getPathDouble({configScale, Lua::configY}, 1)
        };

        auto &entity = *pool.create(prototype);
        entity.setPosition(position);
        entity.setRotation(rotation);
        entity.setScale(scale);

        Game::locator().lua().createStorage(entity);
        Game::locator().lua().reattachInStorage(prototype, entity, Entity::onCreate);
        Game::locator().lua().reattachInStorage(prototype, entity, Entity::onStep);

        Game::locator().lua().cleanEntityEnvironment(entity);
        Game::locator().lua().evalOnCreate(entity);
    });

    Game::locator().lua().pop(); // field configRoom

    Game::locator().lua().pop(); // config file
    return true;
}

Entity& Entities::createEmpty() {
    auto &entity = *pool.create();
    Game::locator().lua().createStorage(entity);
    return entity;
}

Entity &Entities::create(Entity &prototype) {
    auto &entity = *pool.create(prototype);

    Game::locator().lua().createStorage(entity);
    Game::locator().lua().reattachInStorage(prototype, entity, Entity::onCreate);
    Game::locator().lua().reattachInStorage(prototype, entity, Entity::onStep);

    Game::locator().lua().prepareEntityEnvironment(entity);
    Game::locator().lua().evalOnCreate(entity);
    Game::locator().lua().restoreEntityEnvironment();

    return entity;
}

Entity &Entities::create(const char *name) {
    auto iter = prototypes.find(name);
    if (iter == prototypes.end()) {
        return createEmpty();
    }
    return create(*iter->second);
}

void Entities::destroy(Entity &entity) {
    destroying.insert(&entity);
}

void Entities::clearDestroying() {
    for (auto ptr : destroying) {
        auto &entity = *ptr;
        pool.free(&entity);

        Game::locator().lua().deattachInStorage(entity, Entity::storage);
        Game::locator().lua().deattachInStorage(entity, Entity::onCreate);
        Game::locator().lua().deattachInStorage(entity, Entity::onStep);

        if (entity.type) {
            auto mapIter = types.find(entity.type);
            mapIter->second.erase(std::remove(mapIter->second.begin(), mapIter->second.end(), &entity), mapIter->second.end());
        }
    }
    destroying.clear();
}

void Entities::prototype(const char *name, Entity &entity) {
    auto &prototype = *prototypePool.create(entity);
    prototypes.insert(std::make_pair(name, &prototype));

    Game::locator().lua().reattachInStorage(entity, prototype, Entity::onCreate);
    Game::locator().lua().reattachInStorage(entity, prototype, Entity::onStep);
}

void Entities::foreach(std::function<void (Entity &)> action) {
    pool.foreachUsed(action);
}

void Entities::foreach(const char *type, std::function<bool (Entity &)> action) {
    auto mapIter = types.find(type);
    if (mapIter != types.end()) {
        auto &vector = mapIter->second;
        for (auto ptr : vector) {
            if (action(*ptr)) {
                break;
            }
        }
    }
}

Entity &Entities::createPrototype(const char *name) {
    auto *entity = prototypePool.create();
    prototypes.insert(std::make_pair(name, entity));
    return *entity;
}

void Entities::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 0); // meta

    lua_createtable(L, 0, 6);

    luaL_Reg entitiesFunc[] = {
            {"createEmpty", Entities::L_createEmpty},
            {"createCopy", Entities::L_createCopy},
            {"create", Entities::L_create},
            {"destroy", Entities::L_destroy},
            {"prototype", Entities::L_prototype},
            {"foreachType", Entities::L_foreachType},
            {NULL, NULL},
    };
    luaL_register(L, NULL, entitiesFunc);

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Entities");
}

int Entities::L_createEmpty(lua_State *L) {
    Game::locator().entities().createEmpty().pushToLua(L);
    return 1;
}
int Entities::L_createCopy(lua_State *L) {
    Game::locator().entities().create(Entity::getFromLua(L, 1)).pushToLua(L);
    return 1;
}
int Entities::L_create(lua_State *L) {
    Game::locator().entities().create(luaL_checkstring(L, 1)).pushToLua(L);
    return 1;
}

int Entities::L_destroy(lua_State *L) {
    Game::locator().entities().destroy(Entity::getFromLua(L, 1));
    return 0;
}
int Entities::L_prototype(lua_State *L) {
    Game::locator().entities().prototype(luaL_checkstring(L, 1), Entity::getFromLua(L, 2));
    return 0;
}


int Entities::L_foreachType(lua_State *L) {
    const char *type = luaL_checkstring(L, 1);
    if (!lua_isfunction(L, 2)) {
        return luaL_error(L, "Function was expected as #2 argument; got %s", lua_typename(L, lua_type(L, 2)));
    }


    Game::locator().entities().foreach(type, [L](Entity &entity) {
        Game::locator().lua().prepareEntityEnvironment(entity);

        lua_pushvalue(L, 2);
        if (lua_pcall(L, 0, 1, 0)) {
            Game::locator().logger().print(lua_tostring(L, -1));
        } else {
            if (lua_toboolean(L, -1)) {
                return true;
            }
        }

        Game::locator().lua().restoreEntityEnvironment();

        return false;
    });
    return 0;
}