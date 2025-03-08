#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "snake.h"
#include "camera_controller.h"
#include <vector>

enum class ObstacleType {
    TREE,
    ROCK
};

struct Obstacle {
    ObstacleType type;
    Vector3 position;
    float scale;
    float rotation;
};

class Game {
public:
    Game();
    ~Game();
    
    void Initialize();
    void Update();
    void Render();
    void Cleanup();
    
private:
    void SpawnApple();
    void GenerateObstacles();
    bool IsPositionFree(const Vector3& position, float radius);
    bool CheckCollision();
    
    Snake snake;
    CameraController cameraController;
    Vector3 applePosition;
    Model appleModel;
    Texture2D appleTexture;
    
    // Obstacle models
    Model treeModel;
    Model rockModel;
    std::vector<Obstacle> obstacles;
    int maxObstacles;
    
    float arenaSize;
    float moveTimer;
    float moveInterval;
    bool gameOver;
    int score;
};

#endif // GAME_H
