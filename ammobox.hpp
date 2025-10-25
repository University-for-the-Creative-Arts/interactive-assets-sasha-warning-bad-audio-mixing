#ifndef AMMOBOX_HPP
#define AMMOBOX_HPP

#include <raylib.h>
#include "player.hpp"

typedef struct ammoBox {
    Vector2 position;
    Vector2 size;
    int ammoAmount;
    bool isCollected;
} ammoBox; 

ammoBox *CreateAmmoBox(float x, float y, int ammoAmount);

void UpdateAmmoBoxes(ammoBox **boxes, int *boxCount, Player *player);
void DrawAmmoBoxes(ammoBox **boxes, int boxCount);

#endif