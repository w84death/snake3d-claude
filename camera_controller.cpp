#include "camera_controller.h"
#include <cmath>
#include "raymath.h"

CameraController::CameraController() :
    snake(nullptr),
    cameraDistance(20.0f),
    minDistance(18.0f),
    maxDistance(30.0f),
    distancePerSegment(0.4f),
    cameraAngle(135.0f) {  // Change to 135 degrees for more natural controls
}

CameraController::~CameraController() {
}

void CameraController::Initialize(Snake* snakePtr) {
    snake = snakePtr;
    
    // Initialize camera for 3D side view with angle set to make controls feel natural
    float height = 12.0f;
    // Position camera at 135 degrees - rear view that aligns with standard controls
    camera.position = Vector3{cameraDistance, height, cameraDistance};  
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};  // Standard up vector
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    targetPosition = camera.position;
}

void CameraController::Update() {
    if (!snake) return;
    
    const auto& segments = snake->GetSegments();
    if (segments.empty()) return;
    
    // Get head position for tracking
    Vector3 head = segments.front();
    
    // Calculate desired distance based on snake length
    float desiredDistance = minDistance + distancePerSegment * (snake->GetLength() - 3);
    desiredDistance = fmin(desiredDistance, maxDistance);
    
    // Smoothly adjust camera distance
    cameraDistance = cameraDistance * 0.95f + desiredDistance * 0.05f;
    
    // Set camera target at snake head
    camera.target = head;
    
    // Calculate the new camera position - keep it at a fixed angle relative to target
    float height = cameraDistance * 0.6f;  // Camera height proportional to distance
    
    // Fixed angle perspective - position camera to align with controls
    targetPosition = Vector3{
        head.x + cameraDistance * cosf(cameraAngle * DEG2RAD),
        height,
        head.z + cameraDistance * sinf(cameraAngle * DEG2RAD)
    };
    
    // Smooth camera movement with interpolation
    camera.position.x = camera.position.x * 0.95f + targetPosition.x * 0.05f;
    camera.position.y = camera.position.y * 0.95f + targetPosition.y * 0.05f;
    camera.position.z = camera.position.z * 0.95f + targetPosition.z * 0.05f;
}

Camera3D CameraController::GetCamera() const {
    return camera;
}
