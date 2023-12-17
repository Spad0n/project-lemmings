/* -*- compile-command: "make -C .. libplug" -*- */
#include <stdio.h>
#include "entity.h"
#include "plug.h"
#include "array.h"

#define G 32.0f
#define PLAYER_JUMP_SPD 400.0f
#define PLAYER_RECT (Rectangle){0, 0, 48 - (12 * 2), 48 - 12}

//#define SPIKE_RECT (Rectangle){0, 0, 48 - (12 * 2), 48 - 12}

void entity_update(Plug *plug) {
    for (size_t i = 0; i < array_size(plug->players); i++) {
	Entity *player = &(plug->players[i]);
	float dt = GetFrameTime();

	player->velocity.y += G * dt;

	//if (player->rect.x > SCREEN_WIDTH || player->rect.x < 0 || player->rect.y < 0 || player->rect.y > SCREEN_HEIGHT) {
	if (player->rect.x > SCREEN_WIDTH || player->rect.x < 0 || player->rect.y > SCREEN_HEIGHT) {
	    array_pop_at(plug->players, i);
	}

	//if (IsKeyPressed(KEY_RIGHT)) {
	//    player->state = MOVE_RIGHT;
	//}
	//if (IsKeyPressed(KEY_LEFT)) {
	//    player->state = MOVE_LEFT;
	//}

	//if (IsKeyUp(KEY_LEFT) && IsKeyUp(KEY_RIGHT)) {
	//	plug->player.state = STATIC;
	//}

	switch (player->state) {
	case MOVE_RIGHT:
	    player->rect.x += PLAYER_SPEED * dt;
	    break;
	case MOVE_LEFT:
	    player->rect.x -= PLAYER_SPEED * dt;
	    break;
	default: break;
	}

	player->rect.y += player->velocity.y;

	player->on_ground = false;

	bool auto_jump = false;

	for (size_t j = 0; j < array_size(plug->players); j++) {
	    if (CheckCollisionRecs(player->rect, plug->players[j].rect) && j != i) {
		if (player->rect.x < plug->players[j].rect.x) {
		    player->state = MOVE_LEFT;
		} else {
		    player->state = MOVE_RIGHT;
		}
	    }
	}

	for (size_t y = 0; y < TILESY; y++) {
	    for (size_t x = 0; x < TILESX; x++) {
		if (0 < plug->tilemap[y][x] && plug->tilemap[y][x] <= BLOCK_BRICK) {
		    Rectangle block = {
			.x = MAP_TILE_SIZE * x,
			.y = MAP_TILE_SIZE * y,
			.width = MAP_TILE_SIZE,
			.height = MAP_TILE_SIZE,
		    };

		    if (CheckCollisionRecs(player->rect, block) && plug->tilemap[y][x] == BLOCK_DOOR) {
			plug->score += 1;
			array_pop_at(plug->players, i);
			printf("player %ld get the exit !\n", i);
		    }

		    if (CheckCollisionRecs(player->rect, block) && plug->tilemap[y][x] == BLOCK_COIN) {
			plug->tilemap[y][x] = BLOCK_EMPTY;
			plug->coins++;
			printf("player %ld get the coin !\n", i);
		    }

		    if (CheckCollisionRecs(player->rect, block) && plug->tilemap[y][x] == BLOCK_S_BRICK) {
			plug->tilemap[y][x] = BLOCK_EMPTY;
			plug->bricks+= 1;
			printf("player %ld get the small brick !\n", i);
		    }

		    if (CheckCollisionRecs(player->rect, block) && plug->tilemap[y][x] == BLOCK_B_BRICK) {
			plug->tilemap[y][x] = BLOCK_EMPTY;
			plug->bricks += 2;
			printf("player %ld get the small brick !\n", i);
		    }

		    // check if the player collide with a spike
		    if (CheckCollisionRecs(player->rect, block) && plug->tilemap[y][x] == BLOCK_SPIKE) {
			array_pop_at(plug->players, i);
			printf("player %ld get killed by the spike !\n", i);
		    }

		    if (CheckCollisionRecs(player->rect, block) && (plug->tilemap[y][x] < 32 || plug->tilemap[y][x] == BLOCK_BRICK)) {

			float overlapX = 0;
			float overlapY = 0;

			Tile2D player_center = {
			    .x = (player->rect.x * 2 + player->rect.width) / 2,
			    .y = (player->rect.y * 2 + player->rect.height) / 2,
			};

			if (player->state == MOVE_RIGHT) {
			    player_center.x -= player->rect.width/2;
			} else if (player->state == MOVE_LEFT) {
			    player_center.x += player->rect.width/2;
			}

			player_center.x /= MAP_TILE_SIZE;
			player_center.y /= MAP_TILE_SIZE;

			if (player_center.x + 1 < TILESX && player_center.x - 1 > 0) {
			    if ((plug->tilemap[player_center.y][player_center.x + 1] == 25 || plug->tilemap[player_center.y][player_center.x + 1] == 17 || plug->tilemap[player_center.y][player_center.x + 1] == 9 || plug->tilemap[player_center.y][player_center.x + 1] == BLOCK_BRICK) && plug->tilemap[player_center.y - 1][player_center.x + 1] != BLOCK_BRICK && player->state == MOVE_RIGHT) {
				auto_jump = true;
			    } else if ((plug->tilemap[player_center.y][player_center.x - 1] == 19 || plug->tilemap[player_center.y][player_center.x - 1] == 17 || plug->tilemap[player_center.y][player_center.x - 1] == 3 || plug->tilemap[player_center.y][player_center.x - 1] == BLOCK_BRICK) && plug->tilemap[player_center.y - 1][player_center.x - 1] != BLOCK_BRICK && player->state == MOVE_LEFT) {
				auto_jump = true;
			    } else if (((plug->tilemap[player_center.y][player_center.x + 1] > 0 && plug->tilemap[player_center.y][player_center.x + 1] < 32) || plug->tilemap[player_center.y][player_center.x + 1] == BLOCK_BRICK) && player->state == MOVE_RIGHT) {
				player->state = MOVE_LEFT;
			    } else if (((plug->tilemap[player_center.y][player_center.x - 1] > 0 && plug->tilemap[player_center.y][player_center.x - 1] < 32) || plug->tilemap[player_center.y][player_center.x - 1] == BLOCK_BRICK) && player->state == MOVE_LEFT) {
				player->state = MOVE_RIGHT;
			    }
			}

			//if (plug->tilemap[player_center.y][player_center.x + 1] == 29 && player->state == MOVE_RIGHT) {
			//    player->state = MOVE_LEFT;
			//} else if (plug->tilemap[player_center.y][player_center.x - 1] == 23 && player->state == MOVE_LEFT) {
			//    player->state = MOVE_RIGHT;
			//}

			// check overlap
			if (player->rect.x < block.x) {
			    overlapX = block.x - (player->rect.x + player->rect.width);
			} else {
			    overlapX = (block.x + block.width) - player->rect.x;
			}

			if (player->rect.y < block.y) {
			    overlapY = block.y - (player->rect.y + player->rect.height);
			} else {
			    overlapY = (block.y + block.height) - player->rect.y;
			}

			if (fabs(overlapX) < fabs(overlapY)) {
			    player->rect.x += overlapX;
			    player->velocity.x = 0;
			} else {
			    player->rect.y += overlapY;
			    player->velocity.y = 0;
			    if (overlapY < 0) player->on_ground = true;
			}
		    }
		}
	    }
	}

	//if ((IsKeyDown(KEY_SPACE) && player->on_ground) || (player->on_ground && auto_jump)) {
	//    player->velocity.y -= PLAYER_JUMP_SPD * dt;
	//    //plug->player.delai = 35;
	//}

	if (player->on_ground && auto_jump) {
	    player->velocity.y -= PLAYER_JUMP_SPD * dt;
	}
    }
}

Entity entity_init(int x, int y) {
//Entity entity_init(Rectangle rec) {
    Entity entity = {
	.rect = PLAYER_RECT,
	.velocity = (Vector2){0, 0},
	.type = PLAYER,
	.state = STATIC,
	.on_ground = false,
    };
    entity.rect.x += x;
    entity.rect.y += y;
    return entity;
}
