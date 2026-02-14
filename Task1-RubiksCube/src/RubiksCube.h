#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Cubie.h>

enum class Wall
{
    RIGHT,  // R
    LEFT,   // L
    UP,     // U
    DOWN,   // D
    FRONT,  // F
    BACK    // B
};

class RubiksCube
{
private:
    std::vector<Cubie> cubies;
    glm::mat4 wholeCubeTransform;  // Global rotation for entire cube
    
    // Wall rotation state
    bool isRotating;
    Wall currentWall;
    float currentRotationAngle;
    float targetRotationAngle;
    float rotationSpeed;
    bool clockwise;  // Moved before directionMultiplier
    int directionMultiplier;  // 1 for clockwise, -1 for counterclockwise
    std::vector<int> rotatingCubieIndices;
    std::vector<glm::mat4> rotatingCubieStartTransforms;  // rubikTransform at start of rotation (for correct accumulation)
    
    // Rotation parameters
    float angleIncrement;  // 90 or 180 degrees
    
public:
    RubiksCube();
    
    // Initialize all 27 cubies (or 26 if center is excluded)
    void Initialize();
    
    // Start a wall rotation
    bool StartWallRotation(Wall wall, float angle, bool cw);
    
    // Update animation (call each frame)
    void Update(float deltaTime);
    
    // Get cubies for rendering
    const std::vector<Cubie>& GetCubies() const { return cubies; }
    
    // Global cube transform accessors
    void SetWholeCubeTransform(const glm::mat4& transform) { wholeCubeTransform = transform; }
    glm::mat4 GetWholeCubeTransform() const { return wholeCubeTransform; }
    
    // Check if currently animating
    bool IsRotating() const { return isRotating; }
    
    // For pivot-based animation: face center (same units as GetBasePosition()) and axis
    static glm::vec3 GetFaceCenter(Wall wall);
    Wall GetCurrentWall() const { return currentWall; }
    float GetCurrentRotationAngle() const { return currentRotationAngle; }
    int GetCurrentRotationSign() const { return clockwise ? 1 : -1; }
    glm::vec3 GetCurrentRotationAxis() const;
    bool IsCubieInCurrentRotation(int cubieIndex) const;
    glm::mat4 GetRotatingCubieStartTransform(int cubieIndex) const;
    
    // Get cubie by index
    Cubie& GetCubie(int index) { return cubies[index]; }
    
    // Get angle increment (for Z/A keys)
    float GetAngleIncrement() const { return angleIncrement; }
    void SetAngleIncrement(float angle) { angleIncrement = angle; }
    
    // Get direction multiplier
    int GetDirectionMultiplier() const { return directionMultiplier; }
    void SetDirectionMultiplier(int mult) { directionMultiplier = mult; }
    
private:
    // Get indices of cubies on a specific wall
    std::vector<int> GetWallCubies(Wall wall);
    
    // Apply rotation to cubies after animation completes
    void ApplyRotationToCubies();
    
    // Update grid positions after rotation
    void UpdateGridPositions(Wall wall, bool cw);
};
