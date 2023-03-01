//
// Created by KILLdon on 01.06.2019.
//

#ifndef RIFLERANGE_PARTICLES_H
#define RIFLERANGE_PARTICLES_H

#include "locator.h"

#include "object_pool.h"
#include "data.h"
#include "glm/matrix.hpp"
#include "opengl_helper.h"

#include "sprite.h"

class Particle {
public:
    friend class ParticleGenerator;

    glm::mat3 &getTransform(Sprite &sprite);
private:
    Particle();
    ~Particle();

    bool update(float delta, Point<float> gravity);

    Point<float> position;
    float rotation;
    float scale;

    float timeLeft;
    float depth;

    Point<float> speed;
    float scaleSpeed;
    float rotationSpeed;

    glm::mat3 transform;
};

struct ParticleGeneratorPrototype {
    ParticleGeneratorPrototype() = default;
    ParticleGeneratorPrototype(ParticleGeneratorPrototype &prototype) = default;

    typedef ObjectPool<ParticleGeneratorPrototype, 8> Pool;
    friend Pool;

    friend class ParticleMaster;

    float particleCooldown;
    unsigned maxAlive;

    Sprite *sprite;
    unsigned spriteIndex;

    short depthMin;
    short depthMax;

    float lifetimeMin;
    float lifetimeMax;

    float speedMin;
    float speedMax;

    float angle;
    float angleDispersion;

    Point<float> gravity;

    float scaleMin;
    float scaleMax;

    float scaleSpeedMin;
    float scaleSpeedMax;

    float rotation;
    float rotationDispersion;

    float rotationSpeedMin;
    float rotationSpeedMax;
};

class ParticleGenerator : private ParticleGeneratorPrototype {
public:

    typedef ObjectPool<ParticleGenerator, 32> Pool;
    friend Pool;
    friend class ParticleMaster;

    void update(float delta);
    unsigned particleCount();

    Sprite &getSprite();
    unsigned getIndex();

    GLuint getVao();

    void setRecreate(bool flag);

    void setPosition(Point<float> position);
    void setAngle(float angle);

    // Lua

    inline static const char * const metatable = "metatable.particlesGenerator";
    static void setupMeta(lua_State *L);

    static ParticleGenerator &getFromLua(lua_State *L, int index);
    void pushToLua(lua_State *L);

private:
    explicit ParticleGenerator(ParticleGeneratorPrototype &prototype);
    explicit ParticleGenerator(ParticleGeneratorPrototype &&prototype);
    ~ParticleGenerator();

    float lastParticleAgo;
    Point<float> position;

    unsigned particlesLeft;
    bool recreate;

    friend Particle;

    GLuint particlesVao;

    GLuint particlesVboBillboard;
    GLuint particlesVboTransform;
    GLuint particlesVboDepth;
    GLuint particlesVboTimeLeft;

    Particle *particles;
    std::set<Particle *> free;
    std::set<Particle *> used;

    // Lua

    static int L_setRecreate(lua_State *L);
    static int L_setPosition(lua_State *L);
    static int L_setAngle(lua_State *L);
};

class ParticleMaster {
public:

    friend class Locator;
    friend class Game;

    void foreach(std::function<void (ParticleGenerator &)> action);

    ParticleGenerator &create(const char *name);
    void destroy(ParticleGenerator &generator);

    void collectGarbage();

    // Lua

    static void setupLuaVar(lua_State *L);

private:
    ParticleMaster();

    inline static const char * const configName = "name";
    inline static const char * const configSprite = "sprite";
    inline static const char * const configSpriteIndex = "spriteIndex";
    inline static const char * const configDepthMin = "depthMin";
    inline static const char * const configDepthMax = "depthMax";
    inline static const char * const configLifetimeMin = "lifetimeMin";
    inline static const char * const configLifetimeMax = "lifetimeMax";
    inline static const char * const configParticlesPerSecond = "particlesPerSecond";
    inline static const char * const configMaxAlive = "maxAlive";
    inline static const char * const configSpeedMin = "speedMin";
    inline static const char * const configSpeedMax = "speedMax";
    inline static const char * const configGravity = "gravity";
    inline static const char * const configGravityAngle = "gravityAngle";
    inline static const char * const configAngle = "angle";
    inline static const char * const configAngleDispersion = "angleDispersion";
    inline static const char * const configScaleMin = "scaleMin";
    inline static const char * const configScaleMax = "scaleMax";
    inline static const char * const configScaleSpeedMin = "scaleSpeedMin";
    inline static const char * const configScaleSpeedMax = "scaleSpeedMax";
    inline static const char * const configRotation = "rotation";
    inline static const char * const configRotationDispersion = "rotationDispersion";
    inline static const char * const configRotationSpeedMin = "rotationSpeedMin";
    inline static const char * const configRotationSpeedMax = "rotationSpeedMax";

    bool setupConfig(const char *filename);

    ParticleGenerator defaultGenerator;

    ParticleGenerator::Pool pool;
    ParticleGeneratorPrototype::Pool prototypePool;
    std::map<std::string, ParticleGeneratorPrototype *> prototypes;

    // Lua

    static int L_create(lua_State *L);
    static int L_destroy(lua_State *L);

};

#endif //RIFLERANGE_PARTICLES_H
