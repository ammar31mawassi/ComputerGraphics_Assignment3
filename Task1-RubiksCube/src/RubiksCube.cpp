#include "RubiksCube.h"
#include <algorithm>
#include <iostream>
#include <cmath>

RubiksCube::RubiksCube()
    : wholeCubeTransform(1.0f),
      isRotating(false),
      currentRotationAngle(0.0f),
      targetRotationAngle(90.0f),
      rotationSpeed(180.0f),  // degrees per second
      clockwise(true),
      directionMultiplier(1),
      angleIncrement(90.0f)
{
    Initialize();
}

void RubiksCube::Initialize()
{
    cubies.clear();
    int id = 0;
    
    // Create 27 cubies in a 3x3x3 grid
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                cubies.push_back(Cubie(id++, x, y, z));
            }
        }
    }
}

std::vector<int> RubiksCube::GetWallCubies(Wall wall)
{
    std::vector<int> indices;
    
    for (size_t i = 0; i < cubies.size(); i++)
    {
        const glm::ivec3& pos = cubies[i].gridPos;
        
        switch (wall)
        {
            case Wall::RIGHT:  // x = 1
                if (pos.x == 1) indices.push_back(i);
                break;
            case Wall::LEFT:   // x = -1
                if (pos.x == -1) indices.push_back(i);
                break;
            case Wall::UP:     // y = 1
                if (pos.y == 1) indices.push_back(i);
                break;
            case Wall::DOWN:   // y = -1
                if (pos.y == -1) indices.push_back(i);
                break;
            case Wall::FRONT:  // z = 1
                if (pos.z == 1) indices.push_back(i);
                break;
            case Wall::BACK:   // z = -1
                if (pos.z == -1) indices.push_back(i);
                break;
        }
    }
    
    return indices;
}

bool RubiksCube::StartWallRotation(Wall wall, float angle, bool cw)
{
    if (isRotating)
    {
        std::cout << "Already rotating! Wait for current rotation to complete." << std::endl;
        return false;
    }
    
    currentWall = wall;
    targetRotationAngle = angle;
    clockwise = cw;
    currentRotationAngle = 0.0f;
    rotatingCubieIndices = GetWallCubies(wall);
    rotatingCubieStartTransforms.clear();
    for (int idx : rotatingCubieIndices)
        rotatingCubieStartTransforms.push_back(cubies[idx].rubikTransform);
    isRotating = true;
    
    std::cout << "Starting " << (cw ? "clockwise" : "counterclockwise") 
              << " rotation of " << angle << " degrees" << std::endl;
    
    return true;
}

glm::vec3 RubiksCube::GetFaceCenter(Wall wall)
{
    const float d = 1.1f;
    switch (wall)
    {
        case Wall::RIGHT:  return glm::vec3( d, 0, 0);
        case Wall::LEFT:   return glm::vec3(-d, 0, 0);
        case Wall::UP:     return glm::vec3(0,  d, 0);
        case Wall::DOWN:   return glm::vec3(0, -d, 0);
        case Wall::FRONT:  return glm::vec3(0, 0,  d);
        case Wall::BACK:   return glm::vec3(0, 0, -d);
    }
    return glm::vec3(0);
}

glm::vec3 RubiksCube::GetCurrentRotationAxis() const
{
    switch (currentWall)
    {
        case Wall::RIGHT:  return glm::vec3(1, 0, 0);
        case Wall::LEFT:   return glm::vec3(-1, 0, 0);
        case Wall::UP:     return glm::vec3(0, 1, 0);
        case Wall::DOWN:   return glm::vec3(0, -1, 0);
        case Wall::FRONT:  return glm::vec3(0, 0, 1);
        case Wall::BACK:   return glm::vec3(0, 0, -1);
    }
    return glm::vec3(1, 0, 0);
}

bool RubiksCube::IsCubieInCurrentRotation(int cubieIndex) const
{
    for (int idx : rotatingCubieIndices)
        if (idx == cubieIndex) return true;
    return false;
}

glm::mat4 RubiksCube::GetRotatingCubieStartTransform(int cubieIndex) const
{
    for (size_t i = 0; i < rotatingCubieIndices.size(); i++)
        if (rotatingCubieIndices[i] == cubieIndex)
            return rotatingCubieStartTransforms[i];
    return glm::mat4(1.0f);
}

void RubiksCube::Update(float deltaTime)
{
    if (!isRotating)
        return;
    
    float step = rotationSpeed * deltaTime;
    currentRotationAngle += step;
    
    if (currentRotationAngle >= targetRotationAngle)
    {
        currentRotationAngle = targetRotationAngle;
        ApplyRotationToCubies();
        UpdateGridPositions(currentWall, clockwise);
        isRotating = false;
        std::cout << "Rotation complete!" << std::endl;
        return;
    }
    // During animation, model matrix is computed in main using pivot in world space (T(face)*R*T(-face)*T(basePos)*start*selection). Do not modify rubikTransform here.
}

void RubiksCube::ApplyRotationToCubies()
{
    glm::vec3 axis;
    float angle = targetRotationAngle * (clockwise ? 1.0f : -1.0f);
    switch (currentWall)
    {
        case Wall::RIGHT:  axis = glm::vec3(1, 0, 0); break;
        case Wall::LEFT:   axis = glm::vec3(-1, 0, 0); break;
        case Wall::UP:     axis = glm::vec3(0, 1, 0); break;
        case Wall::DOWN:   axis = glm::vec3(0, -1, 0); break;
        case Wall::FRONT:  axis = glm::vec3(0, 0, 1); break;
        case Wall::BACK:   axis = glm::vec3(0, 0, -1); break;
    }
    
    // Apply only the rotation to orientation; grid positions are updated in UpdateGridPositions so basePos is already correct.
    glm::mat4 exactRot = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    for (size_t i = 0; i < rotatingCubieIndices.size(); i++)
    {
        int idx = rotatingCubieIndices[i];
        cubies[idx].rubikTransform = exactRot * rotatingCubieStartTransforms[i];
    }
}

void RubiksCube::UpdateGridPositions(Wall wall, bool cw)
{
    // Update logical grid positions after rotation
    // This is crucial for subsequent rotations to work correctly
    
    for (int idx : rotatingCubieIndices)
    {
        glm::ivec3& pos = cubies[idx].gridPos;
        glm::ivec3 newPos = pos;
        
        // Calculate new grid position based on 90-degree rotation
        // For 180-degree rotations, this needs to be applied twice
        int times = static_cast<int>(targetRotationAngle / 90.0f);
        
        for (int t = 0; t < times; t++)
        {
            glm::ivec3 oldPos = newPos;
            
            switch (wall)
            {
                case Wall::RIGHT:  // Rotate around X-axis
                case Wall::LEFT:
                    if (cw ^ (wall == Wall::LEFT))
                        newPos = glm::ivec3(oldPos.x, -oldPos.z, oldPos.y);
                    else
                        newPos = glm::ivec3(oldPos.x, oldPos.z, -oldPos.y);
                    break;
                    
                case Wall::UP:     // Rotate around Y-axis
                case Wall::DOWN:
                    if (cw ^ (wall == Wall::DOWN))
                        newPos = glm::ivec3(oldPos.z, oldPos.y, -oldPos.x);
                    else
                        newPos = glm::ivec3(-oldPos.z, oldPos.y, oldPos.x);
                    break;
                    
                case Wall::FRONT:  // Rotate around Z-axis
                case Wall::BACK:
                    if (cw ^ (wall == Wall::BACK))
                        newPos = glm::ivec3(-oldPos.y, oldPos.x, oldPos.z);
                    else
                        newPos = glm::ivec3(oldPos.y, -oldPos.x, oldPos.z);
                    break;
            }
        }
        
        pos = newPos;
    }
}
