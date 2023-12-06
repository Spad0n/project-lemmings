/* -*- compile-command: "make -C .. libplug" -*- */
#include "macro.h"
#include "plug.h"
#include "raylib.h"
#include <stddef.h>

#define DOOR_VERTICAL_SIZE 54

#define TEXTURE_GRASS (Rectangle){0, 0, 36, 36}
#define TEXTURE_COIN (Rectangle){396, 252, 36, 36}
#define TEXTURE_KEY (Rectangle){252, 36, 36, 36}
#define TEXTURE_LEVER (Rectangle){180, 108, 36, 36}
#define TEXTURE_BIG_BRICK (Rectangle){252, 0, 36, 36}
#define TEXTURE_SMALL_BRICK (Rectangle){288, 0, 36, 36}
#define TEXTURE_DOOR (Rectangle){360, 198, 36, 54}

#define TEXTURE_PLAYER (Rectangle){0, 0, 48, 48}

void plug_init(Plug *plug) {

    plug->camera.zoom = 1.0f;

    // Init position
    plug->mouse_position = (Vector2){0.0f, 0.0f};
    plug->mouse_tile_pos = (Tile2D){0, 0};
    plug->eraser = false;

    plug->show = true;

    plug->rec = (Rectangle){
	.x = SCREEN_WIDTH - (3 * MAP_TILE_SIZE),
	.y = 0,
	.width = 3 * MAP_TILE_SIZE,
	.height = SCREEN_HEIGHT,
    };

    plug->block_selected = BLOCK_MIDDLE;
    
    player_init(&plug->player, MAP_TILE_SIZE * 2, MAP_TILE_SIZE * 2);
}

void plug_update(Plug *plug) {
    float dt = GetFrameTime();

    // Drag mouse mouvement
    //if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    //	Vector2 delta = GetMouseDelta();
    //	delta = Vector2Scale(delta, -1.0f/plug->camera.zoom);
    //	plug->camera.target = Vector2Add(plug->camera.target, delta);
    //}

    if (IsKeyPressed(KEY_T)) plug->show = plug->show ? false : true;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), plug->rec)) {
	BlockID tiletype[] = {BLOCK_MIDDLE, BLOCK_COIN, BLOCK_KEY, BLOCK_LEVER, BLOCK_S_BRICK, BLOCK_B_BRICK};
	for (size_t i = 0; i < 7; i++) {
	    float j = 3 * i + 1;
	    Rectangle obj = {
		.x = plug->rec.x + MAP_TILE_SIZE/1.2f,
		.y = plug->rec.y + (MAP_TILE_SIZE/2 * j),
		.width = (MAP_TILE_SIZE * 1.3f),
		.height = (MAP_TILE_SIZE * 1.3f),
	    };
	    if (CheckCollisionPointRec(GetMousePosition(), obj)) {
		plug->block_selected = tiletype[i];
	    }
	}
    }

    // toggle block selection
    if (plug->show) {
	if (plug->rec.x > SCREEN_WIDTH - (3 * MAP_TILE_SIZE)) {
	    plug->rec.x -= 500 * dt;
	} else {
	    plug->rec.x = SCREEN_WIDTH - (3 * MAP_TILE_SIZE);
	}
    } else {
	if (plug->rec.x < SCREEN_WIDTH) {
	    plug->rec.x += 500 * dt;
	} else {
	    plug->rec.x = SCREEN_WIDTH;
	}
    }

    // clean all on screen
    if (IsKeyPressed(KEY_D)) {
	for (size_t y = 0; y < TILESY; y++) {
	    for (size_t x = 0; x < TILESX; x++) {
		plug->tilemap[y][x] = BLOCK_EMPTY;
	    }
	}
    }

    // mouse zooming
    //float wheel = GetMouseWheelMove();
    //if (wheel != 0) {
    //	Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), plug->camera);
    //	plug->camera.offset = GetMousePosition();
    //	plug->camera.target = mouseWorldPos;

    //	const float zoomIncrement = 0.125f;
    //	plug->camera.zoom += wheel * zoomIncrement;
    //	if (plug->camera.zoom < zoomIncrement) plug->camera.zoom = zoomIncrement;
    //}

    plug->mouse_position = GetMousePosition();
    plug->mouse_tile_pos.x = (plug->mouse_position.x / plug->camera.zoom + plug->camera.target.x - (plug->camera.offset.x / plug->camera.zoom)) / MAP_TILE_SIZE;
    plug->mouse_tile_pos.y = (plug->mouse_position.y / plug->camera.zoom + plug->camera.target.y - (plug->camera.offset.y / plug->camera.zoom)) / MAP_TILE_SIZE;

    if (IsKeyPressed(KEY_F)) {
	plug->eraser = plug->eraser ? false : true;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX && plug->mouse_tile_pos.y < TILESY && !CheckCollisionPointRec(GetMousePosition(), plug->rec)) {
	int posX = plug->mouse_tile_pos.x;
	int posY = plug->mouse_tile_pos.y;

	// check eraser enabled
	if (!plug->eraser) {
	    plug->tilemap[posY][posX] = plug->block_selected;
	} else {
	    plug->tilemap[posY][posX] = BLOCK_EMPTY;
	}

	// if the block is a grass
	if (plug->block_selected == BLOCK_MIDDLE || plug->block_selected == BLOCK_EMPTY) {
	    if (posY + 1 < TILESY && plug->tilemap[posY + 1][posX] != 0 && plug->tilemap[posY + 1][posX] < BLOCK_COIN) {
		if (plug->tilemap[posY][posX]) {
		    plug->tilemap[posY][posX] |= BLOCK_BOTTOM;
		    plug->tilemap[posY + 1][posX] |= BLOCK_TOP;
		} else {
		    plug->tilemap[posY + 1][posX] &= ~BLOCK_TOP;
		}
	    }

	    if (posY - 1 >= 0 && plug->tilemap[posY - 1][posX] != 0 && plug->tilemap[posY - 1][posX] < BLOCK_COIN) {
		if (plug->tilemap[posY][posX]) {
		    plug->tilemap[posY][posX] |= BLOCK_TOP;
		    plug->tilemap[posY - 1][posX] |= BLOCK_BOTTOM;
		} else {
		    plug->tilemap[posY - 1][posX] &= ~BLOCK_BOTTOM;
		}
	    }

	    if (posX - 1 >= 0 && plug->tilemap[posY][posX - 1] != 0 && plug->tilemap[posY][posX - 1] < BLOCK_COIN) {
		if (plug->tilemap[posY][posX]) {
		    plug->tilemap[posY][posX] |= BLOCK_LEFT;
		    plug->tilemap[posY][posX - 1] |= BLOCK_RIGHT;
		} else {
		    plug->tilemap[posY][posX - 1] &= ~BLOCK_RIGHT;
		}
	    }

	    if (posX + 1 < TILESX && plug->tilemap[posY][posX + 1] != 0 && plug->tilemap[posY][posX + 1] < BLOCK_COIN) {
		if (plug->tilemap[posY][posX]) {
		    plug->tilemap[posY][posX] |= BLOCK_RIGHT;
		    plug->tilemap[posY][posX + 1] |= BLOCK_LEFT;
		} else {
		    plug->tilemap[posY][posX + 1] &= ~BLOCK_LEFT;
		}
	    }
	} else {
	    if (posY + 1 < TILESY && plug->tilemap[posY + 1][posX] < BLOCK_KEY) plug->tilemap[posY + 1][posX] &= ~BLOCK_TOP;
	    if (posY - 1 >= 0 && plug->tilemap[posY - 1][posX] < BLOCK_KEY) plug->tilemap[posY - 1][posX] &= ~BLOCK_BOTTOM;
	    if (posX - 1 >= 0 && plug->tilemap[posY][posX - 1] < BLOCK_KEY) plug->tilemap[posY][posX - 1] &= ~BLOCK_RIGHT;
	    if (posX + 1 < TILESX && plug->tilemap[posY][posX + 1] < BLOCK_KEY) plug->tilemap[posY][posX + 1] &= ~BLOCK_LEFT;
	}
    }
}

static void draw_tilemap(int tile, size_t x, size_t y, Texture2D demoTile) {
    switch (tile) {
    case BLOCK_MIDDLE: return DrawTextureRec(demoTile, (Rectangle){0, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){0, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_TOP: return DrawTextureRec(demoTile, (Rectangle){0, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_LEFT: return DrawTextureRec(demoTile, (Rectangle){108, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT: return DrawTextureRec(demoTile, (Rectangle){36, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){0, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT: return DrawTextureRec(demoTile, (Rectangle){72, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){108, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_TOP: return DrawTextureRec(demoTile, (Rectangle){108, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){36, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT | BLOCK_TOP: return DrawTextureRec(demoTile, (Rectangle){36, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT | BLOCK_TOP: return DrawTextureRec(demoTile, (Rectangle){72, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){72, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){108, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){36, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(demoTile, (Rectangle){72, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 

    // Drawing Other type of blocks
    case BLOCK_COIN: return DrawTextureRec(demoTile, TEXTURE_COIN, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_KEY: return DrawTextureRec(demoTile, TEXTURE_KEY, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_LEVER: return DrawTextureRec(demoTile, TEXTURE_LEVER, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_S_BRICK: return DrawTextureRec(demoTile, TEXTURE_SMALL_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_B_BRICK: return DrawTextureRec(demoTile, TEXTURE_BIG_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    default: return;
    }
}

static void draw_stone_rectangle(Rectangle rec, Texture2D menu) {

    // draw stone rectangle
    Rectangle BOTTOM_RIGHT = {72, 180, 36, 36};
    Rectangle RIGHT = {72, 144, 36, 36};
    Rectangle TOP_RIGHT = {72, 108, 36, 36};
    Rectangle TOP = {36, 108, 36, 36};
    Rectangle TOP_LEFT = {0, 108, 36, 36};
    Rectangle LEFT = {0, 144, 36, 36};
    Rectangle BOTTOM_LEFT = {0, 180, 36, 36};
    Rectangle BOTTOM = {36, 180, 36, 36};
    Rectangle MIDDLE = {36, 144, 36, 36};

    size_t tile_height = MAP_TILE_SIZE * (int)(rec.height / MAP_TILE_SIZE);
    size_t tile_width = MAP_TILE_SIZE * (int)(rec.width / MAP_TILE_SIZE);

    for (size_t y = 0; y < tile_height; y += MAP_TILE_SIZE) {
	for (size_t x = 0; x < tile_width; x += MAP_TILE_SIZE) {
	    if (x == 0 && y == 0) DrawTextureRec(menu, TOP_LEFT, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else if (x == 0 && y == tile_height - MAP_TILE_SIZE) DrawTextureRec(menu, BOTTOM_LEFT, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else if (x == tile_width - MAP_TILE_SIZE && y == 0) DrawTextureRec(menu, TOP_RIGHT, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else if (x == tile_width - MAP_TILE_SIZE && y == tile_height - MAP_TILE_SIZE) DrawTextureRec(menu, BOTTOM_RIGHT, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else if (y == 0) DrawTextureRec(menu, TOP, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else if (x == 0) DrawTextureRec(menu, LEFT, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else if (y == tile_height - MAP_TILE_SIZE) DrawTextureRec(menu, BOTTOM, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else if (x == tile_width - MAP_TILE_SIZE) DrawTextureRec(menu, RIGHT, (Vector2){rec.x + x, rec.y + y}, WHITE);
	    else DrawTextureRec(menu, MIDDLE, (Vector2){rec.x + x, rec.y + y}, WHITE);
	}
    }
}

static void draw_selection(Rectangle rec, Texture2D demoTile) {

    Rectangle recs[7] = {
	TEXTURE_GRASS,
	TEXTURE_COIN,
	TEXTURE_KEY,
	TEXTURE_LEVER,
	TEXTURE_SMALL_BRICK,
	TEXTURE_BIG_BRICK,
	TEXTURE_DOOR
    };

    for (size_t i = 0; i < 7; i++) {
	float j = 3 * i + 1;
	//DrawTexturePro(demoTile, recs[i], (Rectangle){rec.x + ((float) MAP_TILE_SIZE / 1.2f), rec.y + (MAP_TILE_SIZE/2) * j, MAP_TILE_SIZE * 1.3, MAP_TILE_SIZE * 1.3}, (Vector2){0, 0}, 0.0f, WHITE);
	if (i < 6) {
	    DrawTexturePro(demoTile, recs[i], (Rectangle){rec.x, rec.y, MAP_TILE_SIZE * 1.3f, MAP_TILE_SIZE * 1.3f}, (Vector2){ - MAP_TILE_SIZE/1.2f, - (MAP_TILE_SIZE/2 * j)}, 0.0f, WHITE);
	} else {
	    DrawTexturePro(demoTile, recs[i], (Rectangle){rec.x, rec.y, MAP_TILE_SIZE * 1.3f, 54 * 1.3f}, (Vector2){ - MAP_TILE_SIZE/1.2f, - (MAP_TILE_SIZE/2 * j)}, 0.0f, WHITE);
	}

	//DrawRectanglePro((Rectangle){rec.x, rec.y, MAP_TILE_SIZE * 1.3f, MAP_TILE_SIZE * 1.3f}, (Vector2){ - MAP_TILE_SIZE/1.2f, - (MAP_TILE_SIZE/2 * j)}, 0.0f, RED);
    }

}

void plug_temp(Plug *plug) {
    player_physic_update(&plug->player);
    player_state_update(plug);
    player_update(&plug->player);
}

static void draw_player(Plug *plug, Texture2D player) {
    DrawTextureRec(player, TEXTURE_PLAYER, plug->player.player_position, WHITE);
}

void plug_render(Plug *plug, Texture2D background, Texture2D demoTile, Texture2D menu, Texture2D player) {
    Drawing {
	ClearBackground(BLACK);
	Mode2D(plug->camera) {
	    DrawTextureEx(background, (Vector2){0, 0}, 0, 6.67, WHITE);
	    for (int y = 0; y < TILESY; y++) {
		for (int x = 0; x < TILESX; x++) {
		    if (plug->tilemap[y][x]) {
			draw_tilemap(plug->tilemap[y][x], x, y, demoTile);
		    }
		    DrawRectangleLines(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, x == plug->mouse_tile_pos.x && y == plug->mouse_tile_pos.y ? RED : Fade(BLACK, 0.3f));
		}
	    }
	}
	//draw_stone_rectangle((Rectangle){SCREEN_WIDTH - (3 * MAP_TILE_SIZE), 0, MAP_TILE_SIZE * 3, SCREEN_HEIGHT}, menu);
	draw_stone_rectangle(plug->rec, menu);
	draw_selection(plug->rec, demoTile);
	
	draw_player(plug, player);

	DrawText(TextFormat("Mouse coordinate on tile: [%d,%d]", plug->mouse_tile_pos.x, plug->mouse_tile_pos.y), 10, 10, 20, BLACK);
	DrawText(TextFormat("Mouse coordinate: [%1.f,%1.f]", plug->mouse_position.x, plug->mouse_position.y), 10, 35, 20, BLACK);
	DrawText(TextFormat("Tile value: %d", plug->tilemap[plug->mouse_tile_pos.y][plug->mouse_tile_pos.x]), 10, 60, 20, BLACK);
    }
}
