#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Represents a single small cube in the Rubik's cube
class Cubie
{
public:
    int id;                           // Unique identifier
    glm::ivec3 gridPos;              // Position in 3x3x3 grid (-1, 0, 1)
    glm::mat4 rubikTransform;        // Transformations from wall rotations
    glm::mat4 selectionTransform;    // Transformations from user picking/manipulation
    
    Cubie(int _id, int x, int y, int z)
        : id(_id), 
          gridPos(x, y, z),
          rubikTransform(1.0f),
          selectionTransform(1.0f)
    {}
    
    // Get the base position for this cubie in world space
    glm::vec3 GetBasePosition() const
    {
        return glm::vec3(gridPos.x * 1.1f, gridPos.y * 1.1f, gridPos.z * 1.1f);
    }
    
    // Get full model matrix for this cubie
    glm::mat4 GetModelMatrix() const
    {
        // Start with translation to grid position
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), GetBasePosition());
        // Apply Rubik's cube rotation state, then selection transforms
        return trans * rubikTransform * selectionTransform;
    }
};
