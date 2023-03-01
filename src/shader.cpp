//
// Created by KILLdon on 28.05.2019.
//

#include "shader.h"

#include "game.h"
#include "logger.h"

Shader::Shader(GLuint programId, std::vector<const char *> &&uniforms) : programId(programId) {
    for (auto uniform : uniforms) {
        auto location = glGetUniformLocation(programId, uniform);
        this->uniforms.insert(std::make_pair(uniform, location));
    }
}

Shader::~Shader() {
    glDeleteProgram(programId);
}

void Shader::use() {
    glUseProgram(programId);
}

Shaders::Shaders() : defaultShader(0) {}

Shader &Shaders::load(std::vector<const char *> uniforms, const char *vertexSource, const char *fragmentSource) {
    GLchar infoLog[512];
    GLint success;

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexSource, nullptr);

    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, nullptr, infoLog);
        Game::locator().logger().print("Vertex shader compilation failed\n%s", infoLog);
        glDeleteShader(vertexShaderId);
        return defaultShader;
    };

    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentSource, nullptr);

    glCompileShader(fragmentShaderId);
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShaderId, 512, nullptr, infoLog);
        Game::locator().logger().print("Fragment shader compilation failed\n%s", infoLog);
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        return defaultShader;
    };

    GLuint programId = glCreateProgram();

    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);

    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        Game::locator().logger().print("Program shader linking failed\n%s", infoLog);
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        glDeleteProgram(programId);
        return defaultShader;
    }

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return *pool.create(programId, std::move(uniforms));;
}

void Shader::uni1F(const char *name, float value) {
    glUniform1f(getLocation(name), value);
}

void Shader::uniMat3x3F(const char *name, const glm::mat3x3 &matrix) {
    glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

GLint Shader::getLocation(const char *name) {
    auto iter = uniforms.find(name);
    if (iter == uniforms.end()) {
        return -1;
    }
    return iter->second;
}
