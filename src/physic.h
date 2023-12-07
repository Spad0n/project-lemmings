#ifndef PHYSIC_H_
#define PHYSIC_H_

#include <stdbool.h>
#include "raylib.h"
#include "raymath.h"

typedef struct Plug Plug;

typedef struct {
    Vector2 physic_vector;
    bool physic_collision;
} Physic;

typedef enum {
    STATE_STATIC,
    STATE_MOVE_LEFT,
    STATE_MOVE_RIGHT,
    STATE_FALL,
    STATE_CLIMB,
    STATE_JUMP,
    STATE_COLLISION
} State;

typedef struct {
    Vector2 player_position;
    State player_state; 
    Physic player_physic;
} Player;

Physic physic_init ();
Physic physic_move_left (float deltaTime);
Physic physic_move_right (float deltaTime);
Physic physic_fall (float deltaTime, Player *player);
Physic physic_climb (float deltaTime);
Physic physic_jump (float deltaTime, Player *player);
Physic physic_collision (Player *player);

void player_init (Player *player, float x, float y);
void player_physic_update (Player *player);
void player_state_update (Plug *plug);
void player_update (Player *player);

#endif //PHYSIC_H_
