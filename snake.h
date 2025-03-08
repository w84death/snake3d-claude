#ifndef SNAKE_H
#define SNAKE_H

#include "raylib.h"
#include <vector>

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Snake {
public:
    Snake();
    ~Snake();
    
    void Initialize(const Vector3& startPos);
    void Reset(const Vector3& startPos);
    void Move();
    void Update(float deltaTime);  // New update method for smooth movement
    void Grow();
    void Draw();
    
    Direction GetDirection() const;
    void SetDirection(Direction dir);
    
    const std::vector<Vector3>& GetSegments() const;
    float GetLength() const;
    
private:
    std::vector<Vector3> segments;        // Current visual positions
    std::vector<Vector3> targetPositions; // Target grid positions
    Direction direction;
    Direction nextDirection;
    Model sphereModel;
    Texture2D snakeTexture;
    bool shouldGrow;
    float moveSpeed;                      // Speed for smooth movement
    bool isMoving;                        // Flag to track if currently moving
};

#endif // SNAKE_H
