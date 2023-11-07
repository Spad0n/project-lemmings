#include "raylib.h"
#include "raymath.h"
#include "macro.h"
#include <math.h>
#include <stddef.h>
#include <stdbool.h>

#define ARRAY_IMPLEMENTATION
#include "array.h"

#define G 400
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_HOR_SPD 200.0f

typedef struct {
    Vector2 position;
    float speed;
    bool canJump;
} Player;

typedef struct {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float dt) {
    if (IsKeyDown(KEY_LEFT) && player->position.x - 20 > envItems[1].rect.x) {
	player->position.x -= PLAYER_HOR_SPD * dt;
    }
    if (IsKeyDown(KEY_RIGHT) && player->position.x + 20 < envItems[1].rect.width) {
	player->position.x += PLAYER_HOR_SPD * dt;
    }
    if (IsKeyDown(KEY_SPACE) && player->canJump) {
	player->speed = -PLAYER_JUMP_SPD;
	player->canJump = false;
    }

    int hitObstacle = false;
    for (size_t i = 0; i < envItemsLength; i++) {
	EnvItem *ei = envItems + i;
	Vector2 *p = &(player->position);
	if (ei->blocking && ei->rect.x <= p->x && ei->rect.x + ei->rect.width >= p->x && ei->rect.y >= p->y && ei->rect.y <= p->y + player->speed * dt) {
	    hitObstacle = true;
	    player->speed = 0.0f;
	    p->y = ei->rect.y;
	}
    }

    if (!hitObstacle) {
	player->position.y += player->speed * dt;
	player->speed += G * dt;
	player->canJump = false;
    } else {
	player->canJump = true;
    }
}

void UpdateCameraFreeInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float dt, int width, int height) {
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    if (IsKeyDown(KEY_A)) camera->target.x -= PLAYER_HOR_SPD * dt * 2;
    if (IsKeyDown(KEY_D)) camera->target.x += PLAYER_HOR_SPD * dt * 2;
    if (IsKeyDown(KEY_W)) camera->target.y -= PLAYER_HOR_SPD * dt * 2;
    if (IsKeyDown(KEY_S)) camera->target.y += PLAYER_HOR_SPD * dt * 2;

    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

    for (size_t i = 0; i < envItemsLength; i++) {
    	EnvItem *ei = envItems + i;
    	minX = fminf(ei->rect.x, minX);
    	maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
    	minY = fminf(ei->rect.y, minY);
    	maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);

    if (max.x < width) camera->target.x -= PLAYER_HOR_SPD * dt * 2;
    if (max.y < height) camera->target.y -= PLAYER_HOR_SPD * dt * 2;
    if (min.x > 0) camera->target.x += PLAYER_HOR_SPD * dt * 2;
    if (min.y > 0) camera->target.y += PLAYER_HOR_SPD * dt * 2;
}

int main(void) {
    //const size_t factor = 60;
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");

    Player player = {
	.position = (Vector2){ 400, 200 },
	.speed = 0,
	.canJump = false,
    };

    //EnvItem *envItems = array_create_init(6, sizeof(EnvItem));
    //array_push(envItems, ((EnvItem){{  0,   0, 1000, 400}, 0, LIGHTGRAY}));
    //array_push(envItems, ((EnvItem){{  0, 400, 1000, 200}, 1, RED}));
    //array_push(envItems, ((EnvItem){{300, 200,  400,  10}, 1, GRAY}));
    //array_push(envItems, ((EnvItem){{250, 300,  100,  10}, 1, GRAY}));
    //array_push(envItems, ((EnvItem){{650, 300,  100,  10}, 1, GRAY}));
    EnvItem envItems[] = {
	((EnvItem){{  0,   0, 1000, 400}, 0, LIGHTGRAY}),
	((EnvItem){{  0, 400, 1000, 200}, 1, RED}),
	((EnvItem){{300, 200,  400,  10}, 1, GRAY}),
	((EnvItem){{250, 300,  100,  10}, 1, GRAY}),
	((EnvItem){{650, 300,  100,  10}, 1, GRAY}),
    };

    int envItemsLength = 5;

    Camera2D camera = {
	.target = player.position,
	.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f },
	.rotation = 0.0f,
	.zoom = 1.0f
    };

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
	float deltatime = GetFrameTime();
	UpdatePlayer(&player, envItems, envItemsLength, deltatime);

	camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

	if (camera.zoom > 1.5f) camera.zoom = 1.5f;
	else if (camera.zoom < 0.90f) camera.zoom = 0.90f;

	if (IsKeyPressed(KEY_R)) {
	    camera.zoom = 1.0f;
	    player.position = (Vector2){ 400, 280 };
	}

	UpdateCameraFreeInsideMap(&camera, &player, envItems, envItemsLength, deltatime, screenWidth, screenHeight);

	Drawing {
            ClearBackground(LIGHTGRAY);

	    Mode2D(camera) {
		for (size_t i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);
		Rectangle playerRect = { player.position.x - 20, player.position.y - 40, 40, 40 };
		DrawRectangleRec(playerRect, RED);
		DrawPixelV(player.position, LIGHTGRAY);
	    }

	    DrawText("Controls:", 20, 20, 10, BLACK);
	    DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
	    DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
	    DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);
	    DrawText("- ZQSD for moving the camera", 40, 100, 10, DARKGRAY);
	    DrawText("Current camera mode:", 20, 120, 10, DARKGRAY);
	    DrawText("Free Camera, but clamp to map edges", 40, 140, 10, DARKGRAY);
	}
    }

    //array_free(envItems);
    CloseWindow();

    return 0;
}
