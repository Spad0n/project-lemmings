#include "raylib.h"
#include "raymath.h"
#include "macro.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#define ARRAY_IMPLEMENTATION
#include "array.h"

typedef enum {
    PLAYER,
    ENEMY,
    PROJECTILE,
} EntityType;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float reload_counter;
    float bullet_decay;
    float dash_counter;
    int hp;
    int id;
    EntityType type;
} Entity;

typedef struct {
    float dt;
    Entity* entities;
} Game;

static int id = 0;

void render_entities(Game *game) {
    for (size_t i = 0; i < array_size(game->entities); i++) {
	Entity entity = game->entities[i];
	switch (entity.type) {
	case PLAYER:
	    DrawRectangle(entity.pos.x, entity.pos.y, 10, 10, RED); break;
	case ENEMY:
	    DrawRectangle(entity.pos.x, entity.pos.y, 10, 10, BLUE); break;
	case PROJECTILE:
	    DrawPixelV(entity.pos, BLUE); break;
	}
    }
}

//void render_entity(Entity entity) {
//    switch (entity.type) {
//    case PLAYER:
//	return DrawRectangleLines(entity.pos.x, entity.pos.y, 10, 10, RED);
//    case ENEMY:
//	return DrawRectangleLines(entity.pos.x, entity.pos.y, 10, 10, BLUE);
//    case PROJECTILE:
//	return DrawPixelV(entity.pos, BLUE);
//    }
//}

Entity create_entity(EntityType type, float posX, float posY, int hp) {
    id++;
    Entity result = {0};
    result.type = type;
    result.pos = (Vector2){ posX, posY };
    result.vel = (Vector2){0, 0};
    result.hp = hp;
    result.id = id;
    result.reload_counter = 500;
    return result;
}

Entity create_projectile(float posX, float posY, Vector2 vel, float bullet_decay) {
    id++;
    Entity result = {0};
    result.type = PROJECTILE;
    result.pos = (Vector2){ posX, posY };
    result.vel = vel;
    result.hp = 1;
    result.id = id;
    result.bullet_decay = bullet_decay;

    return result;
    //return (Entity){
    //	.type = PROJECTILE,
    //	.pos = (Vector2){ posX, posY },
    //	.vel = vel,
    //	.hp = 1,
    //	.id = id,
    //	.bullet_decay = bullet_decay,
    //};
}

Entity* find_entity(EntityType type, Game *game) {
    for (size_t i = 0; i < array_size(game->entities); i++) {
	if (game->entities[i].type == type) {
	    return &game->entities[i];
	}
    }
    return NULL;
}

void update_entities(Game *game) {
    for (size_t i = 0; i < array_size(game->entities); i++) {
	switch (game->entities[i].type) {
	case PLAYER: {
		Vector2 dir = {0, 0};
		if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
		    dir.y -= 1;
		if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
		    dir.y += 1;
		if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
		    dir.x -= 1;
		if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
		    dir.x += 1;
		game->entities[i].pos = Vector2Add(game->entities[i].pos, Vector2Scale(dir, (200.0 * game->dt)));

		// Keep in the map
		game->entities[i].pos = Vector2Clamp(game->entities[i].pos, (Vector2){0, 0},
						     (Vector2){640.0f - 10.0f, 480.0f - 10.0f});
		break;
	    }
	case ENEMY: {
		// Towards player
		Entity *player = find_entity(PLAYER, game);
		if (player == NULL)
		    return;
		Vector2 dir = Vector2Subtract(player->pos, game->entities[i].pos);
		game->entities[i].pos = Vector2Add(game->entities[i].pos, Vector2Scale(dir, game->dt));
		
		for (size_t j = 0; j < array_size(game->entities); j++) {
		    if (game->entities[j].type == ENEMY &&
			game->entities[i].id != game->entities[j].id) {
			Vector2 dir = Vector2Subtract(game->entities[i].pos, game->entities[j].pos);
			float dis = Vector2Length(dir);
			if (dis > 0) {
			    game->entities[i].pos = Vector2Add(
							       game->entities[i].pos,
							       Vector2Scale(dir, (1.0 / (dis * dis)) * 100.0 * game->dt));
			}
		    }
		}

		if (game->entities[i].reload_counter <= 0) {
		    game->entities[i].reload_counter = 1000;
		    array_push(game->entities,
			       create_projectile(game->entities[i].pos.x, game->entities[i].pos.y,
						 Vector2Scale(dir, 3.0), 750));
		} else {
		    game->entities[i].reload_counter -= 900 * game->dt;
		}
		break;
	    }
	case PROJECTILE: {
		game->entities[i].pos =
		    Vector2Add(game->entities[i].pos, Vector2Scale(game->entities[i].vel, game->dt));
		game->entities[i].bullet_decay -= 500.0f * game->dt;
		if (game->entities[i].bullet_decay < 0) {
		    game->entities[i].hp = 0;
		} else {
		    Entity *player = find_entity(PLAYER, game);
		    if (player == NULL)
			return;
		    Rectangle rect = {.x = player->pos.x,
				      .y = player->pos.y,
				      .width = 10,
				      .height = 10};
		    if (CheckCollisionPointRec(game->entities[i].pos, rect)) {
			player->hp -= 1;
			printf("HIT (HP: %d)\n", player->hp);
			game->entities[i].hp = 0;
		    }
		}
		break;
	    }
	}
    }
}

int main(void) {
    const int screenWidth = 640;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "chase in space");

    Game game = {
	.dt = 0.0f,
	.entities = array_create_init(1, sizeof(Entity)),
    };

    array_push(game.entities, create_entity(PLAYER, 50.0f, 400.0f, 10));
    array_push(game.entities, create_entity(ENEMY, 50.0f, 50.0f, 1));
    array_push(game.entities, create_entity(ENEMY, 100.0f, 100.0f, 1));
    array_push(game.entities, create_entity(ENEMY, 200.0f, 200.0f, 1));

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

	game.dt = GetFrameTime();

	update_entities(&game);

	for (size_t i = 0; i < array_size(game.entities); i++) {
	    if (game.entities[i].hp <= 0) {
		array_pop_at(game.entities, i);
	    }
	}

	Drawing {
	    ClearBackground(BLACK);

	    //for (size_t i = 0; i < array_size(game.entities); i++) {
	    //	render_entity(game.entities[i]);
	    //}
	    render_entities(&game);
	}
    }

    array_free(game.entities);
    CloseWindow();

    return 0;
}
