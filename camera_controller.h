#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "raylib.h"
#include "snake.h"

class CameraController {
public:
    CameraController();
    ~CameraController();
    
    void Initialize(Snake* snakePtr);
    void Update();
    Camera3D GetCamera() const;
    
private:
    Camera3D camera;
    Snake* snake;
    Vector3 targetPosition;
    float cameraDistance;
    float minDistance;
    float maxDistance;
    float distancePerSegment;
    float cameraAngle;  // Camera angle in degrees
};

#endif // CAMERA_CONTROLLER_H
