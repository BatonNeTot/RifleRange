//
// Created by KILLdon on 28.05.2019.
//

#include "render.h"

#include "game.h"
#include "window.h"

#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

Render::~Render() {
    glDeleteVertexArrays(1, &billboardVao);
    glDeleteBuffers(1, &billboardVbo);
}

void Render::init() {
    const GLfloat bufferData[] = {
            -1.0f, 1.0f,
            1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, -1.0f,
    };

    glGenBuffers(1, &billboardVbo);
    glBindBuffer(GL_ARRAY_BUFFER, billboardVbo);

    glBufferData(
            GL_ARRAY_BUFFER, sizeof(bufferData),
            bufferData, GL_STATIC_DRAW
    );

    glGenVertexArrays(1, &billboardVao);

    glBindVertexArray(billboardVao);
    glBindBuffer(GL_ARRAY_BUFFER, billboardVbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO
    glBindVertexArray(0); // unbind VAO

    const char *spriteVertexSource = R"(
#version 410 core

in vec2 pos;

uniform mat3 transform;
uniform float depth;

out vec2 coord;

void main() {
    gl_Position = vec4((transform * vec3(pos, 1.0)).xy, depth, 1.0);
    coord = vec2((pos.x + 1) / 2, (1 - pos.y) / 2);
}
)";

    const char *spriteFragmentSource = R"(
#version 410 core

in vec2 coord;

uniform sampler2D sprite;
uniform float spriteHead;
uniform float spriteSize;

out vec4 colour;

void main() {
    vec4 textureColour = texture(sprite, vec2(coord.x * spriteSize + spriteHead, coord.y));

    if (textureColour.a < 0.5)
        discard;

    colour = textureColour;
}
)";

    spriteShader = &shaders.load({
                                         "depth", "transform", "spriteHead", "spriteSize"
                                 }, spriteVertexSource, spriteFragmentSource);

    const char *backgroundVertexSource = R"(
#version 410 core

in vec2 pos;

out vec2 coord;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    coord = vec2((pos.x + 1) / 2, (1 - pos.y) / 2);
}
)";

    const char *backgroundFragmentSource = R"(
#version 410 core

in vec2 coord;

uniform sampler2D background;
uniform float backgroundSize;

out vec4 colour;

void main() {
    vec4 textureColour = texture(background, vec2(coord.x * backgroundSize, coord.y));

    if (textureColour.a < 0.5)
        discard;

    colour = textureColour;
}
)";

    backgroundShader = &shaders.load({
                                             "backgroundSize"
                                     }, backgroundVertexSource, backgroundFragmentSource);

    const char *particlesVertexSource = R"(
#version 410 core

layout (location = 0) in vec2 pos;
layout (location = 1) in mat3 transform;
layout (location = 4) in float depth;
layout (location = 5) in float timeLeft;

out vec2 coord;

void main() {
    gl_Position = vec4((transform * vec3(pos, 1.0)).xy, depth, 1.0);
    coord = vec2((pos.x + 1) / 2, (1 - pos.y) / 2);
}
)";

    const char *particlesFragmentSource = R"(
#version 410 core

in vec2 coord;

uniform sampler2D sprite;
uniform float spriteHead;
uniform float spriteSize;

out vec4 colour;

void main() {
    vec4 textureColour = texture(sprite, vec2(coord.x * spriteSize + spriteHead, coord.y));

    if (textureColour.a < 0.5)
        discard;

    colour = textureColour;
}
)";

    particlesShader = &shaders.load({
                                            "spriteHead", "spriteSize"
                                    }, particlesVertexSource, particlesFragmentSource);

    const char *letterVertexSource = R"(
#version 410 core

in vec2 pos;

uniform mat3 transform;
uniform float depth;

out vec2 coord;

void main() {
    gl_Position = vec4((transform * vec3(pos, 1.0)).xy, depth, 1.0);
    coord = vec2((pos.x + 1) / 2, (1 - pos.y) / 2);
}
)";

    const char *letterFragmentSource = R"(
#version 410 core

in vec2 coord;

uniform sampler2D font;
uniform float letterX;
uniform float letterY;
uniform float letterWidth;
uniform float letterHeight;

out vec4 colour;

void main() {
    vec4 textureColour = texture(font, vec2(coord.x * letterWidth + letterX,
                                              coord.y * letterHeight + letterY));

    if (textureColour.a < 0.5)
        discard;

    colour = textureColour;
}
)";

    letterShader = &shaders.load({
                                         "transform", "depth", "letterX", "letterY", "letterWidth", "letterHeight"
                                 }, letterVertexSource, letterFragmentSource);


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);
}

void Render::renderWorld() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);

    auto &background = *Game::locator().window().background;
    if (background.textureId != 0) {
        backgroundShader->use();
        backgroundShader->uni1F("backgroundSize", 1.0f / background.getSlides());

        glBindTexture(GL_TEXTURE_2D, background.textureId);

        glBindVertexArray(billboardVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glDepthMask(GL_TRUE);

    Game::locator().entities().foreach([](Entity &entity){
        Game::locator().render().renderEntity(entity);
    });
}

void Render::renderParticles(ParticleGenerator &generator) {
    particlesShader->use();

    auto &sprite = generator.getSprite();

    particlesShader->uni1F("spriteHead", (float) generator.getIndex() / sprite.getSlides());
    particlesShader->uni1F("spriteSize", 1.0f / sprite.getSlides());

    glBindTexture(GL_TEXTURE_2D, sprite.textureId);

    glBindVertexArray(generator.getVao());
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, generator.particleCount());
}

void Render::renderText(Font &font, std::vector<unsigned> &textCodes, Point<float> pos, Font::ALIGNMENT alignment, short depth) {
    if (font.getTexture() != 0) {
        letterShader->use();
        glBindTexture(GL_TEXTURE_2D, font.getTexture());
        glBindVertexArray(billboardVao);

        auto &letterSize = font.getLetterSize();

        Point<float> letterPos;
        letterPos.x = pos.x - ((((unsigned)alignment) & (4u - 1u)) % 3) * (textCodes.size() * letterSize.width / 2.0f);
        letterPos.y = pos.y - (((((unsigned)alignment) >> 2u) & (4u - 1u)) % 3) * (letterSize.height / 2.0f);

        for (unsigned &textCode : textCodes) {
            const auto &room = Game::locator().window().getRoom();

            glm::mat3 transform(1.0);

            glm::mat3 scaleMat(1.0);
            scaleMat[0][0] = (float)letterSize.width / room.width;
            scaleMat[1][1] = (float)letterSize.height / room.height;

            transform = scaleMat * transform;

            glm::mat3 offsetMat(1.0);
            offsetMat[2][0] = (float)letterSize.width/ room.width;
            offsetMat[2][1] = -(float)letterSize.height / room.height;

            transform =  offsetMat * transform;

            glm::mat3 translateMat(1.0);
            translateMat[2][0] = letterPos.x * 2.0f / room.width - 1;
            translateMat[2][1] = -letterPos.y * 2.0f / room.height + 1;

            transform = translateMat * transform;

            letterShader->uniMat3x3F("transform", transform);
            letterShader->uni1F("depth", (double)depth / SHRT_MAX);

            auto &slices = font.getSlices();

            letterShader->uni1F("letterX", (float)(textCode % slices.x) / slices.x);
            letterShader->uni1F("letterY", (float)(textCode / slices.x) / slices.y);

            letterShader->uni1F("letterWidth", 1.0f / slices.x);
            letterShader->uni1F("letterHeight", 1.0f / slices.y);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            letterPos.x += letterSize.width;
        }
    }
}

void Render::renderEntity(Entity &entity) {
    if (entity.getSprite().textureId != 0) {
        spriteShader->use();

        auto &sprite = entity.getSprite();

        spriteShader->uniMat3x3F("transform", entity.getTransform());
        spriteShader->uni1F("depth", (double)entity.getDepth() / SHRT_MAX);
        spriteShader->uni1F("spriteHead", (float) entity.getIndex() / sprite.getSlides());
        spriteShader->uni1F("spriteSize", 1.0f / sprite.getSlides());

        glBindTexture(GL_TEXTURE_2D, sprite.textureId);

        glBindVertexArray(billboardVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}