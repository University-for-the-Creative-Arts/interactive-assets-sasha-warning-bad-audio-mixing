#include <raylib.h>
#include <stdlib.h>
#include "ammobox.hpp"
#include "player.hpp"

ammoBox *CreateAmmoBox(float x, float y, int ammoAmount) {
    ammoBox *box = (ammoBox *)malloc(sizeof(ammoBox));
    if (box) {
        box->position = (Vector2){ x, y };
        box->size = (Vector2){ 40, 40 }; // default box size
        box->ammoAmount = ammoAmount;
        box->isCollected = false;
    }
    return box;
}

void UpdateAmmoBoxes(ammoBox **boxes, int *boxCount, Player *player) {
    for (int i = 0; i < *boxCount; i++) {
        ammoBox *box = boxes[i];
        if (!box->isCollected) {
            Rectangle playerRect = { player->position.x - 20, player->position.y - 40, 40, 40 };
            Rectangle boxRect = { box->position.x, box->position.y, box->size.x, box->size.y };

            if (CheckCollisionRecs(playerRect, boxRect)) {
                player->Ammo += box->ammoAmount;
                box->isCollected = true;
            }
        }
    }

    // Compact array & free collected boxes
    int newCount = 0;
    for (int i = 0; i < *boxCount; i++) {
        if (boxes[i]->isCollected) {
            free(boxes[i]);
            boxes[i] = NULL;
        } else {
            boxes[newCount++] = boxes[i];
        }
    }

    *boxCount = newCount;
}

void DrawAmmoBoxes(ammoBox **boxes, int boxCount) {
    for (int i = 0; i < boxCount; i++) {
        ammoBox *box = boxes[i];
        if (!box->isCollected) {
            DrawRectangleV(box->position, box->size, YELLOW);
            DrawRectangleLines((int)box->position.x,(int)box->position.y,(int)box->size.x,(int)box->size.y,BLACK);
        }
    }
}
