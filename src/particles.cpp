//
// Created by KILLdon on 02.06.2019.
//

#include "particles.h"

#include "logger.h"
#include "game.h"
#include "common.h"
#include "lua_engine.h"

#include <list>
#include <cmath>

Particle::Particle() :
        transform(),

        position(),
        rotation(0),
        scale(1),

        timeLeft(0),
        depth(0),
        speed(),
        scaleSpeed(0),
        rotationSpeed(0) {

}

Particle::~Particle() {
    timeLeft = -1;
}

glm::mat3 &Particle::getTransform(Sprite &sprite) {
    const auto &room = Game::locator().window().getRoom();
    auto spriteSize = sprite.getSize();

    transform = glm::mat3(1.0);

    glm::mat3 scaleMat(1.0);
    scaleMat[0][0] = (float)spriteSize.width * scale / room.width;
    scaleMat[1][1] = (float)spriteSize.height * scale / room.height;

    transform = scaleMat * transform;

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

    return transform;
}

bool Particle::update(float delta, Point<float> gravity) {
    timeLeft -= delta;

    if (timeLeft > 0) {
        position.x += speed.x * delta;
        position.y += speed.y * delta;

        speed.x += gravity.x * delta;
        speed.y += gravity.y * delta;

        scale += scaleSpeed * delta;
        rotation = Common::cropDeg(rotation + rotationSpeed * delta);
        return true;
    } else {
        return false;
    }
}

ParticleGenerator::ParticleGenerator(ParticleGeneratorPrototype &prototype) :
    ParticleGeneratorPrototype(prototype),

    lastParticleAgo(0),
    position(0,0),

    particlesLeft(maxAlive),
    recreate(false),

    particlesVao(0),
    particlesVboBillboard(0),
    particlesVboTransform(0),
    particlesVboDepth(0),
    particlesVboTimeLeft(0),

    particles(nullptr),
    free(),
    used() {
    particles = (Particle *)::operator new(sizeof(Particle) * maxAlive);

    auto iter = free.insert(particles).first;
    for (int i = 0; i < maxAlive; i++) {
        iter = free.insert(iter, particles + i);
    }

    const GLfloat bufferData[] = {
            -1.0f, 1.0f,
            1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, -1.0f,
    };

    glGenVertexArrays(1, &particlesVao);
    glBindVertexArray(particlesVao);

    glGenBuffers(1, &particlesVboBillboard);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVboBillboard);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    glGenBuffers(1, &particlesVboTransform);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVboTransform);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat3) * maxAlive, nullptr, GL_STREAM_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), nullptr);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (const GLvoid*)(sizeof(GLfloat) * 3));
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (const GLvoid*)(sizeof(GLfloat) * 6));
    glVertexAttribDivisor(3, 1);

    glGenBuffers(1, &particlesVboDepth);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVboDepth);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * maxAlive, nullptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), nullptr);
    glVertexAttribDivisor(4, 1);

    glGenBuffers(1, &particlesVboTimeLeft);
    glBindBuffer(GL_ARRAY_BUFFER, particlesVboTimeLeft);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * maxAlive, nullptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), nullptr);
    glVertexAttribDivisor(5, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

ParticleGenerator::ParticleGenerator(ParticleGeneratorPrototype &&prototype) :
        ParticleGeneratorPrototype(prototype),

        lastParticleAgo(0),
        position(0,0),

        particlesLeft(0),
        recreate(false),

        particlesVao(0),
        particlesVboBillboard(0),
        particlesVboTransform(0),
        particlesVboDepth(0),
        particlesVboTimeLeft(0),

        particles(nullptr),
        free(),
        used() {}

ParticleGenerator::~ParticleGenerator() {
    ::operator delete(particles);
    glDeleteVertexArrays(1, &particlesVao);
    glDeleteBuffers(1, &particlesVboBillboard);
    glDeleteBuffers(1, &particlesVboTransform);
    glDeleteBuffers(1, &particlesVboDepth);
    glDeleteBuffers(1, &particlesVboTimeLeft);
}

void ParticleGenerator::update(float delta) {
    lastParticleAgo += delta;

    while (lastParticleAgo >= particleCooldown) {

        if (recreate || particlesLeft > 0) {
            auto iter = free.begin();
            if (iter != free.end()) {
                particlesLeft--;

                auto *ptr = *iter;
                free.erase(iter);
                auto &particle = *(new(ptr) Particle());
                used.insert(ptr);

                particle.position = this->position;
                particle.rotation = Common::generateDeg(rotation, rotationDispersion);
                particle.scale = Common::generateFloat(scaleMin, scaleMax);

                particle.timeLeft = Common::generateFloat(lifetimeMin, lifetimeMax);
                particle.depth = (float) Common::generateInt(depthMin, depthMax) / SHRT_MAX;

                float speed = Common::generateFloat(speedMin, speedMax);
                float launchAngle = Common::toRad(Common::generateDeg(angle, angleDispersion));
                particle.speed.x = speed * cosf(launchAngle);
                particle.speed.y = speed * -sinf(launchAngle);
                particle.scaleSpeed = Common::generateFloat(scaleSpeedMin, scaleSpeedMax);
                particle.rotationSpeed = Common::generateFloat(rotationSpeedMin, rotationSpeedMax);
            }
        }

        lastParticleAgo -= particleCooldown;
    }

    unsigned counter = 0;
    std::list<Particle *> notAliveAnymore;

    for (auto ptr : used) {
        if (!ptr->update(delta, gravity)) {
            notAliveAnymore.push_back(ptr);
        } else {
            glBindBuffer(GL_ARRAY_BUFFER, particlesVboTransform);
            glBufferSubData(GL_ARRAY_BUFFER, counter * sizeof(glm::mat3), sizeof(glm::mat3), &ptr->getTransform(*sprite));
            glBindBuffer(GL_ARRAY_BUFFER, particlesVboDepth);
            glBufferSubData(GL_ARRAY_BUFFER, counter * sizeof(float), sizeof(float), &ptr->depth);
            glBindBuffer(GL_ARRAY_BUFFER, particlesVboTimeLeft);
            glBufferSubData(GL_ARRAY_BUFFER, counter * sizeof(float), sizeof(float), &ptr->timeLeft);

            counter++;
        }
    }
    for (auto ptr : notAliveAnymore) {
        used.erase(ptr);
        free.insert(ptr);
    }
}

unsigned ParticleGenerator::particleCount() {
    return used.size();
}

Sprite& ParticleGenerator::getSprite() {
    return *sprite;
}

unsigned ParticleGenerator::getIndex() {
    return spriteIndex;
}

GLuint ParticleGenerator::getVao() {
    return particlesVao;
}

void ParticleGenerator::setRecreate(bool flag) {
    if (!flag) {
        particlesLeft = maxAlive;
    }
    this->recreate = flag;
}

void ParticleGenerator::setPosition(Point<float> position) {
    this->position = position;
}

void ParticleGenerator::setAngle(float angle) {
    this->angle = Common::cropDeg(angle);
}

void ParticleGenerator::setupMeta(lua_State *L) {
    luaL_newmetatable(L, ParticleGenerator::metatable);
    lua_createtable(L, 0, 3);

    luaL_Reg particleGeneratorFunc[] = {
            {"setRecreate", ParticleGenerator::L_setRecreate},
            {"setPosition", ParticleGenerator::L_setPosition},
            {"setAngle", ParticleGenerator::L_setAngle},
            {NULL, NULL},
    };
    luaL_register(L, NULL, &particleGeneratorFunc[0]);

    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

int ParticleGenerator::L_setRecreate(lua_State *L) {
    auto &generator = getFromLua(L, 1);
    generator.setRecreate(lua_toboolean(L, 2));
    return 0;
}
int ParticleGenerator::L_setPosition(lua_State *L) {
    auto &generator = getFromLua(L, 1);
    generator.setPosition({(float)luaL_checknumber(L, 2),
                           (float)luaL_checknumber(L, 3)});
    return 0;
}
int ParticleGenerator::L_setAngle(lua_State *L) {
    auto &generator = getFromLua(L, 1);
    generator.setAngle((float)luaL_checknumber(L, 2));
    return 0;
}

ParticleGenerator& ParticleGenerator::getFromLua(lua_State *L, int index) {
    return **(ParticleGenerator **)luaL_checkudata(L, index, ParticleGenerator::metatable);
}

void ParticleGenerator::pushToLua(lua_State *L) {
    auto ptr = (ParticleGenerator **)lua_newuserdata(L, sizeof(ParticleGenerator *));
    *ptr = this;
    luaL_newmetatable(L, ParticleGenerator::metatable);
    lua_setmetatable(L, -2);
}

ParticleMaster::ParticleMaster() : defaultGenerator({}) {}

bool ParticleMaster::setupConfig(const char *filename) {
    if (!Game::locator().lua().loadObjectFile(filename)) {
        Game::locator().logger().print("Error loading particles file!");
        Game::locator().logger().print(Game::locator().lua().getString());
        Game::locator().lua().pop();
        return false;
    }

    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Particles file isn't a table!");
        Game::locator().lua().pop();
        return false;
    }

    Game::locator().lua().foreachInTable([this](unsigned index){
        auto *ptr = prototypePool.create();

        const char* name = Game::locator().lua().getPathString({configName}, nullptr);
        if (!name) {
            Game::locator().logger().print("Particles in %ud index doesn't have name", index);
            prototypePool.free(ptr);
            return;
        }

        const char* spriteName = Game::locator().lua().getPathString({configSprite}, nullptr);
        if (!spriteName) {
            Game::locator().logger().print("Particles in %ud index doesn't have sprite", index);
            prototypePool.free(ptr);
            return;
        }
        Sprite &sprite = Game::locator().sprites().get(spriteName);
        if (sprite.getSize().width == 0 || sprite.getSize().height == 0) {
            Game::locator().logger().print("Particles in %ud index has wrong spriteName", index);
            prototypePool.free(ptr);
            return;
        }
        ptr->sprite = &sprite;

        long inputLong = Game::locator().lua().getPathLong({configSpriteIndex}, 0);
        if (inputLong < 0) {
            Game::locator().logger().print("Particles in %ud index has wrong spriteIndex; must be zero or more", index);
            ptr->spriteIndex = 0;
        } else {
            ptr->spriteIndex = inputLong;
        }

        ptr->depthMin = Game::locator().lua().getPathLong({configDepthMin}, 0);
        ptr->depthMax = Game::locator().lua().getPathLong({configDepthMax}, 0);

        double inputDouble = Game::locator().lua().getPathDouble({configLifetimeMin}, 0);
        if (inputDouble <= 0) {
            Game::locator().logger().print("Particles in %ud index has wrong lifetimeMin; must be more than zero", index);
            prototypePool.free(ptr);
            return;
        }
        ptr->lifetimeMin = inputDouble;

        inputDouble = Game::locator().lua().getPathDouble({configLifetimeMax}, 0);
        if (inputDouble <= 0) {
            Game::locator().logger().print("Particles in %ud index has wrong lifetimeMax; must be more than zero", index);
            prototypePool.free(ptr);
            return;
        }
        ptr->lifetimeMax = inputDouble;

        inputDouble = Game::locator().lua().getPathDouble({configParticlesPerSecond}, 0);
        if (inputDouble <= 0) {
            Game::locator().logger().print("Particles in %ud index has wrong particlesPerSecond; must be more than zero", index);
            prototypePool.free(ptr);
            return;
        }
        ptr->particleCooldown = 1 / inputDouble;

        inputLong = Game::locator().lua().getPathLong({configMaxAlive}, 0);
        if (inputLong <= 0) {
            Game::locator().logger().print("Particles in %ud index has wrong maxAlive; must be more than zero");
            prototypePool.free(ptr);
            return;
        }
        ptr->maxAlive = inputLong;

        ptr->speedMin = Game::locator().lua().getPathDouble({configSpeedMin}, 0);
        ptr->speedMax = Game::locator().lua().getPathDouble({configSpeedMax}, 0);

        ptr->angle = Game::locator().lua().getPathDouble({configAngle}, 0);
        inputDouble = Game::locator().lua().getPathDouble({configAngleDispersion}, 0);
        if (inputDouble < 0) {
            Game::locator().logger().print("Particles in %ud index has wrong angleDispersion; must be zero or more", index);
            ptr->angleDispersion = 0;
        } else {
            ptr->angleDispersion = inputDouble;
        }

        double gravity = Game::locator().lua().getPathDouble({configGravity}, 0);
        double gravityAngle = Game::locator().lua().getPathDouble({configGravityAngle}, 270);

        ptr->gravity.x = gravity * cos(Common::toRad(gravityAngle));
        ptr->gravity.y = gravity * -sin(Common::toRad(gravityAngle));

        ptr->scaleMin = Game::locator().lua().getPathDouble({configScaleMin}, 1);
        ptr->scaleMax = Game::locator().lua().getPathDouble({configScaleMax}, 1);

        ptr->scaleSpeedMin = Game::locator().lua().getPathDouble({configScaleSpeedMin}, 0);
        ptr->scaleSpeedMax = Game::locator().lua().getPathDouble({configScaleSpeedMax}, 0);

        ptr->rotation = Game::locator().lua().getPathDouble({configRotation}, 0);
        inputDouble = Game::locator().lua().getPathDouble({configRotationDispersion}, 0);
        if (inputDouble < 0) {
            Game::locator().logger().print("Particles in %ud index has wrong rotationDispersion; must be zero or more", index);
            ptr->rotationDispersion = 0;
        } else {
            ptr->rotationDispersion = inputDouble;
        }

        ptr->rotationSpeedMin = Game::locator().lua().getPathDouble({configRotationSpeedMin}, 0);
        ptr->rotationSpeedMax = Game::locator().lua().getPathDouble({configRotationSpeedMax}, 0);

        prototypes.insert(std::make_pair(name, ptr));
    });

    Game::locator().lua().pop();
    return true;
}

void ParticleMaster::foreach(std::function<void (ParticleGenerator &)> action) {
    pool.foreachUsed(action);
}

ParticleGenerator &ParticleMaster::create(const char *name) {
    auto iter = prototypes.find(name);
    if (iter == prototypes.end()) {
        return defaultGenerator;
    }
    return *pool.create(*iter->second);
}

void ParticleMaster::destroy(ParticleGenerator &generator) {
    pool.free(&generator);
}

void ParticleMaster::collectGarbage() {
    std::list<ParticleGenerator *> destroying;
    pool.foreachUsed([&destroying](ParticleGenerator &generator){
        if (!generator.recreate && generator.particlesLeft <= 0 && generator.used.size() == 0) {
            destroying.push_back(&generator);
        }
    });
    for (ParticleGenerator *ptr : destroying) {
        pool.free(ptr);
    }
}

void ParticleMaster::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 2); // meta

    lua_createtable(L, 0, 2);

    luaL_Reg particlesFunc[] = {
            {"create", ParticleMaster::L_create},
            {"destroy", ParticleMaster::L_destroy},
            {NULL, NULL},
    };
    luaL_register(L, NULL, particlesFunc);

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Particles");
}

int ParticleMaster::L_create(lua_State *L) {
    Game::locator().particleMaster().create(luaL_checkstring(L, 1)).pushToLua(L);
    return 1;
}
int ParticleMaster::L_destroy(lua_State *L) {
    Game::locator().particleMaster().destroy(ParticleGenerator::getFromLua(L, 1));
    return 0;
}