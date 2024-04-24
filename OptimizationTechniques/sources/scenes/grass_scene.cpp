#include "grass_scene.h"

float doubleQuadVertices[] = {
    // positions         // uvs
    -0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // bottom-left
     0.5f, -0.5f,  0.0f,  0.0f,  0.0f, // bottom-right
     0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // top-right
     0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // top-right
    -0.5f,  0.5f,  0.0f,  1.0f,  1.0f, // top-left
    -0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // bottom-left
     0.0f, -0.5f, -0.5f,  1.0f,  0.0f, // bottom-back
     0.0f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-front
     0.0f,  0.5f,  0.5f,  0.0f,  1.0f, // top-front
     0.0f,  0.5f,  0.5f,  0.0f,  1.0f, // top-front
     0.0f,  0.5f, -0.5f,  1.0f,  1.0f, // top-back
     0.0f, -0.5f, -0.5f,  1.0f,  0.0f, // bottom-back
};

float triangleVertices[] = {
    // positions         // normals
    -0.5f, -0.5f,  0.0f,  0.0f,  0.0f,  1.0f, // bottom-left
     0.5f, -0.5f,  0.0f,  0.0f,  0.0f,  1.0f, // bottom-right
     0.0f,  0.5f,  0.0f,  0.0f,  0.0f,  1.0f, // top
};

GrassScene::GrassScene()
	: Scene(), currGrassType(GrassType::MONOCHROMATIC), nextGrassType(GrassType::MONOCHROMATIC),
      grassRenderShader(nullptr), vao(nullptr), vbo(nullptr), instanceMatrices(nullptr), modelMatrices(nullptr), instances(10000),
      windDirection(1.0f, 0.0f, 1.0f), windIntensity(0.5f), time(),
      texture(nullptr),
      shadowMapRender(nullptr), shadowMapSize(8192), shadowMap(nullptr), shadowMapRenderer(nullptr), renderShadowMap(false)
{
}

void GrassScene::setup()
{
    std::size_t vec4_s = sizeof(glm::vec4);
    int axisLim = int(std::sqrtf(float(instances)));
    int axisOffset = axisLim / 2;

    if (currGrassType == GrassType::TEXTURIZED)
    {
        float grassDensity = 8.0f;
	    
        grassRenderShader = new ShaderProgram("sources/shaders/render_texturized_grass_vs.glsl", "sources/shaders/render_texturized_grass_fs.glsl");
    
	    modelMatrices = new glm::mat4[instances];

        for (int x = 0; x < axisLim; ++x)
        {
            for (int z = 0; z < axisLim; ++z)
            {
                float xOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float zOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                glm::vec3 position = glm::vec3(x + xOffset - axisOffset, 0.0f, z + zOffset - axisOffset) / grassDensity;
                glm::mat4 model = glm::mat4(1.0f);
                int index = x * axisLim + z;

                model = glm::translate(model, position);
                model = glm::scale(model, glm::vec3(0.5f));

                modelMatrices[index] = model;
            }
        }

        vao = new VAO();
        vbo = new VBO(doubleQuadVertices, sizeof(doubleQuadVertices));
        instanceMatrices = new VBO(&modelMatrices[0], instances * sizeof(glm::mat4));

        vao->bind();
        vbo->bind();

        vao->setVertexAttribute(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
        vao->setVertexAttribute(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        instanceMatrices->bind();

        vao->setVertexAttribute(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(0), 1);
        vao->setVertexAttribute(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(1 * vec4_s), 1);
        vao->setVertexAttribute(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(2 * vec4_s), 1);
        vao->setVertexAttribute(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(3 * vec4_s), 1);

        vao->unbind(); // Unbind VAO before another buffer.
        vbo->unbind();
        instanceMatrices->unbind();

	    texture = new Texture("resources/textures/grass1.png");
    }
    else if (currGrassType == GrassType::MONOCHROMATIC)
    {
        float grassDensity = 8.0f;

        grassRenderShader = new ShaderProgram("sources/shaders/render_monochromatic_grass_vs.glsl", "sources/shaders/render_monochromatic_grass_fs.glsl");

        modelMatrices = new glm::mat4[instances];

        for (int x = 0; x < axisLim; ++x)
        {
            for (int z = 0; z < axisLim; ++z)
            {
                float xOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float zOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float rOffset = static_cast<float>(rand()) / 360.0f;

                glm::vec3 position = glm::vec3(x + xOffset - axisOffset, 0.0f, z + zOffset - axisOffset) / grassDensity;
                glm::mat4 model = glm::mat4(1.0f);
                int index = x * axisLim + z;

                // TODO: give a random rotation.
                model = glm::rotate(model, glm::radians(rOffset), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, position);
                model = glm::scale(model, glm::vec3(0.15f, 1.0f, 0.15f));

                modelMatrices[index] = model;
            }
        }

        vao = new VAO();
        vbo = new VBO(triangleVertices, sizeof(triangleVertices));
        instanceMatrices = new VBO(&modelMatrices[0], instances * sizeof(glm::mat4));

        vao->bind();
        vbo->bind();

        vao->setVertexAttribute(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
        vao->setVertexAttribute(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        instanceMatrices->bind();

        vao->setVertexAttribute(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(0), 1);
        vao->setVertexAttribute(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(1 * vec4_s), 1);
        vao->setVertexAttribute(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(2 * vec4_s), 1);
        vao->setVertexAttribute(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4_s, (void*)(3 * vec4_s), 1);

        vao->unbind(); // Unbind VAO before another buffer.
        vbo->unbind();
        instanceMatrices->unbind();

        shadowMapRender = new ShaderProgram("sources/shaders/render_monochromatic_grass_shadow_map_vs.glsl", "sources/shaders/render_monochromatic_grass_shadow_map_fs.glsl");

        shadowMap = new DepthMap(shadowMapSize, shadowMapSize);

        shadowMapRenderer = new DepthMapRenderer();

        shadowMapRenderer->setup();
    }
}

void GrassScene::clean()
{
    if (currGrassType == GrassType::TEXTURIZED)
    {
        grassRenderShader->clean();
        vao->clean();
        vbo->clean();
        instanceMatrices->clean();
        texture->clean();

        delete grassRenderShader;
        delete vao;
        delete vbo;
        delete instanceMatrices;
        delete texture;

        delete[] modelMatrices;
    }
    else if (currGrassType == GrassType::MONOCHROMATIC)
    {
        grassRenderShader->clean();
        vao->clean();
        vbo->clean();
        instanceMatrices->clean();
        shadowMapRender->clean();
        shadowMap->clean();
        shadowMapRenderer->clean();

        delete grassRenderShader;
        delete vao;
        delete vbo;
        delete instanceMatrices;
        delete shadowMapRender;
        delete shadowMap;
        delete shadowMapRenderer;

        delete[] modelMatrices;
    }
}

void GrassScene::update(float deltaTime)
{
    if (time + deltaTime >= 360.0f)
    {
        time = 0.0f;
    }
    else
    {
        time += deltaTime;
    }

    if (currGrassType != nextGrassType)
    {
        clean();

        currGrassType = nextGrassType;

        setup();
    }
}

void GrassScene::render(const Camera& camera, float deltaTime)
{
    if (currGrassType == GrassType::TEXTURIZED)
    {
        grassRenderShader->bind();
        texture->bind(0);
        vao->bind();

        grassRenderShader->setUniformMatrix4fv("uProjectionMatrix", camera.getProjectionMatrix());
        grassRenderShader->setUniformMatrix4fv("uViewMatrix", camera.getViewMatrix());
        grassRenderShader->setUniform1i("uTexture", 0);
        grassRenderShader->setUniform3f("uWindDirection", windDirection);
        grassRenderShader->setUniform1f("uWindIntensity", windIntensity);
        grassRenderShader->setUniform1f("uTime", time);

        glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 12, instances);

        vao->unbind();
        texture->unbind();
        grassRenderShader->unbind();
    }
    else if (currGrassType == GrassType::MONOCHROMATIC)
    {
        int viewport[4];
        // float x = 10.0f * glm::cos(time / 2.0f);
        // float z = 10.0f * glm::sin(time / 2.0f);

        // glm::vec3 lightPosition = glm::vec3(x, 10.0f, z);
        glm::vec3 lightPosition(10.0f, 10.0f, 10.0f);

        glm::mat4 lightProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 25.0f);
        glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

        shadowMap->bindDepthBuffer(0);
        vao->bind();

        // Render shadow map.
        shadowMap->bind();
        shadowMapRender->bind();

        shadowMapRender->setUniformMatrix4fv("uLightSpaceMatrix", lightSpaceMatrix);
        shadowMapRender->setUniform3f("uWindDirection", windDirection);
        shadowMapRender->setUniform1f("uWindIntensity", windIntensity);
        shadowMapRender->setUniform1f("uTime", time);

        glGetIntegerv(GL_VIEWPORT, viewport); // Save current viewport.
        glViewport(0, 0, shadowMapSize, shadowMapSize);

        glClear(GL_DEPTH_BUFFER_BIT);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, instances);

        shadowMapRender->unbind();
        shadowMap->unbind();

        // Render scene.
        if (!renderShadowMap)
        {
            grassRenderShader->bind();

            grassRenderShader->setUniformMatrix4fv("uProjectionMatrix", camera.getProjectionMatrix());
            grassRenderShader->setUniformMatrix4fv("uViewMatrix", camera.getViewMatrix());
            grassRenderShader->setUniformMatrix4fv("uLightSpaceMatrix", lightSpaceMatrix);
            grassRenderShader->setUniform3f("uWindDirection", windDirection);
            grassRenderShader->setUniform1f("uWindIntensity", windIntensity);
            grassRenderShader->setUniform1f("uTime", time);

            grassRenderShader->setUniform3f("uLight.ambient", glm::vec3(0.5f, 0.5f, 0.5f));
            grassRenderShader->setUniform3f("uLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
            grassRenderShader->setUniform3f("uLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
            grassRenderShader->setUniform3f("uLight.position", lightPosition);
            grassRenderShader->setUniform3f("uMaterial.diffuse", glm::vec3(0.1f, 0.5f, 0.3f));
            grassRenderShader->setUniform3f("uMaterial.specular", glm::vec3(0.2f, 0.6f, 0.4f));
            grassRenderShader->setUniform1f("uMaterial.shininess", 64.0f);
            grassRenderShader->setUniform1i("uShadowMap", 0);
            grassRenderShader->setUniform3f("uViewPos", camera.getPosition());

            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

            glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 3, instances);
    
            grassRenderShader->unbind();
        }
        else
        {
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shadowMapRenderer->render(0);
        }

        vao->unbind();
        shadowMap->unbindDepthBuffer();
    }
}

void GrassScene::processGUI()
{
    bool dialogOpen = true;
    ImGui::Begin("Grass Rendering Dialog", &dialogOpen, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Grass Type"))
        {
            if (ImGui::MenuItem("Texturized", "1", currGrassType == GrassType::TEXTURIZED))
            {
                nextGrassType = GrassType::TEXTURIZED;
            }

            if (ImGui::MenuItem("Monochromatic", "2", currGrassType == GrassType::MONOCHROMATIC))
            {
                nextGrassType = GrassType::MONOCHROMATIC;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (currGrassType == GrassType::MONOCHROMATIC)
    {
        ImGui::Text("%i instances (%i vertices)", instances, 3 * instances);
    }

    ImGui::DragFloat3("Wind Direction", &windDirection[0], 0.5f);
    ImGui::SliderFloat("Wind Intensity", &windIntensity, 0.005f, 5.0f);

    if (currGrassType == GrassType::MONOCHROMATIC)
    {
        ImGui::Checkbox("Render Shadow Map", &renderShadowMap);
    }

    ImGui::End();
}
