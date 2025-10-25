#include "raylib.h"
#include "raymath.h"
#include "player.hpp"
#include "ammobox.hpp"
#include <cstdio>
#include <cmath>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    bool isFullScreen = false;
    bool isGameOver = false;
    char ammoText[32];

    InitWindow(screenWidth, screenHeight, "Shotgun : Chargin' It");
    SetTargetFPS(60);
    
    // Initialize music
    InitAudioDevice ();

    // Initialize player
    Player player = { 0 };
    player.position = (Vector2){ 400, 280 };
    player.speed = (Vector2){ 0, 0 };
    player.onGround = false;
    player.Ammo = 90;
    
    // Load Player Texture and Audio
    Texture2D playerTexture = LoadTexture("resources/Hiker1.png");
    Sound shotgunBlast = LoadSound("resources/explosion.wav");
    Sound reload = LoadSound("resources/reloaddone.wav");
    Music ambience = LoadMusicStream("resources/environment.mp3");

    // Environment setup
    EnvItem envItems[] = {
        {{ 0, 0, 1000, 400 }, 0, 0, 0, WHITE },   // background
        {{ 0, 400, 1000, 200 }, 1, 1, 0, GRAY },  // ground
        {{ 300, 200, 400, 10 }, 1, 1, 0, GRAY },  // solid
        {{ 250, 300, 100, 10 }, 1, 0, 1, GREEN }, // bouncy top platform
        {{ 650, 250, 100, 10 }, 1, 1, 1, RED },   // fully solid and bouncy
    };
    int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);


    int ammoBoxCount = 2;
    ammoBox **ammoBoxes = (ammoBox **)malloc(sizeof(ammoBox *) * ammoBoxCount);
    ammoBoxes[0] = CreateAmmoBox(450, 350, 30);
    ammoBoxes[1] = CreateAmmoBox(600, 180, 30);


    // Camera setup
    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Rectangle mouseBox = { GetScreenWidth()/2.0f - 30, GetScreenHeight()/2.0f - 30, 60, 60 };
    Color mouseColor = ORANGE;
    int screenUpperLimit = 0;
    
    PlayMusicStream(ambience);
        
    // Game loop
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        UpdateMusicStream(ambience);

        if (IsKeyPressed(KEY_F))
        {
            isFullScreen = !isFullScreen;
            ToggleFullscreen();
        }

        UpdatePlayer(&player, envItems, envItemsLength, deltaTime, camera);

        // Update ammo boxes here (AFTER they exist)
        UpdateAmmoBoxes(ammoBoxes, &ammoBoxCount, &player);

        if (player.Ammo <= 0 && player.onGround && player.speed.x < 5 && player.speed.y == 0)
            isGameOver = true;

        camera.zoom += ((float)GetMouseWheelMove() * 0.05f);
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            PlaySound(shotgunBlast);
            if (player.reloadTimer < 0.0f)
            {
                PlaySound(reload);
            } 
        }
        
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            player.position = (Vector2){ 400, 280 };
            player.speed = (Vector2){ 0, 0 };
            player.Ammo = 90;
            isGameOver = false;
        }

        UpdateCameraCenterInsideMap(&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);

        mouseBox.x = GetMouseX() - mouseBox.width / 2;
        mouseBox.y = GetMouseY() - mouseBox.height / 2;
        if (mouseBox.x < 0) mouseBox.x = 0;
        if (mouseBox.y < screenUpperLimit) mouseBox.y = (float)screenUpperLimit;

        mouseColor = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? MAROON : ORANGE;

        // Drawing
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        BeginMode2D(camera);

        // Draw environment
        for (int i = 0; i < envItemsLength; i++) {
            Color c = envItems[i].bouncy ? GREEN : (envItems[i].solidAllSides ? DARKGRAY : envItems[i].color);
            DrawRectangleRec(envItems[i].rect, c);
        }

        // Draw player
        Rectangle playerRect = { player.position.x - 20, player.position.y - 64, 40.0f, 60.0f };
        DrawRectangleRec(playerRect, BLUE);
        DrawCircleV(player.position, 5.0f, GOLD);
        DrawTextureV(playerTexture, (Vector2){playerRect.x, playerRect.y}, WHITE);


        // Gun
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 dir = Vector2Subtract(mouseWorld, player.position);
        float angle = atan2f(dir.y, dir.x) * RAD2DEG;

        Vector2 rectSize = { 60, 10 };
        Vector2 rectOrigin = { 0, rectSize.y / 2 };
        DrawRectanglePro(
            (Rectangle){ player.position.x, player.position.y - rectSize.y - 5 , rectSize.x, rectSize.y },
            rectOrigin,
            angle,
            RED
        );

        // Muzzle position
        Vector2 rectangleFace = Vector2Rotate((Vector2){ 1.0f, 0.0f }, angle * DEG2RAD);
        Vector2 muzzlePos = Vector2Add(player.position, Vector2Scale(rectangleFace, rectSize.x));
        muzzlePos.y = muzzlePos.y - 15; 
        DrawCircleV(muzzlePos, 5.0f, GREEN);

        // ✅ Draw ammo boxes
        DrawAmmoBoxes(ammoBoxes, ammoBoxCount);

        EndMode2D();

        // UI
        DrawRectangleRec(mouseBox, mouseColor);
        DrawText("Press F to toggle Fullscreen", 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Fullscreen = %s", isFullScreen ? "True" : "False"), 10, 30, 20, DARKGRAY);

        snprintf(ammoText, sizeof(ammoText), "Ammo: %d", player.Ammo);
        DrawText(ammoText, 500, 360, 60, GREEN);

        DrawText("Shotgun Charge", 20, 70, 20, BLACK);
        float chargePercent = player.heldTimerMult;
        if (chargePercent > 1.0f) chargePercent = 1.0f;

        int barX = 20, barY = 100, barWidth = 200, barHeight = 20;
        DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
        DrawRectangle(barX, barY, (int)(barWidth * chargePercent), barHeight, ORANGE);

        char chargeText[32];
        snprintf(chargeText, sizeof(chargeText), "%.2fx", player.heldTimerMult);
        DrawText(chargeText, barX + barWidth + 15, barY - 2, 20, RED);

        if (isGameOver) {
            DrawText("STRANDED", 150, 150, 100, RED);
            DrawText("Press 'R' To Continue", 250 , 250, 35, GRAY);
        }

        EndDrawing();
    }

    // Cleanup
    for (int i = 0; i < ammoBoxCount; i++) {
        free(ammoBoxes[i]);
    }
    free(ammoBoxes);
    
    CloseAudioDevice ();
    CloseWindow();
    return 0;
}
