/* -*- compile-command: "make -C .. debug" -*- */
#include "raylib.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#include "plug.h"

#ifdef HOTRELOAD
#include <dlfcn.h>
#endif // HOTRELOAD

const char *libplug_file_name = "libplug.so";
void *libplug = NULL;

#define BASE_PLUG(return_type, name, ...) PLUG(name)
#ifdef HOTRELOAD
#  define PLUG(name) name##_t *name = NULL;
#else
#  define PLUG(name) name##_t name;
#endif // HOTRELOAD
LIST_OF_PLUGS
#undef PLUG
#undef BASE_PLUG

Plug plug = {0};

#ifdef HOTRELOAD
bool reload_libplug(void) {
    if (libplug != NULL) dlclose(libplug);

    libplug = dlopen(libplug_file_name, RTLD_NOW);
    if (libplug == NULL) {
	fprintf(stderr, "ERROR: could not load %s: %s", libplug_file_name, dlerror());
	return false;
    }

#define BASE_PLUG(return_type, name, ...) PLUG(name)
#define PLUG(name) \
    name = dlsym(libplug, #name); \
    if (name == NULL) { \
    	fprintf(stderr, "ERROR: could not find %s in %s: %s\n", #name, libplug_file_name, dlerror()); \
    	return false; \
    }
    LIST_OF_PLUGS
#undef PLUG
#undef BASE_PLUG

    return true;
}
#else
#define reload_libplug() true
#endif // HOTRELOAD

#include "xml.h"
#include <stdlib.h>
#include <ctype.h>

int main(void) {
    if (!reload_libplug()) return 1;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lemmings");

    Texture2D background = LoadTexture("./assets/pixel-platformer/Tilemap/newtest.png");
    Texture2D demoTile = LoadTexture("./assets/pixel-platformer/Tilemap/scaled_packed.png");
    Texture2D menu = LoadTexture("./assets/pixel-platformer-blocks/Tilemap/sand_scaled.png");

    SetTargetFPS(60);

    XMLDocument doc = {0};
    if (xml_load(&doc, "level.xml")) {
    	XMLNode *csv = xml_node_find_tag(doc.root, "csv");
	//printf("csv inner_text: %s\n", csv->inner_text);
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
		plug.tilemap[y][x] = atoi(number);
    	    }
    	}
    	xml_doc_free(&doc);
    } else {
    	for (size_t y = 0; y < TILESY; y++) {
    	    for (size_t x = 0; x < TILESX; x++) {
    		plug.tilemap[y][x] = BLOCK_EMPTY;
    	    }
    	}
    }

    plug_init(&plug);

    while(!WindowShouldClose()) {
	plug_update(&plug);

	if (IsKeyPressed(KEY_Z)) plug.camera.zoom = 1.0f;

	if (IsKeyPressed(KEY_R)) {
	    if (!reload_libplug()) return 1;
	}

	if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
	    int string_size = TILESY * (TILESX * 5) + 1;
	    char *S = malloc(sizeof(char) * string_size);
	    S[0] = '\0';
	    for (size_t y = 0; y < TILESY; y++) {
		for (size_t x = 0; x < TILESX; x++) {
		    char itoa[4];
		    itoa[0] = '\0';
		    sprintf(itoa, "%d", plug.tilemap[y][x]);
		    strncat(S, itoa, 4);

		    if (y != TILESY - 1 || x != TILESX - 1) strcat(S, ",");
		}
		if (y != TILESY - 1) strcat(S, "\n");
	    }
	    doc = xml_doc_init("root");
	    XMLNode *csv = xml_node_new(doc.root);
	    csv->tag = strdup("csv");
	    csv->inner_text = S;
	    xml_attrib_add(csv, "width", "22");
	    xml_attrib_add(csv, "height", "13");

	    xml_doc_write(&doc, "level.xml", 2);

	    xml_doc_free(&doc);
	}

	plug_render(&plug, background, demoTile, menu);
    }

    UnloadTexture(background);
    UnloadTexture(demoTile);
    CloseWindow();

    return 0;
}
