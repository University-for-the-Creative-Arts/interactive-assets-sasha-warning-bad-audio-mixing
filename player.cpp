#include <raylib.h>
#include <raymath.h>
#include "player.hpp"

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta, Camera2D camera)
{
    // Apply gravity
    player->speed.y += G * delta;

    // Reload timer
    if (player->reloadTimer > 0.0f)
    {
        player->reloadTimer -= delta;
        if (player->reloadTimer < 0.0f) player->reloadTimer = 0.0f;
    }

    // Hold left click to charge
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        player->heldTimer += delta;
        if (player->heldTimer > 1.0f) player->heldTimer = 1.0f;
        player->heldTimerMult = player->heldTimer / 1.0f;
    }

    // Release left click to shoot
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        // Determine ammo multiplier
        if (player->heldTimerMult <= 0.33f)
            player->heldTimerAmmoMult = 1;
        else if (player->heldTimerMult < 1.0f)
            player->heldTimerAmmoMult = 2;
        else
            player->heldTimerAmmoMult = 3;

        int ammoToUse = player->heldTimerAmmoMult;
        if (ammoToUse > player->Ammo) ammoToUse = player->Ammo;

        // Consume ammo if reload finished
        if (ammoToUse > 0 && player->reloadTimer <= 0.0f)
        {
            player->Ammo -= ammoToUse;

            Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
            Vector2 direction = Vector2Subtract(player->position, mouseWorld); // push away from cursor

            if (Vector2Length(direction) > 0.1f)
            {
                direction = Vector2Normalize(direction);
                player->speed.x += direction.x * MOUSE_FORCE * delta * player->heldTimerMult;
                player->speed.y += direction.y * MOUSE_FORCE * delta * player->heldTimerMult;
                player->reloadTimer = 0.2f;
                player->heldTimer = 0.0f;
                player->heldTimerMult = 0.0f;
            }
        }
    }

    // Predict next position
    Vector2 nextPos = Vector2Add(player->position, Vector2Scale(player->speed, delta));

    // Collision 
    player->onGround = false;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        Rectangle rect = ei->rect;
        Rectangle playerRect = { nextPos.x - 20, nextPos.y - 40, 40, 40 };

        if (ei->solidAllSides)
        {
            // Full block collision (all sides)
            if (CheckCollisionRecs(playerRect, rect))
            {
                float overlapLeft   = (playerRect.x + playerRect.width) - rect.x;
                float overlapRight  = (rect.x + rect.width) - playerRect.x;
                float overlapTop    = (playerRect.y + playerRect.height) - rect.y;
                float overlapBottom = (rect.y + rect.height) - playerRect.y;

                float minOverlapX = fminf(overlapLeft, overlapRight);
                float minOverlapY = fminf(overlapTop, overlapBottom);

                if (minOverlapX < minOverlapY)
                {
                    // Resolve X
                    if (overlapLeft < overlapRight)
                        nextPos.x = rect.x - 20;
                    else
                        nextPos.x = rect.x + rect.width + 20;

                    if (ei->bouncy)
                        player->speed.x *= -BOUNCE_DAMPING;
                    else
                        player->speed.x = 0;
                }
                else
                {
                    // Resolve Y
                    if (overlapTop < overlapBottom)
                    {
                        nextPos.y = rect.y - 0.1f;
                        if (ei->bouncy)
                            player->speed.y = -fabsf(player->speed.y) * BOUNCE_DAMPING;
                        else
                        {
                            player->speed.y = 0;
                            player->onGround = true;
                        }
                    }
                    else
                    {
                        nextPos.y = rect.y + rect.height + 40;
                        if (ei->bouncy)
                            player->speed.y = fabsf(player->speed.y) * BOUNCE_DAMPING;
                        else
                            player->speed.y = 0;
                    }
                }
            }
        }
        else if (ei->blocking)
        {
            // Top-only collision
            if (ei->rect.x <= nextPos.x &&
                ei->rect.x + ei->rect.width >= nextPos.x &&
                ei->rect.y >= player->position.y &&
                ei->rect.y <= nextPos.y)
            {
                player->onGround = true;
                if (ei->bouncy)
                    player->speed.y = -fabsf(player->speed.y) * BOUNCE_DAMPING;
                else
                    player->speed.y = 0;

                nextPos.y = ei->rect.y;
            }
        }
    }

    if (player->onGround)
        player->speed.x *= FRICTION;

    player->position = nextPos;
}

void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->target = player->position;
    camera->offset = (Vector2){ width / 2.0f, height / 2.0f };

    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);

    if (max.x < width) camera->offset.x = width - (max.x - width / 2);
    if (max.y < height) camera->offset.y = height - (max.y - height / 2);
    if (min.x > 0) camera->offset.x = width / 2 - min.x;
    if (min.y > 0) camera->offset.y = height / 2 - min.y;
}
