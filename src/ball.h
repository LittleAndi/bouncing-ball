#ifndef BALL_H
#define BALL_H

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

struct Ball
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f, 0.5f, 0.2f); // Orange color
    float radius = 0.3f;
    float mass = 1.0f;

    // Physics constants
    static constexpr float GRAVITY = -9.8f; // m/s^2
    static constexpr float DAMPING = 0.95f; // Energy loss on bounce (5%)

    // Returns the maximum wall-impact speed this frame (0 if no collision occurred)
    float update(float deltaTime, const glm::vec3 &boxMin, const glm::vec3 &boxMax)
    {
        // Apply gravity
        velocity.y += GRAVITY * deltaTime;

        // Update position based on velocity
        position += velocity * deltaTime;

        float maxImpact = 0.0f;

        // Collision detection and response (AABB collision with box boundaries)
        // Check X axis
        if (position.x - radius < boxMin.x)
        {
            maxImpact = std::max(maxImpact, std::abs(velocity.x));
            position.x = boxMin.x + radius;
            velocity.x = -velocity.x * DAMPING;
        }
        else if (position.x + radius > boxMax.x)
        {
            maxImpact = std::max(maxImpact, std::abs(velocity.x));
            position.x = boxMax.x - radius;
            velocity.x = -velocity.x * DAMPING;
        }

        // Check Y axis
        if (position.y - radius < boxMin.y)
        {
            maxImpact = std::max(maxImpact, std::abs(velocity.y));
            position.y = boxMin.y + radius;
            velocity.y = -velocity.y * DAMPING;
        }
        else if (position.y + radius > boxMax.y)
        {
            maxImpact = std::max(maxImpact, std::abs(velocity.y));
            position.y = boxMax.y - radius;
            velocity.y = -velocity.y * DAMPING;
        }

        // Check Z axis
        if (position.z - radius < boxMin.z)
        {
            maxImpact = std::max(maxImpact, std::abs(velocity.z));
            position.z = boxMin.z + radius;
            velocity.z = -velocity.z * DAMPING;
        }
        else if (position.z + radius > boxMax.z)
        {
            maxImpact = std::max(maxImpact, std::abs(velocity.z));
            position.z = boxMax.z - radius;
            velocity.z = -velocity.z * DAMPING;
        }

        // Optional: Add small damping to prevent infinite bouncing
        if (glm::length(velocity) < 0.01f && position.y - radius <= boxMin.y + 0.01f)
        {
            velocity = glm::vec3(0.0f);
        }

        return maxImpact;
    }
};

#endif
