//
// Created by KILLdon on 28.05.2019.
//

#ifndef RIFLERANGE_RENDER_H
#define RIFLERANGE_RENDER_H

#include "locator.h"

#include "opengl_helper.h"

#include "shader.h"
#include "particles.h"
#include "entity.h"
#include "font.h"

#include <vector>

class Render {
public:

    friend class Locator;
    friend class Game;

    void renderParticles(ParticleGenerator &generator);
    void renderText(Font &font, std::vector<unsigned> &textCodes, Point<float> pos, Font::ALIGNMENT aligment, short depth);
private:
    Render() = default;
    ~Render();

    void init();
    void renderWorld();
    void renderEntity(Entity &entity);

    Shaders shaders;

    GLuint billboardVao;
    GLuint billboardVbo;

    Shader *spriteShader;
    Shader *backgroundShader;
    Shader *particlesShader;
    Shader *letterShader;
};

#endif //RIFLERANGE_RENDER_H
