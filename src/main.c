/* -*- compile-command: "make -C .. debug" -*- */
#include "raylib.h"
#include <stdio.h>
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

int main(void) {

    if (!reload_libplug()) return 1;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lemmings");

    Texture2D background = LoadTexture("./assets/pixel-platformer/Tilemap/newtest.png");
    Texture2D demoTile = LoadTexture("./assets/pixel-platformer/Tilemap/scaled_packed.png");
    Texture2D player = LoadTexture("./assets/pixel-platformer/Tilemap/tilemap-characters_scaled.png");
    Texture2D menu = LoadTexture("./assets/ui-pack-space/Spritesheet/uipackSpace_sheet.png");

    SetTargetFPS(60);

    plug_init(&plug);

    while(!plug.window_should_close && !WindowShouldClose()) {
        plug_temp(&plug);
	plug_update(&plug);

	if (IsKeyPressed(KEY_Z)) plug.camera.zoom = 1.0f;

	if (IsKeyPressed(KEY_R)) {
	    if (!reload_libplug()) return 1;
	}

	if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
	    plug_save(&plug);
	}

	plug_render(&plug, background, demoTile, player, menu);
    }

    plug_free(&plug);

    UnloadTexture(background);
    UnloadTexture(demoTile);
    CloseWindow();

    return 0;
}
