//
// Created by KILLdon on 06.06.2019.
//

#include "locator.h"

#include "logger.h"
#include "entity.h"
#include "font.h"
#include "ini.h"
#include "input.h"
#include "lua_engine.h"
#include "particles.h"
#include "render.h"
#include "sound.h"
#include "sprite.h"
#include "window.h"

Locator::Locator() {
    mLogger = new Logger();

    mWindow = new Window();
    mLua = new Lua();
    mRender = new Render();
    mInput = new Input();
    mSprites = new Sprites();

    mEntities = new Entities();
    mFonts = new Fonts();
    mIni = new Ini();
    mParticleMaster = new ParticleMaster();
    mSoundMaster = new SoundMaster();
}

Locator::~Locator() {
    delete mSoundMaster;
    delete mParticleMaster;
    delete mIni;
    delete mFonts;
    delete mEntities;

    delete mSprites;
    delete mInput;
    delete mRender;
    delete mLua;
    delete mWindow;

    delete mLogger;
}