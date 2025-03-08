#include "snake.h"
#include <cstddef>  // Add this for size_t
#include "raymath.h"  // For Vector3 operations

Snake::Snake() : 
    direction(Direction::RIGHT),
    nextDirection(Direction::RIGHT),
    shouldGrow(false),
    moveSpeed(5.0f),
    isMoving(false) {
}

Snake::~Snake() {
    UnloadModel(sphereModel);
    UnloadTexture(snakeTexture);
}

void Snake::Initialize(const Vector3& startPos) {
    // Create initial segments
    segments.clear();
    targetPositions.clear();
    
    // Initialize both segments and target positions with the same values
    segments.push_back(startPos);
    segments.push_back(Vector3{startPos.x - 1.0f, startPos.y, startPos.z});
    segments.push_back(Vector3{startPos.x - 2.0f, startPos.y, startPos.z});
    
    targetPositions = segments; // Copy segments to targetPositions
    
    // Create optimized sphere model for segments
    sphereModel = LoadModelFromMesh(GenMeshSphere(0.5f, 16, 16)); // Higher detail
    
    // Default green texture for snake
    snakeTexture = LoadTexture("resources/snake_texture.png");
    
    Material material = LoadMaterialDefault();
    material.maps[MATERIAL_MAP_DIFFUSE].color = GREEN;
    sphereModel.materials[0] = material;
    
    direction = Direction::RIGHT;
    nextDirection = Direction::RIGHT;
    shouldGrow = false;
    isMoving = false;
}

void Snake::Reset(const Vector3& startPos) {
    direction = Direction::RIGHT;
    nextDirection = Direction::RIGHT;
    shouldGrow = false;
    isMoving = false;
    
    segments.clear();
    targetPositions.clear();
    
    segments.push_back(startPos);
    segments.push_back(Vector3{startPos.x - 1.0f, startPos.y, startPos.z});
    segments.push_back(Vector3{startPos.x - 2.0f, startPos.y, startPos.z});
    
    targetPositions = segments;
}

void Snake::Move() {
    // Don't move if already moving (waiting for interpolation to complete)
    if (isMoving) return;
    
    // Update the current direction
    direction = nextDirection;
    
    // Store the last target position in case we need to grow
    Vector3 lastPosition = targetPositions.back();
    
    // Move all target positions except the head
    for (int i = targetPositions.size() - 1; i > 0; --i) {
        targetPositions[i] = targetPositions[i - 1];
    }
    
    // Move the head target position based on current direction
    switch (direction) {
        case Direction::UP:
            targetPositions[0].z -= 1.0f;
            break;
        case Direction::DOWN:
            targetPositions[0].z += 1.0f;
            break;
        case Direction::LEFT:
            targetPositions[0].x -= 1.0f;
            break;
        case Direction::RIGHT:
            targetPositions[0].x += 1.0f;
            break;
    }
    
    // Grow if needed
    if (shouldGrow) {
        targetPositions.push_back(lastPosition);
        segments.push_back(lastPosition); // Add new segment at the same position
        shouldGrow = false;
    }
    
    // Set the move flag to true to start interpolation
    isMoving = true;
}

void Snake::Update(float deltaTime) {
    // If not moving, do nothing
    if (!isMoving) return;
    
    // Calculate base speed - increase with snake length to maintain smoothness
    float baseSpeed = moveSpeed * (1.0f + (segments.size() - 3) * 0.05f);
    baseSpeed = fmin(baseSpeed, moveSpeed * 3.0f); // Cap the speed increase
    
    // Only check if head has reached target - not waiting for all segments
    if (Vector3Distance(segments[0], targetPositions[0]) < baseSpeed * deltaTime) {
        segments[0] = targetPositions[0]; // Snap head to target
        isMoving = false; // Allow new movement once head reaches target
    } else {
        // Move head towards target
        Vector3 moveDir = Vector3Normalize(Vector3Subtract(targetPositions[0], segments[0]));
        segments[0] = Vector3Add(segments[0], Vector3Scale(moveDir, baseSpeed * deltaTime));
    }
    
    // Always update all other segments to follow, regardless of head position
    for (std::size_t i = 1; i < segments.size(); ++i) {
        if (i >= targetPositions.size()) continue; // Safety check
        
        // Calculate movement speed - slightly faster for trailing segments for catchup
        float segmentSpeed = baseSpeed * (1.0f + 0.1f * i);
        float moveDelta = segmentSpeed * deltaTime;
        
        // Check if we're close enough to snap to target position
        if (Vector3Distance(segments[i], targetPositions[i]) < moveDelta) {
            segments[i] = targetPositions[i]; // Snap to target
        } else {
            // Move towards target position with increased speed for trailing segments
            Vector3 moveDir = Vector3Normalize(Vector3Subtract(targetPositions[i], segments[i]));
            segments[i] = Vector3Add(segments[i], Vector3Scale(moveDir, moveDelta));
        }
    }
}

void Snake::Grow() {
    shouldGrow = true;
}

void Snake::Draw() {
    // Base snake colors - vibrant green palette
    Color headColor = (Color){ 0, 180, 0, 255 };       // Bright green for head
    Color bodyBaseColor = (Color){ 0, 220, 40, 255 };  // Lighter green for body
    
    // Draw each segment with a gradient effect
    for (std::size_t i = 0; i < segments.size(); ++i) {
        Color segmentColor;
        
        if (i == 0) {
            // Head is distinctly colored
            segmentColor = headColor;
        } else {
            // Create gradient effect for body - shift toward darker and more blue with each segment
            float fadeFactor = static_cast<float>(i) / static_cast<float>(segments.size());
            segmentColor.r = static_cast<unsigned char>(bodyBaseColor.r * (1.0f - fadeFactor * 0.5f));
            segmentColor.g = static_cast<unsigned char>(bodyBaseColor.g * (1.0f - fadeFactor * 0.3f));
            segmentColor.b = static_cast<unsigned char>(bodyBaseColor.b + (135 - bodyBaseColor.b) * fadeFactor);
            segmentColor.a = 255;
        }
        
        // Apply material color
        Material material = sphereModel.materials[0];
        material.maps[MATERIAL_MAP_DIFFUSE].color = segmentColor;
        sphereModel.materials[0] = material;
        
        // Draw the segment
        DrawModel(sphereModel, segments[i], 1.0f, WHITE);
        
        // Add highlight to head for better visibility
        if (i == 0) {
            DrawSphere(Vector3Add(segments[i], (Vector3){ 0.2f, 0.2f, 0.0f }), 0.15f, (Color){ 255, 255, 200, 120 });
        }
    }
}

Direction Snake::GetDirection() const {
    return direction;
}

void Snake::SetDirection(Direction dir) {
    // Prevent 180-degree turns (e.g., can't go right when moving left)
    if ((dir == Direction::LEFT && direction == Direction::RIGHT) ||
        (dir == Direction::RIGHT && direction == Direction::LEFT) ||
        (dir == Direction::UP && direction == Direction::DOWN) ||
        (dir == Direction::DOWN && direction == Direction::UP)) {
        return;
    }
    
    // Always allow direction change, even when moving
    nextDirection = dir;
}

const std::vector<Vector3>& Snake::GetSegments() const {
    return segments;
}

float Snake::GetLength() const {
    return static_cast<float>(segments.size());
}
