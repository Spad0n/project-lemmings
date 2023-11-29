#ifndef PHYSIC_H_
#define PHYSIC_H_

#include <stdbool.h>
#include "raylib.h"
#include "raymath.h"

typedef struct {
    Vector2 physic_vector;
    bool physic_collision;
} Physic;

typedef enum {
    STATE_STATIC,
    STATE_MOVE,
    STATE_FALL,
    STATE_CLIMB,
    STATE_JUMP
} State;

typedef struct {
    Vector2 player_position;
    bool player_facing; //0 : left ; 1 : right
    State player_state; 
    Physic player_physic;
} Player;

Physic physic_init ();

void physic_left (Physic *physic);

void physic_right (Physic *physic);

void physic_climb (Physic *physic);

void physic_fall (Physic *physic);

void physic_collision (Physic *physic);

void player_init (Player *player, float x, float y, bool facing);

void player_update (Player *player);

#endif //PHYSIC_H_
