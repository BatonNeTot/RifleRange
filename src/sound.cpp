//
// Created by KILLdon on 04.06.2019.
//

#include "sound.h"

#include "game.h"
#include "logger.h"
#include "lua_engine.h"

Sound::Sound(ALuint bufferID) : bufferID(bufferID) {}

Sound::~Sound() {
    alDeleteBuffers(1, &bufferID);
}

void Sound::setupMeta(lua_State *L) {
//    luaL_newmetatable(L, Sound::metatable);
//
//    lua_pop(L, 1);
}

Sound &Sound::getFromLua(lua_State *L, int index) {
    return **(Sound **)luaL_checkudata(L, index, Sound::metatable);
}
void Sound::pushToLua(lua_State *L) {
    auto ptr = (Sound **)lua_newuserdata(L, sizeof(Sound *));
    *ptr = this;
    luaL_newmetatable(L, Sound::metatable);
    lua_setmetatable(L, -2);
}

Player::Player() :
    sourceID(0),
    sound(&Game::locator().soundMaster().defaultSound) {
    alGenSources(1, &sourceID);
    alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
}

Player::~Player() {
    alDeleteSources(1, &sourceID);
}

void Player::play() {
    alSourcePlay(sourceID);
}

void Player::pause() {
    alSourcePause(sourceID);
}

void Player::stop() {
    alSourceStop(sourceID);
}

bool Player::isPlaying() {
    ALint state;
    alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    return state != AL_STOPPED && state != AL_PAUSED;
}

bool Player::isStopped() {
    ALint state;
    alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    return state == AL_STOPPED;
}

void Player::setLooping(bool flag) {
    alSourcei(sourceID, AL_LOOPING, flag);
}

void Player::setStatic(bool flag) {
    staticState = flag;
}

void Player::setSound(Sound &sound) {
    stop();
    this->sound = &sound;
    alSourcei(sourceID, AL_BUFFER, sound.bufferID);
}

void Player::setupMeta(lua_State *L) {
    luaL_newmetatable(L, Player::metatable);
    lua_createtable(L, 0, 8);

    luaL_Reg playerFunc[] = {
            {"play", Player::L_play},
            {"pause", Player::L_pause},
            {"stop", Player::L_stop},
            {"isPlaying", Player::L_isPlaying},
            {"isStopped", Player::L_isStopped},
            {"setLooping", Player::L_setLooping},
            {"setStatic", Player::L_setStatic},
            {"setSound", Player::L_setSound},
            {NULL, NULL},
    };
    luaL_register(L, NULL, &playerFunc[0]);

    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

int Player::L_play(lua_State *L) {
    auto &player = getFromLua(L, 1);
    player.play();
    return 0;
}
int Player::L_pause(lua_State *L) {
    auto &player = getFromLua(L, 1);
    player.pause();
    return 0;
}
int Player::L_stop(lua_State *L) {
    auto &player = getFromLua(L, 1);
    player.stop();
    return 0;
}

int Player::L_isPlaying(lua_State *L) {
    auto &player = getFromLua(L, 1);
    lua_pushboolean(L, player.isPlaying());
    return 1;
}
int Player::L_isStopped(lua_State *L) {
    auto &player = getFromLua(L, 1);
    lua_pushboolean(L, player.isStopped());
    return 1;
}

int Player::L_setLooping(lua_State *L) {
    auto &player = getFromLua(L, 1);
    player.setLooping(lua_toboolean(L, 2));
    return 0;
}
int Player::L_setStatic(lua_State *L) {
    auto &player = getFromLua(L, 1);
    player.setStatic(lua_toboolean(L, 2));
    return 0;
}
int Player::L_setSound(lua_State *L) {
    auto &player = getFromLua(L, 1);
    auto &sound = Sound::getFromLua(L, 2);
    player.setSound(sound);
    return 0;
}

Player &Player::getFromLua(lua_State *L, int index) {
    return **(Player **)luaL_checkudata(L, index, Player::metatable);
}
void Player::pushToLua(lua_State *L) {
    auto ptr = (Player **)lua_newuserdata(L, sizeof(Player *));
    *ptr = this;
    luaL_newmetatable(L, Player::metatable);
    lua_setmetatable(L, -2);
}

SoundMaster::SoundMaster() :
    defaultSound(0),
    deviceAL(nullptr) {
    alGetError();

    deviceAL = alcOpenDevice(nullptr);

    if (deviceAL == nullptr)
    {
        Game::locator().logger().print("Failed to init OpenAL device.");
        exit(-1);
    }

    ALCcontext *contextAL = alcCreateContext(deviceAL, nullptr);
    AL_CHECK( alcMakeContextCurrent(contextAL) );

    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
}

SoundMaster::~SoundMaster() {
    alcCloseDevice(deviceAL);
}

void SoundMaster::collectGarbage() {
    std::list<Player *> destroying;
    playerPool.foreachUsed([&destroying](Player &player){
        if (!player.staticState && player.isStopped()) {
            destroying.push_back(&player);
        }
    });
    for (Player *ptr : destroying) {
        playerPool.free(ptr);
    }
}

bool SoundMaster::setupConfig(const char *filename) {

    if (!Game::locator().lua().loadObjectFile(filename)) {
        Game::locator().logger().print("Error loading sounds file!");
        Game::locator().logger().print(Game::locator().lua().getString());
        Game::locator().lua().pop();
        return false;
    }

    if (!Game::locator().lua().isTable()) {
        Game::locator().logger().print("Sounds file isn't a table!");
        Game::locator().lua().pop();
        return false;
    }

    Game::locator().lua().foreachInTable([](unsigned index){
        const char* name = Game::locator().lua().getPathString({configName}, nullptr);
        if (!name) {
            Game::locator().logger().print("Sound in %ud index doesn't have name", index);
            return;
        }

        const char* source = Game::locator().lua().getPathString({configSource}, nullptr);
        if (!source) {
            Game::locator().logger().print("Sound in %ud index doesn't have source", index);
            return;
        }

        Game::locator().soundMaster().load(name, source);
    });

    Game::locator().lua().pop();

    return true;
}

#define BUFFER_SIZE   32768     // 32 KB buffers

void SoundMaster::load(const char *name, const char *source) {
    int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
    int bitStream;
    long bytes;
    char *array = new char[BUFFER_SIZE];    // Local fixed size array

    ALenum format;              // The sound data format
    ALsizei freq;               // The frequency of the sound data

    std::vector<char> buffer; // The sound buffer data from file

    vorbis_info *pInfo;
    OggVorbis_File oggFile;

    ov_fopen(source, &oggFile);
    pInfo = ov_info(&oggFile, -1); // Get some information about the OGG file

    // Check the number of channels... always use 16-bit samples
    if (pInfo->channels == 1)
        format = AL_FORMAT_MONO16;
    else
        format = AL_FORMAT_STEREO16;
    // end if

    // The frequency of the sampling rate
    freq = pInfo->rate;

    do {
        // Read up to a buffer's worth of decoded sound data
        bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);
        // Append to end of buffer
        buffer.insert(buffer.end(), array, array + bytes);
    } while (bytes > 0);

    ov_clear(&oggFile);
    delete[] array;

    ALuint bufferID;            // The OpenAL sound buffer ID

    // Create sound buffer and source
    alGenBuffers(1, &bufferID);

    // Upload sound data to buffer
    alBufferData(bufferID, format, &buffer[0], static_cast <ALsizei>(buffer.size()), freq);

    auto *ptr = soundPool.create(bufferID);
    sounds.insert(std::make_pair(name, ptr));
}

Sound &SoundMaster::getSound(const char *name) {
    auto iter = sounds.find(name);
    if (iter == sounds.end()) {
        return defaultSound;
    }
    return *iter->second;
}

Player &SoundMaster::createPlayer() {
    return *playerPool.create();
}

void SoundMaster::setupLuaVar(lua_State *L) {
    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 2); // meta

    lua_createtable(L, 0, 2);

    luaL_Reg soundsFunc[] = {
            {"getSound", SoundMaster::L_getSound},
            {"createPlayer", SoundMaster::L_createPlayer},
            {NULL, NULL},
    };
    luaL_register(L, NULL, soundsFunc);

    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Lua::emptyFunction);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "Sounds");
}

int SoundMaster::L_getSound(lua_State *L) {
    Game::locator().soundMaster().getSound(luaL_checkstring(L, 1)).pushToLua(L);
    return 1;
}
int SoundMaster::L_createPlayer(lua_State *L) {
    Game::locator().soundMaster().createPlayer().pushToLua(L);
    return 1;
}
