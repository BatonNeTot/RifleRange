//
// Created by KILLdon on 04.06.2019.
//

#ifndef RIFLERANGE_SOUND_H
#define RIFLERANGE_SOUND_H

#include "locator.h"

#include "openal_helper.h"
#include "object_pool.h"
#include "lua_helper.h"

#include <map>

class Sound {
public:

    typedef ObjectPool<Sound, 8> Pool;
    friend Pool;
    friend class Player;
    friend class SoundMaster;

    // Lua

    inline static const char * const metatable = "metatable.sound";
    static void setupMeta(lua_State *L);

    static Sound &getFromLua(lua_State *L, int index);
    void pushToLua(lua_State *L);

private:
    Sound(ALuint bufferID);
    ~Sound();

    const ALuint bufferID;
};

class Player {
public:

    typedef ObjectPool<Player, 8> Pool;
    friend Pool;
    friend Sound;
    friend class SoundMaster;

    void play();
    void pause();
    void stop();

    bool isPlaying();
    bool isStopped();

    void setLooping(bool flag);
    void setStatic(bool flag);
    void setSound(Sound &sound);

    // Lua

    inline static const char * const metatable = "metatable.player";
    static void setupMeta(lua_State *L);

    static Player &getFromLua(lua_State *L, int index);
    void pushToLua(lua_State *L);

private:
    Player();
    ~Player();

    ALuint sourceID;

    Sound *sound;
    bool staticState;

    // Lua

    static int L_play(lua_State *L);
    static int L_pause(lua_State *L);
    static int L_stop(lua_State *L);

    static int L_isPlaying(lua_State *L);
    static int L_isStopped(lua_State *L);

    static int L_setLooping(lua_State *L);
    static int L_setStatic(lua_State *L);
    static int L_setSound(lua_State *L);
};

class SoundMaster {
public:

    void collectGarbage();

    void load(const char *name, const char *source);
    Sound &getSound(const char *name);

    Player &createPlayer();

    // Lua

    static void setupLuaVar(lua_State *L);
private:
    SoundMaster();
    ~SoundMaster();

    inline static const char * const configName = "name";
    inline static const char * const configSource = "source";

    bool setupConfig(const char *filename);

    friend Sound;
    friend Player;
    friend class Locator;
    friend class Game;

    Sound defaultSound;

    Sound::Pool soundPool;
    Player::Pool playerPool;

    std::map<std::string, Sound *> sounds;

    ALCdevice *deviceAL;

    // Lua

    static int L_getSound(lua_State *L);
    static int L_createPlayer(lua_State *L);
};

#endif //RIFLERANGE_SOUND_H
