//
// Created by KILLdon on 28.05.2019.
//

#ifndef RIFLERANGE_SHADER_H
#define RIFLERANGE_SHADER_H

#include "opengl_helper.h"

#include "object_pool.h"

#include "glm/matrix.hpp"

#include <vector>
#include <map>

class Shaders;

class Shader {
public:
    void use();

    void uni1F(const char *name, float value);

    void uniMat3x3F(const char *name, const glm::mat3x3 &matrix);
private:
    Shader(GLuint programId, std::vector<const char *> &&uniforms = {});
    ~Shader();

    typedef ObjectPool<Shader, 16> Pool;

    friend Pool;
    friend Shaders;

    GLuint programId;
    std::map<std::string, GLint> uniforms;

    GLint getLocation(const char *name);
};

class Shaders {
public:
    friend class Render;

    Shader &load(std::vector<const char *> uniforms, const char *vertexSource, const char *fragmentSource);
private:
    Shaders();

    Shader defaultShader;

    Shader::Pool pool;
};

#endif //RIFLERANGE_SHADER_H
