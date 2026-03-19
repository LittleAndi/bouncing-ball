#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include "shader.h" // your wrapper for compiling shaders
#include "ball.h"

int main(int argc, char *argv[])
{
    // Init GLFW
    if (!glfwInit())
    {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Bouncing Ball Engine", NULL, NULL);
    if (window == nullptr)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, 800, 600);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Load shaders
    Shader ourShader("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Parse ball count from CLI arg (default: 3)
    int numBalls = 3;
    if (argc > 1)
        numBalls = std::clamp(std::stoi(argv[1]), 1, 50);

    // Create balls with random positions, velocities, and distinct colors
    auto hsvToRgb = [](float h, float s, float v) -> glm::vec3
    {
        float c = v * s;
        float x = c * (1.0f - std::abs(std::fmod(h * 6.0f, 2.0f) - 1.0f));
        float m = v - c;
        glm::vec3 rgb;
        int sector = static_cast<int>(h * 6.0f) % 6;
        if (sector == 0)
            rgb = {c, x, 0};
        else if (sector == 1)
            rgb = {x, c, 0};
        else if (sector == 2)
            rgb = {0, c, x};
        else if (sector == 3)
            rgb = {0, x, c};
        else if (sector == 4)
            rgb = {x, 0, c};
        else
            rgb = {c, 0, x};
        return rgb + glm::vec3(m);
    };

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> posDist(-1.5f, 1.5f);
    std::uniform_real_distribution<float> velDist(-3.0f, 3.0f);
    std::vector<Ball> balls(numBalls);
    for (int i = 0; i < numBalls; ++i)
    {
        balls[i].position = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
        balls[i].velocity = glm::vec3(velDist(rng), velDist(rng), velDist(rng));
        balls[i].color = hsvToRgb(static_cast<float>(i) / numBalls, 0.85f, 0.95f);
    }

    // Box bounds (cube from -2 to 2 in each axis)
    glm::vec3 boxMin = glm::vec3(-2.0f, -2.0f, -2.0f);
    glm::vec3 boxMax = glm::vec3(2.0f, 2.0f, 2.0f);

    // UV sphere mesh (32 stacks x 32 sectors), radius 0.5, positions + normals
    const int stacks = 32;
    const int sectors = 32;
    const float radius = 0.5f;
    const float PI = glm::pi<float>();
    std::vector<float> sphereVertices;
    sphereVertices.reserve(stacks * sectors * 6 * 6);
    for (int i = 0; i < stacks; ++i)
    {
        float phi0 = PI / 2.0f - i * PI / stacks;
        float phi1 = PI / 2.0f - (i + 1) * PI / stacks;
        for (int j = 0; j < sectors; ++j)
        {
            float theta0 = j * 2.0f * PI / sectors;
            float theta1 = (j + 1) * 2.0f * PI / sectors;

            // Compute 4 corners of the quad
            auto addVertex = [&](float phi, float theta)
            {
                float nx = std::cos(phi) * std::cos(theta);
                float ny = std::sin(phi);
                float nz = std::cos(phi) * std::sin(theta);
                sphereVertices.insert(sphereVertices.end(), {
                                                                nx * radius, ny * radius, nz * radius, // position
                                                                nx, ny, nz                             // normal
                                                            });
            };

            // Triangle 1
            addVertex(phi0, theta0);
            addVertex(phi1, theta0);
            addVertex(phi1, theta1);
            // Triangle 2
            addVertex(phi1, theta1);
            addVertex(phi0, theta1);
            addVertex(phi0, theta0);
        }
    }
    int sphereVertexCount = static_cast<int>(sphereVertices.size() / 6);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Room geometry: floor + back/left/right walls (no ceiling, front left open for viewing)
    // Each surface is a quad (2 triangles), normal faces inward toward the ball
    float roomVertices[] = {
        // Floor (y = -2, normal: 0,1,0)
        -2.0f,
        -2.0f,
        -2.0f,
        0.0f,
        1.0f,
        0.0f,
        2.0f,
        -2.0f,
        -2.0f,
        0.0f,
        1.0f,
        0.0f,
        2.0f,
        -2.0f,
        2.0f,
        0.0f,
        1.0f,
        0.0f,
        2.0f,
        -2.0f,
        2.0f,
        0.0f,
        1.0f,
        0.0f,
        -2.0f,
        -2.0f,
        2.0f,
        0.0f,
        1.0f,
        0.0f,
        -2.0f,
        -2.0f,
        -2.0f,
        0.0f,
        1.0f,
        0.0f,
        // Back wall (z = -2, normal: 0,0,1)
        -2.0f,
        -2.0f,
        -2.0f,
        0.0f,
        0.0f,
        1.0f,
        2.0f,
        -2.0f,
        -2.0f,
        0.0f,
        0.0f,
        1.0f,
        2.0f,
        2.0f,
        -2.0f,
        0.0f,
        0.0f,
        1.0f,
        2.0f,
        2.0f,
        -2.0f,
        0.0f,
        0.0f,
        1.0f,
        -2.0f,
        2.0f,
        -2.0f,
        0.0f,
        0.0f,
        1.0f,
        -2.0f,
        -2.0f,
        -2.0f,
        0.0f,
        0.0f,
        1.0f,
        // Left wall (x = -2, normal: 1,0,0)
        -2.0f,
        -2.0f,
        -2.0f,
        1.0f,
        0.0f,
        0.0f,
        -2.0f,
        -2.0f,
        2.0f,
        1.0f,
        0.0f,
        0.0f,
        -2.0f,
        2.0f,
        2.0f,
        1.0f,
        0.0f,
        0.0f,
        -2.0f,
        2.0f,
        2.0f,
        1.0f,
        0.0f,
        0.0f,
        -2.0f,
        2.0f,
        -2.0f,
        1.0f,
        0.0f,
        0.0f,
        -2.0f,
        -2.0f,
        -2.0f,
        1.0f,
        0.0f,
        0.0f,
        // Right wall (x = 2, normal: -1,0,0)
        2.0f,
        -2.0f,
        -2.0f,
        -1.0f,
        0.0f,
        0.0f,
        2.0f,
        -2.0f,
        2.0f,
        -1.0f,
        0.0f,
        0.0f,
        2.0f,
        2.0f,
        2.0f,
        -1.0f,
        0.0f,
        0.0f,
        2.0f,
        2.0f,
        2.0f,
        -1.0f,
        0.0f,
        0.0f,
        2.0f,
        2.0f,
        -2.0f,
        -1.0f,
        0.0f,
        0.0f,
        2.0f,
        -2.0f,
        -2.0f,
        -1.0f,
        0.0f,
        0.0f,
    };

    unsigned int roomVAO, roomVBO;
    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);
    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Generate PCM data for two sounds
    const int sampleRate = 44100;
    const float soundDuration = 0.25f;
    const int frameCount = static_cast<int>(sampleRate * soundDuration);

    // Ball-ball collision: 220 Hz damped sine
    std::vector<float> pcmBallBall(frameCount);
    for (int i = 0; i < frameCount; ++i)
    {
        float t = static_cast<float>(i) / sampleRate;
        pcmBallBall[i] = std::sin(2.0f * glm::pi<float>() * 220.0f * t) * std::exp(-20.0f * t);
    }

    // Wall collision: 110 Hz damped sine (lower pitch)
    std::vector<float> pcmWall(frameCount);
    for (int i = 0; i < frameCount; ++i)
    {
        float t = static_cast<float>(i) / sampleRate;
        pcmWall[i] = std::sin(2.0f * glm::pi<float>() * 110.0f * t) * std::exp(-20.0f * t);
    }

    ma_engine audioEngine;
    ma_engine_init(NULL, &audioEngine);

    ma_audio_buffer_config ballBallConfig = ma_audio_buffer_config_init(
        ma_format_f32, 1, static_cast<ma_uint64>(frameCount), pcmBallBall.data(), NULL);
    ma_audio_buffer ballBallBuffer;
    ma_audio_buffer_init(&ballBallConfig, &ballBallBuffer);

    ma_audio_buffer_config wallConfig = ma_audio_buffer_config_init(
        ma_format_f32, 1, static_cast<ma_uint64>(frameCount), pcmWall.data(), NULL);
    ma_audio_buffer wallBuffer;
    ma_audio_buffer_init(&wallConfig, &wallBuffer);

    ma_sound ballBallSound;
    ma_sound_init_from_data_source(&audioEngine, &ballBallBuffer,
                                   MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, &ballBallSound);
    ma_sound wallSound;
    ma_sound_init_from_data_source(&audioEngine, &wallBuffer,
                                   MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, &wallSound);

    // Render loop
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Update ball physics (wall collisions)
        float wallImpact = 0.0f;
        for (auto &b : balls)
            wallImpact = std::max(wallImpact, b.update(deltaTime, boxMin, boxMax));

        // Resolve ball-ball collisions
        float ballImpact = 0.0f;
        for (int i = 0; i < static_cast<int>(balls.size()); ++i)
            for (int j = i + 1; j < static_cast<int>(balls.size()); ++j)
                ballImpact = std::max(ballImpact, resolveCollision(balls[i], balls[j]));

        if (wallImpact > 0.5f)
        {
            float volume = std::min(1.0f, std::max(0.05f, wallImpact / 10.0f));
            ma_sound_seek_to_pcm_frame(&wallSound, 0);
            ma_sound_set_volume(&wallSound, volume);
            ma_sound_start(&wallSound);
        }
        if (ballImpact > 0.5f)
        {
            float volume = std::min(1.0f, std::max(0.05f, ballImpact / 10.0f));
            ma_sound_seek_to_pcm_frame(&ballBallSound, 0);
            ma_sound_set_volume(&ballBallSound, volume);
            ma_sound_start(&ballBallSound);
        }

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 camPos = glm::vec3(0.0f, 2.0f, 6.0f);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(55.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        ourShader.use();
        ourShader.setVec3("lightColor", glm::vec3(1.0f));
        ourShader.setVec3("lightPos", glm::vec3(0.0f, 3.5f, 1.0f));
        ourShader.setVec3("viewPos", camPos);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);

        // Draw room (white, no model transform)
        ourShader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", glm::mat4(1.0f));
        glBindVertexArray(roomVAO);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        // Draw balls
        for (const auto &b : balls)
        {
            ourShader.setVec3("objectColor", b.color);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, b.position);
            model = glm::scale(model, glm::vec3(b.radius * 2.0f));
            ourShader.setMat4("model", model);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &roomVAO);
    glDeleteBuffers(1, &roomVBO);
    ma_sound_uninit(&ballBallSound);
    ma_sound_uninit(&wallSound);
    ma_audio_buffer_uninit(&ballBallBuffer);
    ma_audio_buffer_uninit(&wallBuffer);
    ma_engine_uninit(&audioEngine);
    glfwTerminate();
    return 0;
}