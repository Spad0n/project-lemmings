#include "physic.h"

Physic physic_init () {
    Physic physic = {0};
    physic.physic_vector = (Vector2){0.0f, 0.0f};
    physic.physic_collision = false;
    return physic;
}

void physic_left (Physic *physic) {
    physic->physic_vector = (Vector2){-4.0f, 0.0f};
    physic->physic_collision = false;
}

void physic_right (Physic *physic) {
    physic->physic_vector = (Vector2){4.0f, 0.0f};
    physic->physic_collision = false;
}

void physic_climb (Physic *physic) {
    physic->physic_vector = (Vector2){0.0f, 4.0f};
    physic->physic_collision = false;
}

void physic_fall (Physic *physic) {
    physic->physic_vector = (Vector2){0.0f, -4.0f};
    physic->physic_collision = false;
}

void physic_collision (Physic *physic) {
    physic->physic_vector = (Vector2){0.0f, 0.0f};
    physic->physic_collision = true;
}

void player_init (Player *player, float x, float y, bool facing) {
    player->player_position = (Vector2){x, y};
    player->player_facing = facing;
    player->player_state = 0;
    player->player_physic = physic_init();
}

void player_update (Player *player) {
    if (player->player_state) {
        switch(player->player_facing) {
        case 0: return; 
        case 1: return; 
        }
    }
}


