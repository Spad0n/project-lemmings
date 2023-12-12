#ifndef ENTITY_H_
#define ENTITY_H_

#include <stdbool.h>
#include "raylib.h"

#define PLAYER_SPEED 90

typedef struct Plug Plug;

typedef enum {
    PLAYER,
    ENEMY,
    PROJECTILE,
} EntityType;

typedef enum State {
    STATIC,
    MOVE_LEFT,
    MOVE_RIGHT,
} State;

typedef struct {
    Rectangle rect;
    Vector2 velocity;
    EntityType type;
    State state;
    bool on_ground;
} Entity;

void entity_update(Plug *plug);

Entity entity_init(int x, int y);

#endif // ENTITY_H_
