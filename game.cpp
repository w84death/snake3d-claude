#include "game.h"
#include <cstdlib>
#include <ctime>
#include <cmath>      // Add for fmax
#include "raymath.h"  // Add for Vector3Distance

Game::Game() : 
    arenaSize(20.0f), 
    moveTimer(0.0f), 
    moveInterval(0.2f),
    gameOver(false),
    score(0),
    maxObstacles(15) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

Game::~Game() {
}

void Game::Initialize() {
    // Initialize snake at center of arena
    snake.Initialize(Vector3{0.0f, 0.5f, 0.0f});
    
    // Initialize camera controller
    cameraController.Initialize(&snake);
    
    // Create apple model (red sphere) with more appetizing color
    appleModel = LoadModelFromMesh(GenMeshSphere(0.5f, 12, 12));
    appleTexture = LoadTexture("resources/apple_texture.png");
    Material material = LoadMaterialDefault();
    material.maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 220, 20, 60, 255 }; // Crimson red
    appleModel.materials[0] = material;
    
    // Create obstacle models
    treeModel = LoadModelFromMesh(GenMeshCone(0.7f, 2.0f, 8));
    Material treeMaterial = LoadMaterialDefault();
    treeMaterial.maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 34, 139, 34, 255 }; // Forest green
    treeModel.materials[0] = treeMaterial;
    
    rockModel = LoadModelFromMesh(GenMeshSphere(0.8f, 6, 6));
    Material rockMaterial = LoadMaterialDefault();
    rockMaterial.maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 169, 169, 169, 255 }; // Dark grey
    rockModel.materials[0] = rockMaterial;
    
    // Generate obstacles
    GenerateObstacles();
    
    // Spawn first apple
    SpawnApple();
}

void Game::Update() {
    if (gameOver) {
        if (IsKeyPressed(KEY_R)) {
            // Reset game
            snake.Reset(Vector3{0.0f, 0.5f, 0.0f});
            SpawnApple();
            moveInterval = 0.2f;
            score = 0;
            gameOver = false;
        }
        return;
    }

    float deltaTime = GetFrameTime();

    // Handle input for snake direction - adjusted for isometric view
    // Based on the camera angle (45 degrees), we need to map the arrow keys differently
    if (IsKeyPressed(KEY_UP)) {
        snake.SetDirection(Direction::UP);
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        snake.SetDirection(Direction::DOWN);
    }
    else if (IsKeyPressed(KEY_RIGHT)) {
        snake.SetDirection(Direction::RIGHT);
    }
    else if (IsKeyPressed(KEY_LEFT)) {
        snake.SetDirection(Direction::LEFT);
    }
    
    // Add WASD controls as an alternative that may feel more intuitive with this camera angle
    if (IsKeyPressed(KEY_W)) {
        snake.SetDirection(Direction::UP);
    }
    else if (IsKeyPressed(KEY_S)) {
        snake.SetDirection(Direction::DOWN);
    }
    else if (IsKeyPressed(KEY_D)) {
        snake.SetDirection(Direction::RIGHT);
    }
    else if (IsKeyPressed(KEY_A)) {
        snake.SetDirection(Direction::LEFT);
    }
    
    // Update snake movement interpolation
    snake.Update(deltaTime);
    
    // Move snake based on timer
    moveTimer += deltaTime;
    if (moveTimer >= moveInterval) {
        snake.Move();
        moveTimer = 0.0f;
        
        // Check for collisions
        if (CheckCollision()) {
            gameOver = true;
        }
    }
    
    // Update camera position
    cameraController.Update();
}

void Game::Render() {
    BeginDrawing();
    ClearBackground(SKYBLUE);
    
    // Set fog parameters
    Color fogColor = (Color){ 200, 220, 240, 255 };  // Light blue-gray fog
    
    BeginMode3D(cameraController.GetCamera());
    
    // Draw extended terrain with clear depth separation
    float extendedSize = arenaSize * 1.5f;
    
    // Draw base terrain (lowest level) - darker grass for outer area
    DrawPlane(Vector3{0.0f, -0.1f, 0.0f}, Vector2{extendedSize * 2.0f, extendedSize * 2.0f}, 
              (Color){ 65, 160, 20, 255 });
    
    // Draw playable area slightly elevated for clear separation
    DrawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{arenaSize * 2.0f, arenaSize * 2.0f}, 
              (Color){ 76, 187, 23, 255 });
    
    // Draw borders with clearer positioning
    float wallHeight = 1.0f;
    float wallOffset = 0.5f;
    float wallThickness = 1.0f;
    Color wallColor = (Color){ 139, 134, 130, 255 };
    
    // Draw walls with explicit z-ordering
    DrawCube(Vector3{0, wallHeight/2, arenaSize + wallOffset}, arenaSize*2 + wallThickness, wallHeight, wallThickness, wallColor);
    DrawCube(Vector3{0, wallHeight/2, -arenaSize - wallOffset}, arenaSize*2 + wallThickness, wallHeight, wallThickness, wallColor);
    DrawCube(Vector3{arenaSize + wallOffset, wallHeight/2, 0}, wallThickness, wallHeight, arenaSize*2 + wallThickness, wallColor);
    DrawCube(Vector3{-arenaSize - wallOffset, wallHeight/2, 0}, wallThickness, wallHeight, arenaSize*2 + wallThickness, wallColor);
    
    // Draw corner posts with clear z-ordering
    float postSize = 1.2f;
    DrawCube(Vector3{arenaSize + wallOffset/2, wallHeight/2, arenaSize + wallOffset/2}, postSize, wallHeight*1.5f, postSize, wallColor);
    DrawCube(Vector3{-arenaSize - wallOffset/2, wallHeight/2, arenaSize + wallOffset/2}, postSize, wallHeight*1.5f, postSize, wallColor);
    DrawCube(Vector3{arenaSize + wallOffset/2, wallHeight/2, -arenaSize - wallOffset/2}, postSize, wallHeight*1.5f, postSize, wallColor);
    DrawCube(Vector3{-arenaSize - wallOffset/2, wallHeight/2, -arenaSize - wallOffset/2}, postSize, wallHeight*1.5f, postSize, wallColor);
    
    // Create a deterministic seed for consistent decorative object placement
    srand(42);
    
    // Add decorative objects around the area with height variation to prevent z-fighting
    for (int i = 0; i < 24; i++) {
        float angle = (float)i * 15.0f * DEG2RAD;
        
        // Vary the distance to prevent objects from aligning on the same Z
        float distance = extendedSize * 0.9f + ((i % 3) * 0.4f);
        
        float x = sinf(angle) * distance;
        float z = cosf(angle) * distance;
        
        // Add slight height variation for each object to prevent z-fighting
        float baseY = -0.1f + ((i % 5) * 0.02f);
        float scale = 0.8f + ((i * 13) % 50) / 100.0f;  // Deterministic but varied scale
        
        if (i % 2 == 0) {
            // Draw tree with adjusted base height
            DrawModelEx(treeModel, 
                       Vector3{x, baseY + 1.0f, z}, 
                       Vector3{0, 1, 0}, 
                       (float)(i * 30), 
                       Vector3{scale * 1.2f, scale * 1.2f, scale * 1.2f}, 
                       WHITE);
            
            // Draw trunk
            DrawCylinder(
                Vector3{x, baseY + 0.4f, z},
                0.2f * scale * 1.2f,
                0.2f * scale * 1.2f,
                0.8f,
                8,
                (Color){ 139, 69, 19, 255}
            );
        } else {
            // Draw rock with varied height
            DrawModelEx(rockModel,
                      Vector3{x, baseY + 0.05f + ((i % 3) * 0.03f), z},  // Slight height variation
                      Vector3{0, 1, 0},
                      (float)(i * 30),
                      Vector3{scale * 1.5f, scale * 0.9f, scale * 1.5f},
                      (Color){ 150, 150, 150, 255 });
        }
    }
    
    // Restore random seed for gameplay elements
    srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Draw obstacles within the playable area
    for (const auto& obs : obstacles) {
        if (obs.type == ObstacleType::TREE) {
            // Draw tree (cone)
            DrawModelEx(treeModel, 
                       Vector3{obs.position.x, obs.position.y + 1.0f, obs.position.z}, 
                       Vector3{0, 1, 0}, 
                       obs.rotation, 
                       Vector3{obs.scale, obs.scale, obs.scale}, 
                       WHITE);
            
            // Draw tree trunk (cylinder)
            DrawCylinder(
                Vector3{obs.position.x, obs.position.y + 0.4f, obs.position.z},
                0.2f * obs.scale,
                0.2f * obs.scale,
                0.8f,
                8,
                (Color){ 139, 69, 19, 255} // Brown
            );
        } else {
            // Draw rock
            DrawModelEx(rockModel,
                      obs.position,
                      Vector3{0, 1, 0},
                      obs.rotation,
                      Vector3{obs.scale, obs.scale * 0.6f, obs.scale},
                      WHITE);
        }
    }
    
    // Draw snake
    snake.Draw();
    
    // Draw apple with slight shine effect
    DrawModel(appleModel, applePosition, 1.0f, WHITE);
    DrawSphere(Vector3Add(applePosition, (Vector3){ 0.15f, 0.15f, 0.15f }), 0.1f, (Color){ 255, 255, 255, 180 });
    
    // Implement manual fog effect - move it out of objects' way
    DrawCube(Vector3{0, arenaSize * 1.5f, 0}, arenaSize*4, arenaSize*3, arenaSize*4, 
             ColorAlpha(fogColor, 0.03f));  // More subtle and higher up
    
    EndMode3D();
    
    // Draw UI
    DrawText(TextFormat("SCORE: %d", score), 10, 10, 20, WHITE);
    
    if (gameOver) {
        DrawText("GAME OVER", GetScreenWidth()/2 - MeasureText("GAME OVER", 40)/2, 
                GetScreenHeight()/2 - 40, 40, RED);
        DrawText("PRESS R TO RESTART", GetScreenWidth()/2 - MeasureText("PRESS R TO RESTART", 20)/2, 
                GetScreenHeight()/2 + 10, 20, WHITE);
    }
    
    EndDrawing();
}

void Game::Cleanup() {
    UnloadTexture(appleTexture);
    UnloadModel(appleModel);
    UnloadModel(treeModel);
    UnloadModel(rockModel);
}

void Game::SpawnApple() {
    // Random position within arena bounds
    float x = (float)(rand() % (int)(arenaSize * 2) - arenaSize);
    float z = (float)(rand() % (int)(arenaSize * 2) - arenaSize);
    
    // Try to find a valid position for the apple
    bool validPosition = false;
    int attempts = 0;
    
    while (!validPosition && attempts < 50) {
        validPosition = true;
        
        // Check if position is too close to any snake segment
        for (const auto& segment : snake.GetSegments()) {
            if (Vector3Distance(Vector3{x, 0.5f, z}, segment) < 1.0f) {
                validPosition = false;
                x = (float)(rand() % (int)(arenaSize * 2) - arenaSize);
                z = (float)(rand() % (int)(arenaSize * 2) - arenaSize);
                break;
            }
        }
        
        // Check if position is too close to any obstacle
        if (validPosition) {
            for (const auto& obs : obstacles) {
                float obstacleRadius = (obs.type == ObstacleType::TREE) ? 0.7f : 0.8f;
                if (Vector3Distance(Vector3{x, 0.5f, z}, obs.position) < obstacleRadius + 1.0f) {
                    validPosition = false;
                    x = (float)(rand() % (int)(arenaSize * 2) - arenaSize);
                    z = (float)(rand() % (int)(arenaSize * 2) - arenaSize);
                    break;
                }
            }
        }
        
        attempts++;
    }
    
    applePosition = Vector3{x, 0.5f, z};
}

bool Game::CheckCollision() {
    const auto& segments = snake.GetSegments();
    const Vector3& head = segments.front();
    
    // Check collision with apple
    if (Vector3Distance(head, applePosition) < 1.0f) {
        snake.Grow();
        SpawnApple();
        score += 10;
        
        // Adjust speed more gradually as snake grows
        moveInterval = fmax(0.08f, 0.2f - (snake.GetSegments().size() - 3) * 0.005f);
        
        return false;
    }
    
    // Check collision with walls
    if (head.x < -arenaSize || head.x > arenaSize || 
        head.z < -arenaSize || head.z > arenaSize) {
        return true;
    }
    
    // Check collision with self (skip head)
    for (size_t i = 1; i < segments.size(); ++i) {
        if (Vector3Distance(head, segments[i]) < 0.5f) {
            return true;
        }
    }
    
    // Check collision with obstacles
    for (const auto& obs : obstacles) {
        if (obs.type == ObstacleType::TREE) {
            // For trees, check collision with trunk
            Vector3 trunkPosition = Vector3{
                obs.position.x,
                obs.position.y + 0.4f,  // Center of trunk
                obs.position.z
            };
            
            // Use horizontal distance to check collision with trunk
            float dx = head.x - trunkPosition.x;
            float dz = head.z - trunkPosition.z;
            float horizontalDistance = sqrt(dx*dx + dz*dz);
            
            if (horizontalDistance < 0.3f * obs.scale) {
                return true;
            }
        } else {
            // For rocks, use simple sphere collision
            if (Vector3Distance(head, obs.position) < 0.7f * obs.scale) {
                return true;
            }
        }
    }
    
    return false;
}

void Game::GenerateObstacles() {
    obstacles.clear();
    
    // Keep a minimum distance from the center where the snake starts
    const float minDistanceFromCenter = 4.0f;
    
    for (int i = 0; i < maxObstacles; i++) {
        Obstacle obs;
        
        // Decide if it's a tree or rock
        obs.type = (rand() % 2 == 0) ? ObstacleType::TREE : ObstacleType::ROCK;
        
        bool validPosition = false;
        int attempts = 0;
        
        while (!validPosition && attempts < 20) {
            // Random position within arena bounds
            float x = (float)(rand() % (int)(arenaSize * 1.8f) - arenaSize * 0.9f);
            float z = (float)(rand() % (int)(arenaSize * 1.8f) - arenaSize * 0.9f);
            
            // Check distance from center
            if (Vector3Length(Vector3{x, 0, z}) < minDistanceFromCenter) {
                attempts++;
                continue;
            }
            
            obs.position = Vector3{x, 0.0f, z};
            obs.rotation = (float)(rand() % 360);
            obs.scale = 0.8f + (rand() % 50) / 100.0f; // 0.8 to 1.3
            
            if (IsPositionFree(obs.position, obs.type == ObstacleType::TREE ? 1.0f : 0.8f)) {
                validPosition = true;
            }
            
            attempts++;
        }
        
        if (validPosition) {
            obstacles.push_back(obs);
        }
    }
}

bool Game::IsPositionFree(const Vector3& position, float radius) {
    // Check distance from other obstacles
    for (const auto& obs : obstacles) {
        if (Vector3Distance(position, obs.position) < radius * 2.0f) {
            return false;
        }
    }
    
    // Make sure not too close to walls - use smaller margin to allow obstacles closer to walls
    float margin = radius * 1.2f;
    if (position.x > arenaSize - margin || position.x < -arenaSize + margin ||
        position.z > arenaSize - margin || position.z < -arenaSize + margin) {
        return false;
    }
    
    return true;
}
