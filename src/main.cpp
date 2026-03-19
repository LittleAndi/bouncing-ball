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
#include "shader.h" // your wrapper for compiling shaders
#include "ball.h"

int main()
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

    // Create ball
    Ball ball;
    ball.position = glm::vec3(0.0f, 0.5f, 0.0f);
    ball.velocity = glm::vec3(2.0f, 1.5f, -1.0f); // m/s

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

    // Generate bounce sound PCM data: 220 Hz damped sine wave (~250 ms thud)
    const int sampleRate = 44100;
    const float soundDuration = 0.25f;
    const int frameCount = static_cast<int>(sampleRate * soundDuration);
    std::vector<float> pcmData(frameCount);
    for (int i = 0; i < frameCount; ++i)
    {
        float t = static_cast<float>(i) / sampleRate;
        pcmData[i] = std::sin(2.0f * glm::pi<float>() * 220.0f * t) * std::exp(-20.0f * t);
    }

    ma_engine audioEngine;
    ma_engine_init(NULL, &audioEngine);

    ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
        ma_format_f32, 1, static_cast<ma_uint64>(frameCount), pcmData.data(), NULL);
    ma_audio_buffer audioBuffer;
    ma_audio_buffer_init(&bufferConfig, &audioBuffer);

    ma_sound bounceSound;
    ma_sound_init_from_data_source(&audioEngine, &audioBuffer,
                                   MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, &bounceSound);

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

        // Update ball physics
        float impact = ball.update(deltaTime, boxMin, boxMax);
        if (impact > 0.5f)
        {
            float volume = std::min(1.0f, std::max(0.05f, impact / 10.0f));
            ma_sound_seek_to_pcm_frame(&bounceSound, 0);
            ma_sound_set_volume(&bounceSound, volume);
            ma_sound_start(&bounceSound);
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

        // Draw ball
        ourShader.setVec3("objectColor", ball.color);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, ball.position);
        model = glm::scale(model, glm::vec3(ball.radius * 2.0f));
        ourShader.setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &roomVAO);
    glDeleteBuffers(1, &roomVBO);
    ma_sound_uninit(&bounceSound);
    ma_audio_buffer_uninit(&audioBuffer);
    ma_engine_uninit(&audioEngine);
    glfwTerminate();
    return 0;
}