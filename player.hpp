#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <raylib.h>

#define G 3000               // Gravity constant
#define MOUSE_FORCE 67250    // Strength of force away from mouse
#define FRICTION 0.95f       // Friction factor (only applied on ground X)
#define BOUNCE_DAMPING 0.7f  // How much speed is kept after bouncing

typedef struct Player {
    Vector2 position;
    Vector2 speed;
    bool onGround;
    signed int Ammo;
    float reloadTimer; 
    float heldTimer; 
    float heldTimerMult;    // Multiplier for force
    int heldTimerAmmoMult;  // Multiplier for Ammo Use
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;           // 1 = blocks on top
    int solidAllSides;      // 1 = solid block (blocks on all sides)
    int bouncy;             // 1 = reflects player speed when hit
    Color color;
} EnvItem;

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta, Camera2D camera);
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);

#endif
