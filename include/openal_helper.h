//
// Created by KILLdon on 04.06.2019.
//

#ifndef RIFLERANGE_OPENAL_HELPER_H
#define RIFLERANGE_OPENAL_HELPER_H

#include "AL/al.h"
#include "AL/alc.h"
#include "vorbis/vorbisfile.h"

#ifndef AL_CHECK
#ifdef _DEBUG
#define AL_CHECK(stmt) do { \
            stmt; \
            CheckOpenALError(#stmt, __FILE__, __LINE__); \
        } while (0);
#else
#define AL_CHECK(stmt) stmt
#endif
#endif

const char * GetOpenALErrorString(int errID);

void CheckOpenALError(const char* stmt, const char* fname, int line);

#endif //RIFLERANGE_OPENAL_HELPER_H
