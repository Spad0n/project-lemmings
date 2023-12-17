/* -*- compile-command: "make -C .. libplug" -*- */
#include "macro.h"
#include "plug.h"
#include "raylib.h"
#include <stddef.h>
#include <ctype.h>

// utilisation de la librairie raygui pour les widgets
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_NO_ICONS
#include "raygui.h"

#include "layout.h"
#include "array.h"

// liste des coordonnés des textures dans un spritesheet
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

/**
 * @brief Ouvre et initialise un niveau de jeu à partir d'un fichier XML.
 *
 * Cette fonction charge les données du niveau à partir d'un fichier XML, 
 * incluant les positions des joueurs, les informations de la carte de tuiles,
 * et d'autres détails spécifiques au jeu.
 *
 * @param plug Un pointeur vers l'état du jeu (structure Plug).
 * @param file_path Le chemin du fichier XML contenant les données du niveau.
 */
static void open_level(Plug *plug, const char *file_path) {
    // Initialise une structure de document XML.
    XMLDocument doc = {0};

    // Tente de charger les données XML à partir du fichier spécifié.
    if (xml_load(&doc, file_path)) {
	// Trouve le noeud "csv" dans le document XML.
    	XMLNode *csv = xml_node_find_tag(doc.root, "csv");

	// Trouve tous les noeuds "player" dans le document XML et initialise les entités des joueurs.
	Array_XMLNode players = xml_node_find_tags(doc.root, "player");
	for (size_t i = 0; i < array_size(players); i++) {
	    char *char_x = xml_attrib_get_value(players[i], "x");
	    char *char_y = xml_attrib_get_value(players[i], "y");
	    int x = atoi(char_x);
	    int y = atoi(char_y);

	    // Initialise les entités des joueurs et les ajoute au tableau de joueurs de plug.
	    array_push(plug->players, entity_init(MAP_TILE_SIZE * x, MAP_TILE_SIZE * y));
	}

	// Initialise la carte de tuiles à partir du noeud "csv" dans le document XML.
    	int index = 0;
    	for (size_t y = 0; y < TILESY; y++) {
    	    for (size_t x = 0; x < TILESX; x++) {
		// Extrait les valeurs numériques du noeud "csv".
    		char number[4];
    		int num_index = 0;
    		while (isdigit(csv->inner_text[index])) {
    		    number[num_index++] = csv->inner_text[index++];
    		}
		number[num_index] = '\0';
    		index++;

		// Convertit et attribue la valeur numérique à la carte de tuiles.
		plug->tilemap[y][x] = atoi(number);

		// Si la tuile est une pièce, incrémenter le compteur max_coins.
		if (plug->tilemap[y][x] == BLOCK_COIN) {
		    plug->max_coins += 1;
		}
    	    }
    	}
	// Definit le nombre de joueur qui doit aller à la sortie du niveau
	plug->goal = array_size(plug->players);

	// Libère le tableau des nœuds de joueur et le document XML.
	array_free(players);
    	xml_doc_free(&doc);
    } else {
	// Affiche un message d'erreur si le niveau ne peut pas être chargé.
	fprintf(stderr, "failed to open the level: %s\n", file_path);

	// Initialise la carte de tuiles à BLOCK_EMPTY en cas d'échec.
    	for (size_t y = 0; y < TILESY; y++) {
    	    for (size_t x = 0; x < TILESX; x++) {
    		plug->tilemap[y][x] = BLOCK_EMPTY;
    	    }
    	}
	plug->state = EDITOR;
    }
}

/**
 * @brief Initialise la structure Plug utilisée pour le hotreload.
 *
 * Cette fonction initialise la structure Plug utilisée pour le hotreload,
 * définissant divers paramètres tels que le zoom de la caméra, la position
 * de la souris, l'état du curseur, l'élément sélectionné, etc.
 *
 * @param plug Un pointeur vers la structure Plug à initialiser.
 */
void plug_init(Plug *plug) {
    // Initialise le niveau de zoom de la caméra.
    plug->camera.zoom = 1.0f;
    
    // Initialise la position de la souris et sa position en coordonnées de tuiles.
    plug->mouse_position = (Vector2){0.0f, 0.0f};
    plug->mouse_tile_pos = (Tile2D){0, 0};
    
    // Initialise l'état de l'outil gomme (eraser).
    plug->eraser = false;

    // Initialise l'élément sélectionné avec une valeur par défaut de bloc.
    Value val = {0};
    val.block_id = BLOCK_MIDDLE;
    plug->item_selected = set_item(BLOCK, val);

    // Initialise l'état du jeu et du dialogue.
    plug->state = START_MENU;
    plug->dialog = DIALOG_NONE;
    
    // Initialise le tableau de joueurs et de mises en page.
    plug->players = array_create_init(2, sizeof(Entity));
    plug->layouts = array_create_init(4, sizeof(Layout));

    // Initialise les chemins des fichiers XML de niveaux.
    plug->paths = xml_get_filepaths("levels");

    // Initialise la variable indiquant si la fenêtre doit être fermée.
    plug->window_should_close = false;

    // Initialise la carte de tuiles avec des blocs vides.
    for (size_t y = 0; y < TILESY; y++) {
        for (size_t x = 0; x < TILESX; x++) {
	    plug->tilemap[y][x] = BLOCK_EMPTY;
        }
    }

    // Initialise la page à 0.
    plug->page = 0;
}

/**
 * @brief Met à jour la structure Plug utilisée pour le hotreload.
 *
 * Cette fonction met à jour la structure Plug utilisée pour le hotreload,
 * traitant différents événements tels que la mise à jour des entités,
 * la gestion de la souris, et les actions en fonction de l'état du jeu.
 *
 * @param plug Un pointeur vers la structure Plug à mettre à jour.
 */
void plug_update(Plug *plug) {
    // Met à jour les entités sauf en cas de dialogue en cours.
    if (plug->dialog == DIALOG_NONE) entity_update(plug);

    // Met à jour la position de la souris et sa position en coordonnées de tuiles.
    plug->mouse_position = GetMousePosition();
    plug->mouse_tile_pos.x = (plug->mouse_position.x / plug->camera.zoom + plug->camera.target.x - (plug->camera.offset.x / plug->camera.zoom)) / MAP_TILE_SIZE;
    plug->mouse_tile_pos.y = (plug->mouse_position.y / plug->camera.zoom + plug->camera.target.y - (plug->camera.offset.y / plug->camera.zoom)) / MAP_TILE_SIZE;

    // Gère les événements en fonction de l'état du jeu.
    switch (plug->state) {
    case EDITOR:
	// Affiche le dialogue de l'éditeur.
	if (IsKeyPressed(KEY_A)) plug->dialog = DIALOG_EDITOR;

	// Affiche/masque l'interface de l'éditeur.
	if (IsKeyPressed(KEY_T) && plug->dialog == DIALOG_NONE) plug->show = plug->show ? false : true;

	// Réinitialise la carte de tuiles et les joueurs.
	if (IsKeyPressed(KEY_D) && plug->state == EDITOR && plug->dialog == DIALOG_NONE) {
	    for (size_t y = 0; y < TILESY; y++) {
		for (size_t x = 0; x < TILESX; x++) {
		    plug->tilemap[y][x] = BLOCK_EMPTY;
		}
	    }
	    array_clear(plug->players);
	}

	// Active/désactive l'outil gomme.
	if (IsKeyPressed(KEY_E)) plug->eraser = plug->eraser ? false : true;

	// Ajoute un joueur lors du clic gauche sur une tuile vide.
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX - (plug->show ? 3 : 0) && plug->item_selected.key == ENTITY) {
	    int posX = plug->mouse_tile_pos.x;
	    int posY = plug->mouse_tile_pos.y;
	    array_push(plug->players, entity_init(posX * MAP_TILE_SIZE, posY * MAP_TILE_SIZE));
	}

	// Modifie la carte de tuiles en fonction du clic gauche de la souris.
	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && plug->mouse_tile_pos.x < TILESX - (plug->show ? 3 : 0) && plug->mouse_tile_pos.y < TILESY && plug->dialog == DIALOG_NONE) {
	    int posX = plug->mouse_tile_pos.x;
	    int posY = plug->mouse_tile_pos.y;

	    Value val = plug->item_selected.value;
	    Key key = plug->item_selected.key;

	    // verifie si la gomme est activé
	    if (!plug->eraser) {
		if (key == BLOCK) {
		    plug->tilemap[posY][posX] = val.block_id;
		} else {
		    plug->tilemap[posY][posX] = BLOCK_EMPTY;
		}
	    } else {
		plug->tilemap[posY][posX] = BLOCK_EMPTY;
		for (size_t i = 0; i < array_size(plug->players); i++) {
		    if (CheckCollisionPointRec(GetMousePosition(), plug->players[i].rect)) {
			array_pop_at(plug->players, i);
		    }
		}
	    }

	    // modifier la texture des blocks
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
	// Met le jeu en pause lors de la pression de la touche A.
	if (IsKeyPressed(KEY_A)) plug->dialog = DIALOG_PAUSE;

	// Active/désactive l'outil gomme.
	if (IsKeyPressed(KEY_E)) {
	    plug->eraser = plug->eraser ? false : true;
	}

	// Définit l'état des joueurs lors du clic gauche sur eux.
	for (size_t i = 0; i < array_size(plug->players); i++) {
	    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(plug->mouse_position, plug->players[i].rect)) {
		plug->players[i].state = MOVE_RIGHT;
	    }
	}

	// Modifie la carte de tuiles en fonction du clic gauche de la souris.
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
	break;
    default: break;
    }
}

/**
 * @brief Dessine un élément de la carte de tuiles en fonction du type de bloc.
 *
 * Cette fonction dessine un élément de la carte de tuiles en fonction du type de bloc
 * à la position spécifiée (x, y), en utilisant une texture donnée.
 *
 * @param tile Le type de bloc à dessiner.
 * @param x La position en coordonnée X sur la carte de tuiles.
 * @param y La position en coordonnée Y sur la carte de tuiles.
 * @param tileset La texture utilisée pour dessiner les blocs.
 */
static void draw_tilemap(int tile, size_t x, size_t y, Texture2D tileset) {
    switch (tile) {
    case BLOCK_MIDDLE: return DrawTextureRec(tileset, (Rectangle){0, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){0, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_TOP: return DrawTextureRec(tileset, (Rectangle){0, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_LEFT: return DrawTextureRec(tileset, (Rectangle){108, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT: return DrawTextureRec(tileset, (Rectangle){36, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){0, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE);
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT: return DrawTextureRec(tileset, (Rectangle){72, 0, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){108, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_TOP: return DrawTextureRec(tileset, (Rectangle){108, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){36, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT | BLOCK_TOP: return DrawTextureRec(tileset, (Rectangle){36, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT | BLOCK_TOP: return DrawTextureRec(tileset, (Rectangle){72, 252, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){72, 36, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){108, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_RIGHT | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){36, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_MIDDLE | BLOCK_LEFT | BLOCK_RIGHT | BLOCK_TOP | BLOCK_BOTTOM: return DrawTextureRec(tileset, (Rectangle){72, 216, 36, 36}, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 

    // dessine les block interactif avec le joueur
    case BLOCK_COIN: return DrawTextureRec(tileset, TEXTURE_COIN, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_SPIKE: return DrawTextureRec(tileset, TEXTURE_SPIKE, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_S_BRICK: return DrawTextureRec(tileset, TEXTURE_SMALL_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_B_BRICK: return DrawTextureRec(tileset, TEXTURE_BIG_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    case BLOCK_DOOR: return DrawTextureRec(tileset, TEXTURE_DOOR, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE - 18}, WHITE); 
    case BLOCK_BRICK: return DrawTextureRec(tileset, TEXTURE_BRICK, (Vector2){x * MAP_TILE_SIZE, y * MAP_TILE_SIZE}, WHITE); 
    default: return;
    }
}

/**
 * @brief Dessine les joueurs sur l'écran en utilisant des textures spécifiques.
 *
 * Cette fonction parcourt le tableau des joueurs dans la structure Plug
 * et dessine chaque joueur sur l'écran en fonction de son état de déplacement,
 * utilisant les textures spécifiées pour le mouvement vers la gauche et le mouvement normal.
 *
 * @param plug Un pointeur vers la structure Plug contenant les joueurs à dessiner.
 * @param player_texture La texture utilisée pour dessiner le joueur en mouvement normal.
 * @param player_flop La texture utilisée pour dessiner le joueur en mouvement vers la gauche.
 */
static void draw_player(Plug *plug, Texture2D player_texture, Texture2D player_flop) {
    for (size_t i = 0; i < array_size(plug->players); i++) {
	Entity player = plug->players[i];
	if (player.state == MOVE_LEFT) {
	    // Dessine le joueur avec la texture de mouvement vers la gauche.
	    DrawTextureRec(player_texture, TEXTURE_PLAYER, (Vector2){player.rect.x - 12, player.rect.y - 12}, WHITE);
	} else {
	    // Dessine le joueur avec la texture de mouvement vers la droite.
	    DrawTextureRec(player_flop, TEXTURE_PLAYER_FLOP, (Vector2){player.rect.x - 12, player.rect.y - 12}, WHITE);
	}
    }
}

/**
 * @brief Dessine la boîte d'items dans l'éditeur de niveau.
 *
 * Cette fonction dessine la boîte d'items dans l'éditeur de niveau,
 * permettant à l'utilisateur de sélectionner différents blocs et entités
 * pour les placer sur la carte de tuiles.
 *
 * @param plug Un pointeur vers la structure Plug contenant les informations de l'éditeur.
 * @param tileset La texture utilisée pour dessiner les blocs dans la boîte d'items.
 * @param player La texture utilisée pour dessiner les entités (joueur) dans la boîte d'items.
 */
static void draw_items_box(Plug *plug, Texture tileset, Texture player) {
    float dt = GetFrameTime();
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

    // Animation de transition de la boîte d'items.
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

    int tiletype[] = {
    	BLOCK_MIDDLE,
    	BLOCK_COIN,
    	BLOCK_SPIKE,
    	BLOCK_S_BRICK,
    	BLOCK_B_BRICK,
    	BLOCK_DOOR,
    };

    // Dessine le fond de la boîte d'items.
    GuiDrawRectangle(rec, 2, BLACK, GetColor(0xd6dde7ff));
    rec.width = MAP_TILE_SIZE * (int)(rec.width / MAP_TILE_SIZE);
    rec.height = MAP_TILE_SIZE * (int)(rec.height / MAP_TILE_SIZE);

    // Dessine les items dans la boîte d'items en utilisant une disposition verticale.
    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x, rec.y, rec.width, rec.height), 7, 0) {
	for (size_t i = 0; i < 7; i++) {
	    Rectangle tile_position = layout_stack_slot(&plug->layouts);
	    tile_position.y += 5;

	    // Ajuste les dimensions des tuiles pour s'aligner avec la grille.
	    size_t tile_width = MAP_TILE_SIZE * (int)(tile_position.width / MAP_TILE_SIZE);
	    tile_position.width = tile_width;
	    size_t tile_height = MAP_TILE_SIZE * (int)(tile_position.height / MAP_TILE_SIZE);
	    tile_position.height = tile_height;
	    
	    // Centre la tuile dans l'emplacement de la boîte d'items.
	    tile_position.x = ceilf(tile_position.x + (tile_position.width - recs[i].width) / 2);
	    tile_position.y = ceilf(tile_position.y + (tile_position.height - recs[i].height) / 2);
	    tile_position.width = recs[i].width;
	    tile_position.height = recs[i].height;

	    // Vérifie si la tuile est cliquée et met à jour l'item sélectionné.
	    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), tile_position)) {
		if (i < 6) {
		    plug->item_selected.value.block_id = tiletype[i];
		    plug->item_selected.key = BLOCK;
		} else {
		    plug->item_selected.value.entity = (Tile2D){0, 0};
		    plug->item_selected.key = ENTITY;
		}
	    }

	    // Montre visuellement l'objet sélectionné dans la boîte d'items.
	    if (i < 6) {
		if (plug->item_selected.key == BLOCK && plug->item_selected.value.block_id == tiletype[i]) {
		    layout_item(true, tileset, tile_position, recs[i]);
		} else {
		    layout_item(false, tileset, tile_position, recs[i]);
		}
	    } else {
		if (plug->item_selected.key == ENTITY) {
		    layout_item(true, player, tile_position, recs[i]);
		} else {
		    layout_item(false, player, tile_position, recs[i]);
		}
	    }
	}
    }
}

/**
 * @brief Dessine l'écran de sélection de niveau dans le jeu.
 *
 * Cette fonction dessine l'écran de sélection de niveau, permettant
 * à l'utilisateur de choisir un niveau parmi ceux disponibles.
 *
 * @param plug Un pointeur vers la structure Plug contenant les informations de l'éditeur.
 */
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

	// Dessine la disposition des éléments dans la fenêtre de sélection de niveau.
	LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 3, gap) {

	    // Zone supérieure de la fenêtre.
	    Rectangle top_layout = layout_stack_slot(&plug->layouts);
	    GuiLabel(top_layout, "choose a level");

	    // Bouton "New" pour créer un nouveau niveau.
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
	    
	    // Bouton "Quit" pour quitter l'application.
	    Rectangle top_left = {
		.x = top_layout.x,
		.y = top_layout.y,
		.width = top_layout.height/2,
		.height = top_layout.height/2,
	    };
	    if (GuiButton(top_left, "Quit")) {
		plug->window_should_close = true;
	    }

	    // Zone d'affichage des niveaux disponibles.
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

	    // Zone pour la pagination des niveaux.
	    Rectangle page_recs = layout_stack_slot(&plug->layouts);
	    page_recs.y += page_recs.height / 2;
	    page_recs.height -= page_recs.height / 2;

	    // Boutons précédent et suivant pour changer de page.
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

	    // Affiche le numéro de page actuel.
	    GuiLabel(page_recs, TextFormat("%ld/%d", plug->page + 1, (int)ceil((float)array_size(plug->paths)/9)));

	    // Boutons pour changer de page.
	    if (GuiButton(previous, "<")) {
		if (plug->page > 0) plug->page -= 1;
	    }
	    if (GuiButton(next, ">")) {
		if (9 * (plug->page + 1) < array_size(plug->paths)) plug->page += 1;
	    }
	}
    }
}

/**
 * @brief Sauvegarde l'état actuel du niveau et des joueurs dans un fichier XML.
 *
 * Cette fonction sauvegarde l'état actuel du niveau (configuration des blocs)
 * ainsi que les positions des joueurs dans un fichier XML spécifié par le chemin
 * du fichier.
 *
 * @param plug Un pointeur vers la structure Plug contenant les informations de l'éditeur.
 * @param file_path Le chemin du fichier XML dans lequel sauvegarder les données.
 */
void plug_save(Plug *plug, char *file_path) {
    int string_size = TILESY * (TILESX * 5) + 1;
    char *S = malloc(sizeof(char) * string_size);
    S[0] = '\0';

    // Construit la chaîne de caractères représentant la configuration des blocs.
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

    // Initialise un document XML et ajoute le noeud CSV pour la configuration des blocs.
    XMLDocument doc = xml_doc_init("root");
    XMLNode *csv = xml_node_new(doc.root);
    csv->tag = strdup("csv");
    csv->inner_text = S;

    // Ajoute les informations des joueurs sous forme de noeuds XML.
    for (size_t i = 0; i < array_size(plug->players); i++) {
	XMLNode *player = xml_node_new(doc.root);
	player->tag = strdup("player");
	char char_x[4];
	char_x[0] = '\0';
	char char_y[4];
	char_y[0] = '\0';
	
	// Convertit les positions des joueurs en chaînes de caractères.
	sprintf(char_x, "%d", (int)(plug->players[i].rect.x / MAP_TILE_SIZE));
	sprintf(char_y, "%d", (int)(plug->players[i].rect.y / MAP_TILE_SIZE));

	// Ajoute les attributs x et y au nœud du joueur.
	xml_attrib_add(player, "x", char_x);
	xml_attrib_add(player, "y", char_y);
    }
    
    // Écrit le document XML dans le fichier spécifié par le chemin avec l'indentation de 2 espace.
    xml_doc_write(&doc, file_path, 2);
    
    // Libère la mémoire allouée pour le document XML.
    xml_doc_free(&doc);
}

/**
 * @brief Dessine l'éditeur de niveau avec les éléments graphiques appropriés.
 *
 * Cette fonction dessine l'éditeur de niveau, y compris le fond, la configuration des blocs,
 * les joueurs, la boîte d'outils et la boîte de dialogue éventuelle.
 *
 * @param plug Un pointeur vers la structure Plug contenant les informations de l'éditeur.
 * @param background Texture du fond.
 * @param tileset Texture de l'ensemble de tuiles.
 * @param player Texture du joueur.
 * @param player_flop Texture du joueur (état flop).
 */
static void draw_level_editor(Plug *plug, Texture2D background, Texture2D tileset, Texture2D player, Texture2D player_flop) {
    static char text_box[10];
    Drawing {
	ClearBackground(BLACK);

	// Active le mode 2D avec la caméra spécifiée.
	Mode2D(plug->camera) {

	    // Dessine le fond.
	    DrawTextureEx(background, (Vector2){0, 0}, 0, 6.67, WHITE);

	    // Dessine la configuration des blocs.
	    for (int y = 0; y < TILESY; y++) {
		for (int x = 0; x < TILESX; x++) {
		    if (plug->tilemap[y][x]) {
			draw_tilemap(plug->tilemap[y][x], x, y, tileset);
		    }
		    DrawRectangleLines(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, x == plug->mouse_tile_pos.x && y == plug->mouse_tile_pos.y ? RED : Fade(BLACK, 0.3f));
		}
	    }

	    // Dessine les joueurs.
	    draw_player(plug, player, player_flop);
	}

	// Dessine la boîte d'outils.
	draw_items_box(plug, tileset, player);
	
	// Affiche le mode gomme.
	DrawText(TextFormat("eraser mode: %s", plug->eraser ? "on" : "off"), 10, 10, 20, BLACK);
	
	// Affiche la boîte de dialogue de l'éditeur si elle est active.
	if (plug->dialog == DIALOG_EDITOR) {
	    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.9f));

	    // Définit les coordonnées et la taille de la boîte de dialogue.
	    Rectangle rec = {
		.x = GetScreenWidth()/2 - 400/2,
		.y = GetScreenHeight()/2 - 300/2,
		.width = 400,
		.height = 300,
	    };
	    float gap = 10.0f;
	    
	    // Dessine la boîte de dialogue.
	    GuiDrawRectangle(rec, 2, WHITE, GetColor(0xcde0e766));
	    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
	    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	    
	    // Utilise la mise en page pour organiser les éléments dans la boîte de dialogue.
	    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 5, gap) {
		// Ajoute une étiquette à la boîte de dialogue.
		GuiLabel(layout_stack_slot(&plug->layouts), "do you want to quit ?");
		
		// Ajoute des boutons pour les options de la boîte de dialogue.
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

		// Utilise la mise en page horizontale pour organiser les éléments de manière spécifique.
		LayoutDrawing(&plug->layouts, LO_HORI, layout_stack_slot(&plug->layouts), 2, gap) {
		    // Gère la boîte de texte et le bouton "new save".
		    if (GuiTextBox(layout_stack_slot(&plug->layouts), text_box, 10, true) && plug->dialog == DIALOG_NONE) {
			if (strlen(text_box)) {
			    for (size_t i = 0; i < strlen(text_box); i++) {
				if (!isascii(text_box[i]) || isspace(text_box[i])) {
				    text_box[i] = '_';
				}
			    }
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

/**
 * @brief Dessine l'écran de jeu du niveau avec les éléments graphiques appropriés.
 *
 * Cette fonction dessine l'écran de jeu du niveau, y compris le fond, la configuration des blocs,
 * les joueurs, les informations de jeu et la boîte de dialogue éventuelle.
 *
 * @param plug Un pointeur vers la structure Plug contenant les informations du jeu.
 * @param background Texture du fond.
 * @param tileset Texture de l'ensemble de tuiles.
 * @param player Texture du joueur.
 * @param player_flop Texture du joueur (état flop).
 */
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
		}
	    }
	    draw_player(plug, player, player_flop);
	}

	DrawText(TextFormat("eraser mode: %s", plug->eraser ? "on" : "off"), 10, 10, 20, BLACK);
	DrawText(TextFormat("brick count: %d", plug->bricks), 10, 35, 20, BLACK);

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
		GuiLabel(layout_stack_slot(&plug->layouts), TextFormat("players exit: %d/%d", plug->score_players, plug->goal));

		if (plug->score_players == 0) {
		    GuiLabel(layout_stack_slot(&plug->layouts), "bad !");
		} else if (plug->score_players == plug->goal && plug->max_coins == plug->coins) {
		    GuiLabel(layout_stack_slot(&plug->layouts), "perfect !");
		} else {
		    GuiLabel(layout_stack_slot(&plug->layouts), "good !");
		}

		if (GuiButton(layout_stack_slot(&plug->layouts), "quit")) {
		    plug->score_players = 0;
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

	    LayoutDrawing(&plug->layouts, LO_VERT, layout_make_rec(rec.x + gap, rec.y + gap, rec.width - (gap * 2), rec.height - (gap * 2)), 3, gap) {
		GuiLabel(layout_stack_slot(&plug->layouts), "pause");
		if (GuiButton(layout_stack_slot(&plug->layouts), "edit")) {
		    plug->state = EDITOR;
		    plug->dialog = DIALOG_NONE;
		}
		LayoutDrawing(&plug->layouts, LO_HORI, layout_stack_slot(&plug->layouts), 2, gap) {
		    if (GuiButton(layout_stack_slot(&plug->layouts), "continue")) {
			plug->dialog = DIALOG_NONE;
		    }
		    if (GuiButton(layout_stack_slot(&plug->layouts), "quit")) {
			plug->eraser = false;
			plug->score_players = 0;
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

/**
 * @brief Rendu principal de l'application en fonction de l'état du jeu.
 *
 * Cette fonction sélectionne la fonction de rendu appropriée en fonction de l'état actuel du jeu
 * et appelle la fonction de rendu correspondante.
 *
 * @param plug Un pointeur vers la structure Plug contenant les informations du jeu.
 * @param background Texture du fond.
 * @param tileset Texture de l'ensemble de tuiles.
 * @param player Texture du joueur.
 * @param player_flop Texture du joueur (état flop).
 */
void plug_render(Plug *plug, Texture2D background, Texture2D tileset, Texture2D player, Texture player_flop) {
    switch (plug->state) {
    case START_MENU: return draw_level_select(plug);
    case EDITOR: return draw_level_editor(plug, background, tileset, player, player_flop);
    case GAME: return draw_level_game(plug, background, tileset, player, player_flop);
    default: break;
    }
}

/**
 * @brief Libère les ressources allouées pour la structure Plug.
 *
 * Cette fonction libère toutes les ressources allouées pour la structure Plug,
 * y compris les chemins des fichiers, les joueurs et les mises en page.
 *
 * @param plug Un pointeur vers la structure Plug à libérer.
 */
void plug_free(Plug *plug) {
    for (size_t i = 0; i < array_size(plug->paths); i++) {
	free(plug->paths[i]);
    }
    array_free(plug->paths);
    array_free(plug->players);
    array_free(plug->layouts);
}

