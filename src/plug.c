/* -*- compile-command: "make -C .. libplug" -*- */
#include "macro.h"
#include "plug.h"
#include "raylib.h"
#include <stddef.h>
#include <ctype.h>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_NO_ICONS
#include "raygui.h"

#include "layout.h"
#include "array.h"

#define TEXTURE_GRASS (Rectangle){0, 0, 36, 36}
#define TEXTURE_COIN (Rectangle){396, 252, 36, 36}
#define TEXTURE_KEY (Rectangle){252, 36, 36, 36}
#define TEXTURE_SPIKE (Rectangle){288, 108, 36, 36}
#define TEXTURE_BIG_BRICK (Rectangle){252, 0, 36, 36}
#define TEXTURE_SMALL_BRICK (Rectangle){288, 0, 36, 36}
#define TEXTURE_DOOR (Rectangle){360, 198, 36, 54}
#define TEXTURE_BRICK (Rectangle){216, 0, 36, 36}

#define TEXTURE_PLAYER (Rectangle){0, 0, 48, 48}
#define TEXTURE_PLAYER_FLOP (Rectangle){384, 0, 48, 48}

#define PLAYER_JUMP_SPD 400.0f

static void reset_paths(Plug *plug) {
    for (size_t i = 0; i < array_size(plug->paths); i++) {
	free(plug->paths[i]);
    }
    array_free(plug->paths);
}

static Item set_item(Key key, Value val) {
    Item item = {
	.value = val,
	.key = key,
    };
    return item;
}

static void open_level(Plug *plug, const char *file_path) {
    XMLDocument doc = {0};
    if (xml_load(&doc, file_path)) {
    	XMLNode *csv = xml_node_find_tag(doc.root, "csv");
	//printf("csv inner_text: %s\n", csv->inner_text);
	Array_XMLNode players = xml_node_find_tags(doc.root, "player");
	for (size_t i = 0; i < array_size(players); i++) {
	    char *char_x = xml_attrib_get_value(players[i], "x");
	    char *char_y = xml_attrib_get_value(players[i], "y");
	    int x = atoi(char_x);
	    int y = atoi(char_y);
	    array_push(plug->players, entity_init(MAP_TILE_SIZE * x, MAP_TILE_SIZE * y));
	}
    	int index = 0;
    	for (size_t y = 0; y < TILESY; y++) {
    	    for (size_t x = 0; x < TILESX; x++) {
    		char number[4];
    		int num_index = 0;
    		while (isdigit(csv->inner_text[index])) {
    		    number[num_index++] = csv->inner_text[index++];
    		}
		number[num_index] = '\0';
    		index++;
		plug->tilemap[y][x] = atoi(number);
		if (plug->tilemap[y][x] == BLOCK_COIN) {
		    plug->max_coins += 1;
		}
    	    }
    	}
	plug->attempt = array_size(plug->players);
	array_free(players);
    	xml_doc_free(&doc);
    } else {
	fprintf(stderr, "failed to open the level: %s\n", file_path);
    	for (size_t y = 0; y < TILESY; y++) {
    	    for (size_t x = 0; x < TILESX; x++) {
    		plug->tilemap[y][x] = BLOCK_EMPTY;
    	    }
    	}
    }
}

void plug_init(Plug *plug) {

    plug->camera.zoom = 1.0f;

    // Init position
    plug->mouse_position = (Vector2){0.0f, 0.0f};
    plug->mouse_tile_pos = (Tile2D){0, 0};
    plug->eraser = false;

    Value val = {0};
    //val.block_id = BLOCK_MIDDLE;
    val.entity = (Tile2D){0, 0};

    //plug->item_selected = set_item(BLOCK, val);
    plug->item_selected = set_item(ENTITY, val);

    plug->state = START_MENU;
    plug->dialog = DIALOG_NONE;
    
    plug->players = array_create_init(2, sizeof(Entity));
    plug->layouts = array_create_init(4, sizeof(Layout));

    plug->paths = xml_get_filepaths("levels");

    plug->window_should_close = false;

    for (size_t y = 0; y < TILESY; y++) {
        for (size_t x = 0; x < TILESX; x++) {
	    plug->tilemap[y][x] = BLOCK_EMPTY;
        }
    }

    plug->page = 0;
}

void plug_update(Plug *plug) {
    // debug
    if (IsKeyPressed(KEY_Y)) plug->state = plug->state == EDITOR ? GAME : EDITOR;

    switch (plug->state) {
    case EDITOR:
	if (IsKeyPressed(KEY_A)) plug->dialog = DIALOG_EDITOR;
	if (IsKeyPressed(KEY_T) && plug->dialog == DIALOG_NONE) plug->show = plug->show ? false : true;
	if (IsKeyPressed(KEY_D) && plug->state == EDITOR && plug->dialog == DIALOG_NONE) {
	    for (size_t y = 0; y < TILESY; y++) {
		for (size_t x = 0; x < TILESX; x++) {
		    plug->tilemap[y][x] = BLOCK_EMPTY;
		}
	    }
	    array_clear(plug->players);
	}
	plug->mouse_position = GetMousePosition();
	plug->mouse_tile_pos.x = (plug->mouse_position.x / plug->camera.zoom + plug->camera.target.x - (plug->camera.offset.x / plug->camera.zoom)) / MAP_TILE_SIZE;
	plug->mouse_tile_pos.y = (plug->mouse_position.y / plug->camera.zoom + plug->camera.target.y - (plug->camera.offset.y / plug->camera.zoom)) / MAP_TILE_SIZE;

	if (IsKeyPressed(KEY_E)) {
	    plug->eraser = plug->eraser ? false : true;
	}

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX - (plug->show ? 3 : 0) && plug->item_selected.key == ENTITY) {
	    int posX = plug->mouse_tile_pos.x;
	    int posY = plug->mouse_tile_pos.y;
	    array_push(plug->players, entity_init(posX * MAP_TILE_SIZE, posY * MAP_TILE_SIZE));
	}

	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX - (plug->show ? 3 : 0) && plug->mouse_tile_pos.y < TILESY && plug->dialog == DIALOG_NONE) {
	    int posX = plug->mouse_tile_pos.x;
	    int posY = plug->mouse_tile_pos.y;

	    Value val = plug->item_selected.value;
	    Key key = plug->item_selected.key;
	    // check eraser enabled
	    if (!plug->eraser) {
		switch (key) {
		case BLOCK:
		    plug->tilemap[posY][posX] = val.block_id;
		    break;
		case ENTITY:
		    plug->tilemap[posY][posX] = BLOCK_EMPTY;
		    //array_push(plug->players, entity_init(posX * MAP_TILE_SIZE, posY * MAP_TILE_SIZE));
		    break;
		default: break;
		}
	    } else {
		plug->tilemap[posY][posX] = BLOCK_EMPTY;
		for (size_t i = 0; i < array_size(plug->players); i++) {
		    if (CheckCollisionPointRec(GetMousePosition(), plug->players[i].rect)) {
			array_pop_at(plug->players, i);
		    }
		}
	    }

	    // if the block is a grass
	    if (plug->tilemap[posY][posX] == BLOCK_MIDDLE || plug->tilemap[posY][posX] == BLOCK_EMPTY) {
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
		if (posY + 1 < TILESY && plug->tilemap[posY + 1][posX] < BLOCK_COIN) plug->tilemap[posY + 1][posX] &= ~BLOCK_TOP;
		if (posY - 1 >= 0 && plug->tilemap[posY - 1][posX] < BLOCK_COIN) plug->tilemap[posY - 1][posX] &= ~BLOCK_BOTTOM;
		if (posX - 1 >= 0 && plug->tilemap[posY][posX - 1] < BLOCK_COIN) plug->tilemap[posY][posX - 1] &= ~BLOCK_RIGHT;
		if (posX + 1 < TILESX && plug->tilemap[posY][posX + 1] < BLOCK_COIN) plug->tilemap[posY][posX + 1] &= ~BLOCK_LEFT;
	    }
	}
	break;
    case GAME:
	if (IsKeyPressed(KEY_A)) plug->dialog = DIALOG_PAUSE;
	plug->mouse_position = GetMousePosition();
	plug->mouse_tile_pos.x = (plug->mouse_position.x / plug->camera.zoom + plug->camera.target.x - (plug->camera.offset.x / plug->camera.zoom)) / MAP_TILE_SIZE;
	plug->mouse_tile_pos.y = (plug->mouse_position.y / plug->camera.zoom + plug->camera.target.y - (plug->camera.offset.y / plug->camera.zoom)) / MAP_TILE_SIZE;

	if (IsKeyPressed(KEY_E)) {
	    plug->eraser = plug->eraser ? false : true;
	}

	for (size_t i = 0; i < array_size(plug->players); i++) {
	    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), plug->players[i].rect)) {
		plug->players[i].state = MOVE_RIGHT;
		//plug->player_selected = i;
	    }
	}

	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
	    int posX = plug->mouse_tile_pos.x;
	    int posY = plug->mouse_tile_pos.y;
	    if (plug->tilemap[posY][posX] == BLOCK_EMPTY && plug->bricks && !plug->eraser) {
		plug->tilemap[posY][posX] = BLOCK_BRICK;
		plug->bricks--;
	    } else if (plug->tilemap[posY][posX] == BLOCK_BRICK && plug->eraser) {
		plug->tilemap[posY][posX] = BLOCK_EMPTY;
		plug->bricks++;
	    }
	}

	//if (plug->player_selected < array_size(plug->players)) {
	//    Entity *player = &plug->players[plug->player_selected];
	//    if (IsKeyPressed(KEY_RIGHT)) {
	//	//plug->players[plug->player_selected].state = MOVE_RIGHT;
	//	player->state = MOVE_RIGHT;
	//    }
	//    if (IsKeyPressed(KEY_LEFT)) {
	//	//plug->players[plug->player_selected].state = MOVE_LEFT;
	//	player->state = MOVE_LEFT;
	//    }
	//    //if (IsKeyPressed(KEY_SPACE) && plug->players[plug->player_selected].on_ground && plug->players[plug->player_selected]) {
	//    if (IsKeyPressed(KEY_SPACE) && player->on_ground && player->state != STATIC) {
	//	float dt = GetFrameTime();
	//	//plug->players[plug->player_selected].velocity.y -= PLAYER_JUMP_SPD * dt; 
	//	player->velocity.y -= PLAYER_JUMP_SPD * dt;
	//	//player->velocity.y -= PLAYER_JUMP_SPD * dt;
	//    }
	//} else {
	//    plug->player_selected = 0;
	//}
	break;
    default: break;
    }
}

//void plug_update(Plug *plug) {
//
//    // Drag mouse mouvement
//    //if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
//    //	Vector2 delta = GetMouseDelta();
//    //	delta = Vector2Scale(delta, -1.0f/plug->camera.zoom);
//    //	plug->camera.target = Vector2Add(plug->camera.target, delta);
//    //}
//
//    if (IsKeyPressed(KEY_A) && plug->state == EDITOR) plug->dialog = DIALOG_EDITOR;
//
//    if (IsKeyPressed(KEY_T) && plug->state == EDITOR && plug->dialog == DIALOG_NONE) plug->show = plug->show ? false : true;
//
//    //if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), plug->rec) && plug->state == EDITOR) {
//    //	BlockID tiletype[] = {BLOCK_MIDDLE, BLOCK_COIN, BLOCK_KEY, BLOCK_LEVER, BLOCK_S_BRICK, BLOCK_B_BRICK, BLOCK_DOOR};
//    //	for (size_t i = 0; i < 7; i++) {
//    //	    float j = 3 * i + 1;
//    //	    Rectangle obj = {
//    //		.x = plug->rec.x + MAP_TILE_SIZE/1.2f,
//    //		.y = plug->rec.y + (MAP_TILE_SIZE/2 * j),
//    //		.width = (MAP_TILE_SIZE * 1.3f),
//    //		.height = (MAP_TILE_SIZE * 1.3f),
//    //	    };
//    //	    if (CheckCollisionPointRec(GetMousePosition(), obj)) {
//    //		plug->block_selected = tiletype[i];
//    //	    }
//    //	}
//    //}
//
//    // toggle block selection
//    //if (plug->show) {
//    //	if (plug->rec.x > SCREEN_WIDTH - (3 * MAP_TILE_SIZE)) {
//    //	    plug->rec.x -= 500 * dt;
//    //	} else {
//    //	    plug->rec.x = SCREEN_WIDTH - (3 * MAP_TILE_SIZE);
//    //	}
//    //} else {
//    //	if (plug->rec.x < SCREEN_WIDTH) {
//    //	    plug->rec.x += 500 * dt;
//    //	} else {
//    //	    plug->rec.x = SCREEN_WIDTH;
//    //	}
//    //}
//
//    // clean all on screen
//    if (IsKeyPressed(KEY_D) && plug->state == EDITOR && plug->dialog == DIALOG_NONE) {
//	for (size_t y = 0; y < TILESY; y++) {
//	    for (size_t x = 0; x < TILESX; x++) {
//		plug->tilemap[y][x] = BLOCK_EMPTY;
//	    }
//	}
//	array_clear(plug->players);
//    }
//
//    // mouse zooming
//    //float wheel = GetMouseWheelMove();
//    //if (wheel != 0) {
//    //	Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), plug->camera);
//    //	plug->camera.offset = GetMousePosition();
//    //	plug->camera.target = mouseWorldPos;
//
//    //	const float zoomIncrement = 0.125f;
//    //	plug->camera.zoom += wheel * zoomIncrement;
//    //	if (plug->camera.zoom < zoomIncrement) plug->camera.zoom = zoomIncrement;
//    //}
//
//    plug->mouse_position = GetMousePosition();
//    plug->mouse_tile_pos.x = (plug->mouse_position.x / plug->camera.zoom + plug->camera.target.x - (plug->camera.offset.x / plug->camera.zoom)) / MAP_TILE_SIZE;
//    plug->mouse_tile_pos.y = (plug->mouse_position.y / plug->camera.zoom + plug->camera.target.y - (plug->camera.offset.y / plug->camera.zoom)) / MAP_TILE_SIZE;
//
//    if (IsKeyPressed(KEY_F)) {
//	plug->eraser = plug->eraser ? false : true;
//    }
//
//    int side_menu = plug->show ? 3 : 0 ;
//
//    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX - side_menu && plug->item_selected.key == ENTITY) {
//	int posX = plug->mouse_tile_pos.x;
//	int posY = plug->mouse_tile_pos.y;
//	array_push(plug->players, entity_init(posX * MAP_TILE_SIZE, posY * MAP_TILE_SIZE));
//    }
//
//    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX - side_menu && plug->mouse_tile_pos.y < TILESY && plug->state == EDITOR && plug->dialog == DIALOG_NONE) {
//	int posX = plug->mouse_tile_pos.x;
//	int posY = plug->mouse_tile_pos.y;
//
//	Value val = plug->item_selected.value;
//	Key key = plug->item_selected.key;
//	// check eraser enabled
//	if (!plug->eraser) {
//	    switch (key) {
//	    case BLOCK:
//		plug->tilemap[posY][posX] = val.block_id;
//		break;
//	    case ENTITY:
//		plug->tilemap[posY][posX] = BLOCK_EMPTY;
//		//array_push(plug->players, entity_init(posX * MAP_TILE_SIZE, posY * MAP_TILE_SIZE));
//		break;
//	    default: break;
//	    }
//	} else {
//	    plug->tilemap[posY][posX] = BLOCK_EMPTY;
//	    for (size_t i = 0; i < array_size(plug->players); i++) {
//		if (CheckCollisionPointRec(GetMousePosition(), plug->players[i].rect)) {
//		    array_pop_at(plug->players, i);
//		}
//	    }
//	}
//
//	// if the block is a grass
//	if (plug->tilemap[posY][posX] == BLOCK_MIDDLE || plug->tilemap[posY][posX] == BLOCK_EMPTY) {
//	    if (posY + 1 < TILESY && plug->tilemap[posY + 1][posX] != 0 && plug->tilemap[posY + 1][posX] < BLOCK_COIN) {
//		if (plug->tilemap[posY][posX]) {
//		    plug->tilemap[posY][posX] |= BLOCK_BOTTOM;
//		    plug->tilemap[posY + 1][posX] |= BLOCK_TOP;
//		} else {
//		    plug->tilemap[posY + 1][posX] &= ~BLOCK_TOP;
//		}
//	    }
//
//	    if (posY - 1 >= 0 && plug->tilemap[posY - 1][posX] != 0 && plug->tilemap[posY - 1][posX] < BLOCK_COIN) {
//		if (plug->tilemap[posY][posX]) {
//		    plug->tilemap[posY][posX] |= BLOCK_TOP;
//		    plug->tilemap[posY - 1][posX] |= BLOCK_BOTTOM;
//		} else {
//		    plug->tilemap[posY - 1][posX] &= ~BLOCK_BOTTOM;
//		}
//	    }
//
//	    if (posX - 1 >= 0 && plug->tilemap[posY][posX - 1] != 0 && plug->tilemap[posY][posX - 1] < BLOCK_COIN) {
//		if (plug->tilemap[posY][posX]) {
//		    plug->tilemap[posY][posX] |= BLOCK_LEFT;
//		    plug->tilemap[posY][posX - 1] |= BLOCK_RIGHT;
//		} else {
//		    plug->tilemap[posY][posX - 1] &= ~BLOCK_RIGHT;
//		}
//	    }
//
//	    if (posX + 1 < TILESX && plug->tilemap[posY][posX + 1] != 0 && plug->tilemap[posY][posX + 1] < BLOCK_COIN) {
//		if (plug->tilemap[posY][posX]) {
//		    plug->tilemap[posY][posX] |= BLOCK_RIGHT;
//		    plug->tilemap[posY][posX + 1] |= BLOCK_LEFT;
//		} else {
//		    plug->tilemap[posY][posX + 1] &= ~BLOCK_LEFT;
//		}
//	    }
//	} else {
//	    if (posY + 1 < TILESY && plug->tilemap[posY + 1][posX] < BLOCK_COIN) plug->tilemap[posY + 1][posX] &= ~BLOCK_TOP;
//	    if (posY - 1 >= 0 && plug->tilemap[posY - 1][posX] < BLOCK_COIN) plug->tilemap[posY - 1][posX] &= ~BLOCK_BOTTOM;
//	    if (posX - 1 >= 0 && plug->tilemap[posY][posX - 1] < BLOCK_COIN) plug->tilemap[posY][posX - 1] &= ~BLOCK_RIGHT;
//	    if (posX + 1 < TILESX && plug->tilemap[posY][posX + 1] < BLOCK_COIN) plug->tilemap[posY][posX + 1] &= ~BLOCK_LEFT;
//	}
//    }
//}

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

    // dessine les block interactif avec le joueur
    case BLOCK_COIN: return DrawTextureRec(demoTile, TEXTURE_COIN, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_SPIKE: return DrawTextureRec(demoTile, TEXTURE_SPIKE, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_S_BRICK: return DrawTextureRec(demoTile, TEXTURE_SMALL_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_B_BRICK: return DrawTextureRec(demoTile, TEXTURE_BIG_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_DOOR: return DrawTextureRec(demoTile, TEXTURE_DOOR, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE - 18}, WHITE); 
    case BLOCK_BRICK: return DrawTextureRec(demoTile, TEXTURE_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    default: return;
    }
}

void plug_temp(Plug *plug) {
    entity_update(plug);
}

static void draw_player(Plug *plug, Texture2D player_texture, Texture2D player_flop) {
    //DrawTextureRec(player, TEXTURE_PLAYER, plug->player.player_position, WHITE);
    for (size_t i = 0; i < array_size(plug->players); i++) {
	//Entity player = array_last(plug->player);
	Entity player = plug->players[i];
	if (player.state == MOVE_LEFT) {
	    DrawTextureRec(player_texture, TEXTURE_PLAYER, (Vector2){player.rect.x - 12, player.rect.y - 12}, WHITE);
	    //DrawRectangleRec(player.rect, Fade(RED, 0.5f));
	} else {
	    DrawTextureRec(player_flop, TEXTURE_PLAYER_FLOP, (Vector2){player.rect.x - 12, player.rect.y - 12}, WHITE);
	    //DrawRectangleRec(player.rect, Fade(RED, 0.5f));
	}
    }

    //DrawRectangle(plug->player.rect.x + plug->player.rect.width, plug->player.rect.y, 2, 2, BLUE);
}

static void draw_metal_box(Plug *plug, Texture demoTile, Texture player) {
    float dt = GetFrameTime();
    //static Rectangle rec = {
    //	.x = SCREEN_WIDTH,
    //	.y = 0,
    //	.width = 3 * UI_TILE_SIZE,
    //	.height = SCREEN_HEIGHT,
    //};
    static Rectangle rec = {
	.x = SCREEN_WIDTH,
	.y = 0,
	.width = 3 * MAP_TILE_SIZE,
	.height = SCREEN_HEIGHT,
    };

    Rectangle recs[7] = {
	TEXTURE_GRASS,
	TEXTURE_COIN,
	TEXTURE_SPIKE,
	TEXTURE_SMALL_BRICK,
	TEXTURE_BIG_BRICK,
	TEXTURE_DOOR,
	TEXTURE_PLAYER,
    };

    if (plug->show) {
    	if (rec.x > SCREEN_WIDTH - (3 * MAP_TILE_SIZE)) {
    	    rec.x -= 500 * dt;
    	} else {
    	    rec.x = SCREEN_WIDTH - (3 * MAP_TILE_SIZE);
    	}
    } else {
    	if (rec.x < SCREEN_WIDTH) {
    	    rec.x += 500 * dt;
    	} else {
    	    rec.x = SCREEN_WIDTH;
    	}
    }

    //if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), rec) && plug->state == EDITOR) {
    //	BlockID tiletype[] = {BLOCK_MIDDLE, BLOCK_COIN, BLOCK_KEY, BLOCK_LEVER, BLOCK_S_BRICK, BLOCK_B_BRICK, BLOCK_DOOR};
    //	for (size_t i = 0; i < 7; i++) {
    //	    float j = 3 * i + 1;
    //	    Rectangle obj = {
    //		.x = rec.x + MAP_TILE_SIZE/1.2f,
    //		.y = rec.y + (MAP_TILE_SIZE/2 * j),
    //		.width = (MAP_TILE_SIZE * 1.3f),
    //		.height = (MAP_TILE_SIZE * 1.3f),
    //	    };
    //	    if (CheckCollisionPointRec(GetMousePosition(), obj)) {
    //		plug->block_selected = tiletype[i];
    //	    }
    //	}
    //}

    int tiletype[] = {
    	BLOCK_MIDDLE,
    	BLOCK_COIN,
    	BLOCK_SPIKE,
    	BLOCK_S_BRICK,
    	BLOCK_B_BRICK,
    	BLOCK_DOOR,
    };

    // draw stone rectangle
    //ui_draw_dialog(rec, menu, Metal, WHITE);
    GuiDrawRectangle(rec, 2, BLACK, GetColor(0xd6dde7ff));
    rec.width = MAP_TILE_SIZE * (int)(rec.width / MAP_TILE_SIZE);
    rec.height = MAP_TILE_SIZE * (int)(rec.height / MAP_TILE_SIZE);
    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x, rec.y, rec.width, rec.height), 7, 0) {
	for (size_t i = 0; i < 7; i++) {
	    Rectangle tile_position = layout_stack_slot(&plug->layouts);
	    tile_position.y -= 5;

	    size_t tile_width = MAP_TILE_SIZE * (int)(tile_position.width / MAP_TILE_SIZE);
	    //tile_position.x = tile_position.x + (tile_position.width - tile_width) / 2;
	    tile_position.width = tile_width;
	    size_t tile_height = MAP_TILE_SIZE * (int)(tile_position.height / MAP_TILE_SIZE);
	    //tile_position.y = tile_position.y + (tile_position.height - tile_height) / 2;
	    tile_position.height = tile_height;
	    
	    tile_position.x = ceilf(tile_position.x + (tile_position.width - recs[i].width) / 2);
	    tile_position.y = ceilf(tile_position.y + (tile_position.height - recs[i].height) / 2);
	    tile_position.width = recs[i].width;
	    tile_position.height = recs[i].height;

	    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), tile_position)) {
		//plug->block_selected = tiletype[i];
		if (i < 6) {
		    plug->item_selected.value.block_id = tiletype[i];
		    plug->item_selected.key = BLOCK;
		} else {
		    plug->item_selected.value.entity = (Tile2D){0, 0};
		    plug->item_selected.key = ENTITY;
		}
	    }

	    //DrawRectangleRec(tile_position, Fade(RED, 0.5f));
	    if (i < 6) {
		layout_item(demoTile, tile_position, recs[i]);
	    } else {
		layout_item(player, tile_position, recs[i]);
	    }
	}
    }
}

//static void draw_dialog_glass(Plug *plug, Texture2D dialog) {
//    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.9f));
//    Rectangle rec = {
//	.x = GetScreenWidth()/2 - 400/2,
//	.y = GetScreenHeight()/2 - 235/2,
//	.width = UI_TILE_SIZE * (int)(400 / UI_TILE_SIZE),
//	.height = UI_TILE_SIZE * (int)(235 / UI_TILE_SIZE),
//    };
//    float gap = 10.0f;
//    //ui_draw_dialog(rec, dialog, Glass, WHITE);
//
//    GuiDrawRectangle(rec, 2, WHITE, GetColor(0xcde0e766));
//    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
//    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
//
//    UILayoutDrawing(&plug->layouts, LO_VERT, ui_make_layout_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 2, gap) {
//	//ui_widget_text(ui_layout_stack_slot(&plug->layouts), "do you want to quit ?", WHITE);
//	GuiLabel(ui_layout_stack_slot(&plug->layouts), "do you want to quit ?");
//	UILayoutDrawing(&plug->layouts, LO_HORI, ui_layout_stack_slot(&plug->layouts), 2, gap) {
//	    if (GuiButton(ui_layout_stack_slot(&plug->layouts), "yes")) {
//		plug->window_should_close = true;
//	    }
//	    if (GuiButton(ui_layout_stack_slot(&plug->layouts), "no")) {
//		plug->state = EDITOR;
//	    }
//	}
//    }
//}

static void draw_level_select(Plug *plug) {
    Rectangle rec = {
	.x = GetScreenWidth()/2 - 700/2,
	.y = GetScreenHeight()/2 - 420/2,
	.width = 700,
	.height = 420,
    };

    float gap = 10.0f;

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, 0xffffffff);

    size_t index = 9 * plug->page;

    Drawing {
	ClearBackground(BLACK);
	GuiDrawRectangle(rec, 2, WHITE, GetColor(0xcde0e766));
	LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 3, gap) {
	    Rectangle top_layout = layout_stack_slot(&plug->layouts);
	    GuiLabel(top_layout, "choose a level");
	    Rectangle top_right = {
		.x = top_layout.width - gap,
		.y = top_layout.y,
		.width = top_layout.height/2,
		.height = top_layout.height/2,
	    };
	    if (GuiButton(top_right, "New")) {
		array_clear(plug->players);
		for (size_t y = 0; y < TILESY; y++) {
		    for (size_t x = 0; x < TILESX; x++) {
			plug->tilemap[y][x] = BLOCK_EMPTY;
		    }
		}
		plug->state = EDITOR;
	    }

	    Rectangle top_left = {
		.x = top_layout.x,
		.y = top_layout.y,
		.width = top_layout.height/2,
		.height = top_layout.height/2,
	    };
	    if (GuiButton(top_left, "Quit")) {
		plug->window_should_close = true;
	    }

	    Rectangle level_recs = layout_stack_slot(&plug->layouts);
	    level_recs.height += level_recs.height / 2;
	    LayoutDrawing(&plug->layouts, LO_VERT, level_recs, 3, gap) {
		for (size_t i = 0; i < 3; i++) {
		    LayoutDrawing(&plug->layouts, LO_HORI, layout_stack_slot(&plug->layouts), 3, gap) {
			for (size_t j = 0; j < 3; j++) {
			    if (index < array_size(plug->paths)) {
				if (GuiButton(layout_stack_slot(&plug->layouts), plug->paths[index])) {
				    array_clear(plug->players);
				    open_level(plug, plug->paths[index]);
				    //plug->state = EDITOR;
				    plug->state = GAME;
				    plug->level_selected = index;
				}
			    }
			    index += 1;
			}
		    }
		}
	    }
	    Rectangle page_recs = layout_stack_slot(&plug->layouts);
	    page_recs.y += page_recs.height / 2;
	    page_recs.height -= page_recs.height / 2;
	    Rectangle previous = {
		.x = page_recs.x,
		.y = page_recs.y,
		.width = page_recs.height,
		.height = page_recs.height,
	    };
	    Rectangle next = {
		.x = page_recs.width - gap,
		.y = page_recs.y,
		.width = page_recs.height,
		.height = page_recs.height,
	    };
	    char page[10];
	    sprintf(page, "%ld/%d", plug->page + 1, (int)ceil((float)array_size(plug->paths)/9));
	    GuiLabel(page_recs, page);
	    if (GuiButton(previous, "<")) {
		if (plug->page > 0) plug->page -= 1;
	    }
	    if (GuiButton(next, ">")) {
		if (9 * (plug->page + 1) < array_size(plug->paths)) plug->page += 1;
	    }
	}

	if (plug->dialog == DIALOG_SELECT) {
	    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.9f));
	    Rectangle rec = {
		.x = GetScreenWidth()/2 - 450/2,
		.y = GetScreenWidth()/2 - 250/2,
		.width = 450,
		.height = 250,
	    };
	    GuiDrawRectangle(rec, 2, WHITE, GetColor(0xcde0e766));
	    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 3, gap) {
		GuiLabel(layout_stack_slot(&plug->layouts), "choose a mode");
		if (GuiButton(layout_stack_slot(&plug->layouts), "Play")) {
		    plug->state = GAME;
		    plug->dialog = DIALOG_NONE;
		}
		if (GuiButton(layout_stack_slot(&plug->layouts), "Edit")) {
		    plug->state = EDITOR;
		    plug->dialog = DIALOG_NONE;
		}
		if (GuiButton(layout_stack_slot(&plug->layouts), "Back")) {
		    plug->dialog = DIALOG_NONE;
		}
	    }
	}
    }
}

void plug_save(Plug *plug, char *file_path) {
    int string_size = TILESY * (TILESX * 5) + 1;
    char *S = malloc(sizeof(char) * string_size);
    S[0] = '\0';
    for (size_t y = 0; y < TILESY; y++) {
	for (size_t x = 0; x < TILESX; x++) {
	    char itoa[4];
	    itoa[0] = '\0';
	    sprintf(itoa, "%d", plug->tilemap[y][x]);
	    strncat(S, itoa, 4);

	    if (y != TILESY - 1 || x != TILESX - 1) strcat(S, ",");
	}
	if (y != TILESY - 1) strcat(S, "\n");
    }
    XMLDocument doc = xml_doc_init("root");
    XMLNode *csv = xml_node_new(doc.root);
    csv->tag = strdup("csv");
    csv->inner_text = S;

    for (size_t i = 0; i < array_size(plug->players); i++) {
	XMLNode *player = xml_node_new(doc.root);
	player->tag = strdup("player");
	char char_x[4];
	char_x[0] = '\0';
	char char_y[4];
	char_y[0] = '\0';

	sprintf(char_x, "%d", (int)(plug->players[i].rect.x / MAP_TILE_SIZE));
	sprintf(char_y, "%d", (int)(plug->players[i].rect.y / MAP_TILE_SIZE));
	xml_attrib_add(player, "x", char_x);
	xml_attrib_add(player, "y", char_y);
    }

    xml_doc_write(&doc, file_path, 2);

    xml_doc_free(&doc);
}

static void draw_level_editor(Plug *plug, Texture2D background, Texture2D tileset, Texture2D player, Texture2D player_flop) {
    static char text_box[10];
    Drawing {
	ClearBackground(BLACK);
	Mode2D(plug->camera) {
	    DrawTextureEx(background, (Vector2){0, 0}, 0, 6.67, WHITE);
	    for (int y = 0; y < TILESY; y++) {
		for (int x = 0; x < TILESX; x++) {
		    if (plug->tilemap[y][x]) {
			draw_tilemap(plug->tilemap[y][x], x, y, tileset);
		    }
		    DrawRectangleLines(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, x == plug->mouse_tile_pos.x && y == plug->mouse_tile_pos.y ? RED : Fade(BLACK, 0.3f));
		}
	    }
	    draw_player(plug, player, player_flop);
	}
	draw_metal_box(plug, tileset, player);

	DrawText(TextFormat("Mouse coordinate on tile: [%d,%d]", plug->mouse_tile_pos.x, plug->mouse_tile_pos.y), 10, 10, 20, BLACK);
	DrawText(TextFormat("Mouse coordinate: [%1.f,%1.f]", plug->mouse_position.x, plug->mouse_position.y), 10, 35, 20, BLACK);
	DrawText(TextFormat("Tile value: %d", plug->tilemap[plug->mouse_tile_pos.y][plug->mouse_tile_pos.x]), 10, 60, 20, BLACK);

	if (plug->dialog == DIALOG_EDITOR) {
	    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.9f));
	    Rectangle rec = {
		.x = GetScreenWidth()/2 - 400/2,
		.y = GetScreenHeight()/2 - 300/2,
		.width = 400,
		.height = 300,
	    };
	    float gap = 10.0f;
	    
	    GuiDrawRectangle(rec, 2, WHITE, GetColor(0xcde0e766));
	    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
	    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	    
	    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 5, gap) {
		GuiLabel(layout_stack_slot(&plug->layouts), "do you want to quit ?");

		if (GuiButton(layout_stack_slot(&plug->layouts), "quit")) {
		    plug->state = START_MENU;
		    plug->dialog = DIALOG_NONE;
		}
		if (GuiButton(layout_stack_slot(&plug->layouts), "continue")) {
		    plug->dialog = DIALOG_NONE;
		}
		if (GuiButton(layout_stack_slot(&plug->layouts), "save")) {
		    printf("saving %s\n", plug->paths[plug->level_selected]);
		    plug_save(plug, plug->paths[plug->level_selected]);
		    reset_paths(plug);
		    plug->paths = xml_get_filepaths("levels");
		    plug->state = START_MENU;
		    plug->dialog = DIALOG_NONE;
		}
		LayoutDrawing(&plug->layouts, LO_HORI, layout_stack_slot(&plug->layouts), 2, gap) {
		    if (GuiTextBox(layout_stack_slot(&plug->layouts), text_box, 10, true) && plug->dialog == DIALOG_NONE) {
			if (strlen(text_box)) {
			    for (size_t i = 0; i < strlen(text_box); i++) {
				if (!isascii(text_box[i]) || isspace(text_box[i])) {
				    text_box[i] = '_';
				}
			    }
			    printf("textbox: saving levels/%s.xml\n", text_box);
			    text_box[0] = '\0';
			}
		    }
		    if (GuiButton(layout_stack_slot(&plug->layouts), "new save")) {
			if (strlen(text_box)) {
			    for (size_t i = 0; i < strlen(text_box); i++) {
				if (!isascii(text_box[i]) || isspace(text_box[i])) {
				    text_box[i] = '_';
				}
			    }
			    //printf("button: saving levels/%s.xml\n", text_box);
			    char file_path[25];
			    file_path[0] = '\0';
			    sprintf(file_path, "levels/%s.xml", text_box);
			    printf("%s\n", file_path);
			    text_box[0] = '\0';
			    plug_save(plug, file_path);
			    reset_paths(plug);
			    plug->paths = xml_get_filepaths("levels");
			    plug->state = START_MENU;
			    plug->dialog = DIALOG_NONE;
			}
		    }
		}
	    }
	}
    }
}

static void draw_level_game(Plug *plug, Texture2D background, Texture2D tileset, Texture2D player, Texture2D player_flop) {
    Drawing {
	ClearBackground(BLACK);
	Mode2D(plug->camera) {
	    DrawTextureEx(background, (Vector2){0, 0}, 0, 6.67, WHITE);
	    for (int y = 0; y < TILESY; y++) {
		for (int x = 0; x < TILESX; x++) {
		    if (plug->tilemap[y][x]) {
			draw_tilemap(plug->tilemap[y][x], x, y, tileset);
		    }
		    //DrawRectangleLines(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, x == plug->mouse_tile_pos.x && y == plug->mouse_tile_pos.y ? RED : Fade(BLACK, 0.3f));
		}
	    }
	    draw_player(plug, player, player_flop);
	}

	if (array_size(plug->players) == 0) {
	    plug->dialog = DIALOG_GAME;
	}

	if (plug->dialog == DIALOG_GAME) {
	    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.9f));
	    Rectangle rec = {
		.x = GetScreenWidth()/2 - 400/2,
		.y = GetScreenHeight()/2 - 300/2,
		.width = 400,
		.height = 300,
	    };
	    float gap = 10.0f;
	    
	    GuiDrawRectangle(rec, 2, WHITE, GetColor(0xcde0e766));
	    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
	    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	    
	    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 3, gap) {
		char result[20];
		sprintf(result, "players exit: %d/%d", plug->score, plug->attempt);
		GuiLabel(layout_stack_slot(&plug->layouts), result);

		if (plug->score == 0) {
		    GuiLabel(layout_stack_slot(&plug->layouts), "bad !");
		} else if (plug->score == plug->attempt) {
		    GuiLabel(layout_stack_slot(&plug->layouts), "perfect !");
		} else {
		    GuiLabel(layout_stack_slot(&plug->layouts), "good !");
		}

		if (GuiButton(layout_stack_slot(&plug->layouts), "quit")) {
		    plug->score = 0;
		    plug->coins = 0;
		    plug->max_coins = 0;
		    plug->bricks = 0;
		    plug->state = START_MENU;
		    plug->dialog = DIALOG_NONE;
		}

	    }
	}
	if (plug->dialog == DIALOG_PAUSE) {
	    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.9f));
	    Rectangle rec = {
		.x = GetScreenWidth()/2 - 400/2,
		.y = GetScreenHeight()/2 - 300/2,
		.width = 400,
		.height = 300,
	    };
	    float gap = 10.0f;

	    GuiDrawRectangle(rec, 2, WHITE, GetColor(0xcde0e766));
	    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
	    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

	    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 2, gap) {
		GuiLabel(layout_stack_slot(&plug->layouts), "pause");
		LayoutDrawing(&plug->layouts, LO_HORI, layout_stack_slot(&plug->layouts), 2, gap) {
		    if (GuiButton(layout_stack_slot(&plug->layouts), "continue")) {
			plug->dialog = DIALOG_NONE;
		    }
		    if (GuiButton(layout_stack_slot(&plug->layouts), "quit")) {
			plug->score = 0;
			plug->coins = 0;
			plug->max_coins = 0;
			plug->bricks = 0;
			plug->state = START_MENU;
			plug->dialog = DIALOG_NONE;
		    }
		}
	    }
	}
    }
}

void plug_render(Plug *plug, Texture2D background, Texture2D tileset, Texture2D player, Texture player_flop) {
    switch (plug->state) {
    case START_MENU: return draw_level_select(plug);
    case EDITOR: return draw_level_editor(plug, background, tileset, player, player_flop);
    case GAME: return draw_level_game(plug, background, tileset, player, player_flop);
    default: break;
    }
    //if (plug->state == START_MENU) {
    //	//draw_main_menu(plug, menu);
    //	draw_select_level(plug);
    //} else {
    //	Drawing {
    //	    ClearBackground(BLACK);
    //	    Mode2D(plug->camera) {
    //		DrawTextureEx(background, (Vector2){0, 0}, 0, 6.67, WHITE);
    //		for (int y = 0; y < TILESY; y++) {
    //		    for (int x = 0; x < TILESX; x++) {
    //			if (plug->tilemap[y][x]) {
    //			    draw_tilemap(plug->tilemap[y][x], x, y, demoTile);
    //			}
    //			DrawRectangleLines(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, x == plug->mouse_tile_pos.x && y == plug->mouse_tile_pos.y ? RED : Fade(BLACK, 0.3f));
    //		    }
    //		}
    //		draw_player(plug, player);
    //	    }
    //	    draw_metal_box(plug, menu, demoTile, player);
    //	
    //	    DrawText(TextFormat("Mouse coordinate on tile: [%d,%d]", plug->mouse_tile_pos.x, plug->mouse_tile_pos.y), 10, 10, 20, BLACK);
    //	    DrawText(TextFormat("Mouse coordinate: [%1.f,%1.f]", plug->mouse_position.x, plug->mouse_position.y), 10, 35, 20, BLACK);
    //	    DrawText(TextFormat("Tile value: %d", plug->tilemap[plug->mouse_tile_pos.y][plug->mouse_tile_pos.x]), 10, 60, 20, BLACK);

    //	    if (plug->state == QUIT_MENU) draw_dialog_glass(plug, menu);
    //	}
    //}
}

void plug_free(Plug *plug) {
    for (size_t i = 0; i < array_size(plug->paths); i++) {
	free(plug->paths[i]);
    }
    array_free(plug->paths);
    array_free(plug->players);
    array_free(plug->layouts);
}

