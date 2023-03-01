//
// Created by KILLdon on 06.06.2019.
//

#ifndef RIFLERANGE_LOCATOR_H
#define RIFLERANGE_LOCATOR_H

class Logger;
class Entities;
class Fonts;
class Ini;
class Input;
class Lua;
class ParticleMaster;
class Render;
class SoundMaster;
class Sprites;
class Window;

class Locator {
public:

    inline Logger &logger() { return *mLogger; }
    inline Entities &entities() { return *mEntities; }
    inline Fonts &fonts() { return *mFonts; }
    inline Ini &ini() { return *mIni; }
    inline Input &input() { return *mInput; }
    inline Lua &lua() { return *mLua; }
    inline ParticleMaster &particleMaster() { return *mParticleMaster; }
    inline Render &render() { return *mRender; }
    inline SoundMaster &soundMaster() { return *mSoundMaster; }
    inline Sprites &sprites() { return *mSprites; }
    inline Window &window() { return *mWindow; }

private:

    friend class Game;

    Locator();
    ~Locator();

    Logger *mLogger;
    Entities *mEntities;
    Fonts *mFonts;
    Ini *mIni;
    Input *mInput;
    Lua *mLua;
    ParticleMaster *mParticleMaster;
    Render *mRender;
    SoundMaster *mSoundMaster;
    Sprites *mSprites;
    Window *mWindow;

};

#endif //RIFLERANGE_LOCATOR_H
