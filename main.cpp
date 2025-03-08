#include "raylib.h"
#include "game.h"
#include <iostream>

int main() {
    // Initialize window and game
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "3D Snake Game");
    SetTargetFPS(60);
    
    // Set background color to a natural sky blue
    SetExitKey(KEY_NULL); // Disable automatic exit with ESC
    
    Game game;
    game.Initialize();
    
    // Main game loop
    while (!WindowShouldClose()) {
        game.Update();
        game.Render();
    }
    
    // Cleanup
    game.Cleanup();
    CloseWindow();
    
    return 0;
}
