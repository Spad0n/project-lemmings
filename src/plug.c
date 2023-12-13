/* -*- compile-command: "make -C .. libplug" -*- */
#include "macro.h"
#include "plug.h"
#include "raylib.h"
#include <stddef.h>
#include <ctype.h>

#include "ui.h"
#include "array.h"

#define DOOR_VERTICAL_SIZE 54

#define TEXTURE_GRASS (Rectangle){0, 0, 36, 36}
#define TEXTURE_COIN (Rectangle){396, 252, 36, 36}
#define TEXTURE_KEY (Rectangle){252, 36, 36, 36}
#define TEXTURE_SPIKE (Rectangle){288, 108, 36, 36}
#define TEXTURE_BIG_BRICK (Rectangle){252, 0, 36, 36}
#define TEXTURE_SMALL_BRICK (Rectangle){288, 0, 36, 36}
#define TEXTURE_DOOR (Rectangle){360, 198, 36, 54}

#define TEXTURE_PLAYER (Rectangle){0, 0, 48, 48}

static Item set_item(Key key, Value val) {
    Item item = {
	.value = val,
	.key = key,
    };
    return item;
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

    plug->state = EDITOR;
    
    plug->players = array_create_init(2, sizeof(Entity));
    plug->layouts = array_create_init(4, sizeof(Layout));

    plug->paths = xml_get_filepaths("levels");

    plug->window_should_close = false;

    XMLDocument doc = {0};
    if (xml_load(&doc, "levels/level.xml")) {
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
    	    }
    	}
	array_free(players);
    	xml_doc_free(&doc);
    } else {
    	for (size_t y = 0; y < TILESY; y++) {
    	    for (size_t x = 0; x < TILESX; x++) {
    		plug->tilemap[y][x] = BLOCK_EMPTY;
    	    }
    	}
    }
}

void plug_update(Plug *plug) {

    // Drag mouse mouvement
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    	Vector2 delta = GetMouseDelta();
    	delta = Vector2Scale(delta, -1.0f/plug->camera.zoom);
    	plug->camera.target = Vector2Add(plug->camera.target, delta);
    }

    if (IsKeyPressed(KEY_A)) plug->state = QUIT_MENU;

    if (IsKeyPressed(KEY_T)) plug->show = plug->show ? false : true;

    //if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), plug->rec) && plug->state == EDITOR) {
    //	BlockID tiletype[] = {BLOCK_MIDDLE, BLOCK_COIN, BLOCK_KEY, BLOCK_LEVER, BLOCK_S_BRICK, BLOCK_B_BRICK, BLOCK_DOOR};
    //	for (size_t i = 0; i < 7; i++) {
    //	    float j = 3 * i + 1;
    //	    Rectangle obj = {
    //		.x = plug->rec.x + MAP_TILE_SIZE/1.2f,
    //		.y = plug->rec.y + (MAP_TILE_SIZE/2 * j),
    //		.width = (MAP_TILE_SIZE * 1.3f),
    //		.height = (MAP_TILE_SIZE * 1.3f),
    //	    };
    //	    if (CheckCollisionPointRec(GetMousePosition(), obj)) {
    //		plug->block_selected = tiletype[i];
    //	    }
    //	}
    //}

    // toggle block selection
    //if (plug->show) {
    //	if (plug->rec.x > SCREEN_WIDTH - (3 * MAP_TILE_SIZE)) {
    //	    plug->rec.x -= 500 * dt;
    //	} else {
    //	    plug->rec.x = SCREEN_WIDTH - (3 * MAP_TILE_SIZE);
    //	}
    //} else {
    //	if (plug->rec.x < SCREEN_WIDTH) {
    //	    plug->rec.x += 500 * dt;
    //	} else {
    //	    plug->rec.x = SCREEN_WIDTH;
    //	}
    //}

    // clean all on screen
    if (IsKeyPressed(KEY_D)) {
	for (size_t y = 0; y < TILESY; y++) {
	    for (size_t x = 0; x < TILESX; x++) {
		plug->tilemap[y][x] = BLOCK_EMPTY;
	    }
	}
	array_clear(plug->players);
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

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX && plug->mouse_tile_pos.y < TILESY && plug->state == EDITOR) {
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
		array_push(plug->players, entity_init(posX * MAP_TILE_SIZE, posY * MAP_TILE_SIZE));
		break;
	    default:
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
    case BLOCK_SPIKE: return DrawTextureRec(demoTile, TEXTURE_SPIKE, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_S_BRICK: return DrawTextureRec(demoTile, TEXTURE_SMALL_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_B_BRICK: return DrawTextureRec(demoTile, TEXTURE_BIG_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_DOOR: return DrawTextureRec(demoTile, TEXTURE_DOOR, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE - 18}, WHITE); 
    default: return;
    }
}

void plug_temp(Plug *plug) {
    entity_update(plug);
}

static void draw_player(Plug *plug, Texture2D player_texture) {
    //DrawTextureRec(player, TEXTURE_PLAYER, plug->player.player_position, WHITE);
    for (size_t i = 0; i < array_size(plug->players); i++) {
	//Entity player = array_last(plug->player);
	Entity player = plug->players[i];
	DrawTextureRec(player_texture, TEXTURE_PLAYER, (Vector2){player.rect.x - 12, player.rect.y - 12}, WHITE);
	DrawRectangleRec(player.rect, Fade(RED, 0.5f));
    }

    //DrawRectangle(plug->player.rect.x + plug->player.rect.width, plug->player.rect.y, 2, 2, BLUE);
}

static void draw_metal_box(Plug *plug, Texture menu, Texture demoTile, Texture player) {
    float dt = GetFrameTime();
    static Rectangle rec = {
    	.x = SCREEN_WIDTH,
    	.y = 0,
    	.width = 3 * UI_TILE_SIZE,
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
    	if (rec.x > SCREEN_WIDTH - (3 * UI_TILE_SIZE)) {
    	    rec.x -= 500 * dt;
    	} else {
    	    rec.x = SCREEN_WIDTH - (3 * UI_TILE_SIZE);
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
    ui_draw_dialog(rec, menu, Metal, WHITE);
    rec.width = UI_TILE_SIZE * (int)(rec.width / UI_TILE_SIZE);
    rec.height = UI_TILE_SIZE * (int)(rec.height / UI_TILE_SIZE);
    UILayoutDrawing(&plug->layouts, LO_VERT, ui_make_layout_rec(rec.x, rec.y, rec.width, rec.height), 7, 0) {
	for (size_t i = 0; i < 7; i++) {
	    Rectangle tile_position = ui_layout_stack_slot(&plug->layouts);
	    tile_position.y -= 5;

	    size_t tile_width = UI_TILE_SIZE * (int)(tile_position.width / UI_TILE_SIZE);
	    //tile_position.x = tile_position.x + (tile_position.width - tile_width) / 2;
	    tile_position.width = tile_width;
	    size_t tile_height = UI_TILE_SIZE * (int)(tile_position.height / UI_TILE_SIZE);
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
		ui_widget_item(demoTile, tile_position, recs[i]);
	    } else {
		ui_widget_item(player, tile_position, recs[i]);
	    }
	}
    }
    //printf("block_selected: %d\n", plug->block_selected);
    //draw_selection(rec, demoTile);
}

static void draw_dialog_glass(Plug *plug, Texture2D dialog) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.9f));
    Rectangle rec = {
	.x = GetScreenWidth()/2 - 400/2,
	.y = GetScreenHeight()/2 - 235/2,
	.width = UI_TILE_SIZE * (int)(400 / UI_TILE_SIZE),
	.height = UI_TILE_SIZE * (int)(235 / UI_TILE_SIZE),
    };
    float gap = 10.0f;
    ui_draw_dialog(rec, dialog, Glass, WHITE);
    Rectangle button_yes = {0};
    Rectangle button_no = {0};
    UILayoutDrawing(&plug->layouts, LO_VERT, ui_make_layout_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 2, gap) {
	ui_widget_text(ui_layout_stack_slot(&plug->layouts), "do you want to quit ?", WHITE);
	UILayoutDrawing(&plug->layouts, LO_HORI, ui_layout_stack_slot(&plug->layouts), 2, gap) {
	    button_yes = ui_layout_stack_slot(&plug->layouts);
	    button_yes.x += gap;
	    button_yes.y += gap;
	    button_yes.width -= gap * 2;
	    button_yes.height -= gap * 2;

	    button_no = ui_layout_stack_slot(&plug->layouts);
	    button_no.x += gap;
	    button_no.y += gap;
	    button_no.width -= gap * 2;
	    button_no.height -= gap * 2;
	    
	    ui_widget_button(dialog, button_yes, "yes");
	    ui_widget_button(dialog, button_no, "no");
	}
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), button_yes)) {
	plug->window_should_close = true;
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), button_no)) {
	plug->state = EDITOR;
    }
}

static void draw_main_menu(Plug *plug, Texture2D dialog) {
    Rectangle rec = {
	.x = GetScreenWidth()/2 - 700/2,
	.y = GetScreenHeight()/2 - 420/2,
	.width = UI_TILE_SIZE * (int)(700 / UI_TILE_SIZE),
	.height = UI_TILE_SIZE * (int)(420 / UI_TILE_SIZE),
    };

    float gap = 10.0f;

    //size_t size_paths = array_size(plug->paths);

    Drawing {
	ClearBackground(BLACK);
	ui_draw_dialog(rec, dialog, Glass, WHITE);
	UILayoutDrawing(&plug->layouts, LO_VERT, ui_make_layout_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 3, gap) {
	    ui_widget_text(ui_layout_stack_slot(&plug->layouts), "choose a level", WHITE);
	    UILayoutDrawing(&plug->layouts, LO_HORI, ui_layout_stack_slot(&plug->layouts), array_size(plug->paths), gap) {
		for (size_t i = 0; i < array_size(plug->paths); i++) {
		    ui_widget_button(dialog, ui_layout_stack_slot(&plug->layouts), plug->paths[i]);
		}
	    }
	}
    }
}

void plug_render(Plug *plug, Texture2D background, Texture2D demoTile, Texture2D player, Texture2D menu) {
    if (plug->state == START_MENU) {
	draw_main_menu(plug, menu);
    } else {
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

			//Vector2 player_center = {
			//	.x = (plug->player.rect.x * 2 + plug->player.rect.width) / 2,
			//	.y = (plug->player.rect.y * 2 + plug->player.rect.height) / 2,
			//};

			//Rectangle block = {
			//	.x = x * MAP_TILE_SIZE,
			//	.y = y * MAP_TILE_SIZE,
			//	.width = MAP_TILE_SIZE,
			//	.height = MAP_TILE_SIZE,
			//};
			//if (CheckCollisionPointRec(player_center, block)) {
			//	DrawRectangleLines(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, BLUE);

			//	// left
			//	DrawRectangleLines(x * MAP_TILE_SIZE - MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, BLUE);

			//	// right
			//	DrawRectangleLines(x * MAP_TILE_SIZE + MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, BLUE);
			//}
		    }
		}
		draw_player(plug, player);
	    }
	    draw_metal_box(plug, menu, demoTile, player);
	

	    //Tile2D player_center = {
	    //    .x = (plug->player->rect.x * 2 + plug->player->rect.width) / 2,
	    //    .y = (plug->player->rect.y * 2 + plug->player->rect.height) / 2,
	    //};
	    //player_center.x /= MAP_TILE_SIZE;
	    //player_center.y /= MAP_TILE_SIZE;

	    DrawText(TextFormat("Mouse coordinate on tile: [%d,%d]", plug->mouse_tile_pos.x, plug->mouse_tile_pos.y), 10, 10, 20, BLACK);
	    DrawText(TextFormat("Mouse coordinate: [%1.f,%1.f]", plug->mouse_position.x, plug->mouse_position.y), 10, 35, 20, BLACK);
	    DrawText(TextFormat("Tile value: %d", plug->tilemap[plug->mouse_tile_pos.y][plug->mouse_tile_pos.x]), 10, 60, 20, BLACK);
	    //DrawText(TextFormat("player on tile: [%d, %d]", player_center.x, player_center.y), 10, 85, 20, BLACK);

	    if (plug->state == QUIT_MENU) draw_dialog_glass(plug, menu);
	}
    }
}

void plug_free(Plug *plug) {
    for (size_t i = 0; i < array_size(plug->paths); i++) {
	free(plug->paths[i]);
    }
    array_free(plug->paths);
    array_free(plug->players);
    array_free(plug->layouts);
}

void plug_save(Plug *plug) {
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
    xml_attrib_add(csv, "width", "22");
    xml_attrib_add(csv, "height", "13");

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

    xml_doc_write(&doc, "levels/level.xml", 2);

    xml_doc_free(&doc);
}
