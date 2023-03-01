//
// Created by KILLdon on 04.06.2019.
//

#include "openal_helper.h"

#include "game.h"
#include "logger.h"

const char * GetOpenALErrorString(int errID) {
    if (errID == AL_NO_ERROR) return "";
    if (errID == AL_INVALID_NAME) return "Invalid name";
    if (errID == AL_INVALID_ENUM) return " Invalid enum ";
    if (errID == AL_INVALID_VALUE) return " Invalid value ";
    if (errID == AL_INVALID_OPERATION) return " Invalid operation ";
    if (errID == AL_OUT_OF_MEMORY) return " Out of memory like! ";

    return " Don't know ";
}

void CheckOpenALError(const char* stmt, const char* fname, int line)
{

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        Game::locator().logger().print("OpenAL error %08x, (%s) at %s:%i - for %s", err, GetOpenALErrorString(err), fname, line, stmt);
    }
};