#include "physic.h"
#include "plug.h"
#include <stdio.h>

Physic physic_init () {
    Physic physic = {0};
    physic.physic_vector = (Vector2){0.0f, 0.0f};
    physic.physic_collision = false;
    return physic;
}

Physic physic_move_left (float deltaTime) {
    Physic physic = {0};
    physic.physic_vector = (Vector2){-50*deltaTime, 0*deltaTime};
    physic.physic_collision = false;
    return physic;
}

Physic physic_move_right (float deltaTime) {
    Physic physic = {0};
    physic.physic_vector = (Vector2){50*deltaTime, 0*deltaTime};
    physic.physic_collision = false;
    return physic;
}

Physic physic_fall (float deltaTime, Player *player) {
    Physic physic = {0};
    physic.physic_vector = Vector2Add(player->player_physic.physic_vector, (Vector2){0*deltaTime, 5*deltaTime});
    physic.physic_collision = false;
    return physic;
}

Physic physic_climb (float deltaTime) {
    Physic physic = {0};
    physic.physic_vector = (Vector2){0*deltaTime, -10*deltaTime};
    physic.physic_collision = false;
    return physic;
}

Physic physic_jump (float deltaTime, Player *player) {
    Physic physic = {0};
    physic.physic_vector = Vector2Add(player->player_physic.physic_vector, (Vector2){0*deltaTime, -5*deltaTime});
    physic.physic_collision = false;
    return physic;
}

Physic physic_collision (Player *player) {
    Physic physic = {0};
    physic.physic_vector = player->player_physic.physic_vector;
    physic.physic_collision = true;
    return physic;
}

void player_init (Player *player, float x, float y) {
    player->player_position = (Vector2){x, y};
    player->player_state = STATE_STATIC;
    player->player_physic = physic_init();
}

void player_physic_update (Player *player) {
    float deltaTime = GetFrameTime();
    switch (player->player_state) {
        case STATE_STATIC: player->player_physic = physic_init(); break;
        case STATE_MOVE_LEFT: player->player_physic = physic_move_left(deltaTime); break;
        case STATE_MOVE_RIGHT: player->player_physic = physic_move_right(deltaTime); break;
        case STATE_FALL: player->player_physic = physic_fall(deltaTime, player); break;
        case STATE_CLIMB: player->player_physic = physic_climb(deltaTime); break;
        case STATE_JUMP: player->player_physic = physic_jump(deltaTime, player); break;
        case STATE_COLLISION: player->player_physic = physic_collision(player); break;
    }
}

//void player_state_update (Player *player, int tilemap[13][22]) {
void player_state_update (Plug *plug) {
    if (IsKeyPressed(KEY_I) && plug->player.player_state == STATE_STATIC) {
        plug->player.player_state = STATE_MOVE_LEFT;
    }
    else if (IsKeyPressed(KEY_O) && plug->player.player_state == STATE_STATIC) {
        plug->player.player_state = STATE_MOVE_RIGHT;
    }
    //else if (tilemap[plug->player.player_position.y + 1][plug->player.player_position.x] == BLOCK_EMPTY) {
    //    plug->player.player_state = STATE_FALL;
    //}
    else if (IsKeyPressed(KEY_K)) {
	plug->player.player_state = STATE_FALL;
    }
    else if (IsKeyPressed(KEY_L)) {
        plug->player.player_state = STATE_JUMP;
    }
    else if (IsKeyPressed(KEY_SPACE)) {
        plug->player.player_state = STATE_STATIC;
    }
}

void player_update (Player *player) {
    player->player_position = Vector2Add(player->player_position, player->player_physic.physic_vector);
}
