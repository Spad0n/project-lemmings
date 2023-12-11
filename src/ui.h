#ifndef UI_H_
#define UI_H_

#include <stddef.h>
#include "raylib.h"

#define UI_TILE_SIZE 33

#define GLASS_PANEL_TOP_LEFT (Rectangle){0, 0, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_TOP (Rectangle){UI_TILE_SIZE, 0, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_TOP_RIGHT (Rectangle){UI_TILE_SIZE * 2, 0, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_LEFT (Rectangle){0, UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_MIDDLE (Rectangle){UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_RIGHT (Rectangle){UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_BOTTOM_LEFT (Rectangle){0, UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_BOTTOM (Rectangle){UI_TILE_SIZE, UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_BOTTOM_RIGHT (Rectangle){UI_TILE_SIZE * 2, UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}

#define GLASS_PANEL_CORNER_TOP_LEFT (Rectangle){100, 200, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_CORNER_TOP_RIGHT (Rectangle){100 + UI_TILE_SIZE * 2, 200, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_CORNER_BOTTOM_LEFT (Rectangle){100, 200 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}
#define GLASS_PANEL_CORNER_BOTTOM_RIGHT (Rectangle){100 + UI_TILE_SIZE * 2, 200 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}

#define METAL_PANEL_TOP_LEFT (Rectangle){200, 200, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_TOP (Rectangle){200 + UI_TILE_SIZE, 200, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_TOP_RIGHT (Rectangle){200 + UI_TILE_SIZE * 2, 200, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_LEFT (Rectangle){200, 200 + UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_MIDDLE (Rectangle){200 + UI_TILE_SIZE, 200 + UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_RIGHT (Rectangle){200 + UI_TILE_SIZE * 2, 200 + UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_BOTTOM_LEFT (Rectangle){200, 200 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_BOTTOM (Rectangle){200 + UI_TILE_SIZE, 200 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}
#define METAL_PANEL_BOTTOM_RIGHT (Rectangle){200 + UI_TILE_SIZE * 2, 200 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}

#define BUTTON_PANEL_TOP_LEFT (Rectangle){200, 300, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_TOP (Rectangle){200 + UI_TILE_SIZE, 300, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_TOP_RIGHT (Rectangle){200 + UI_TILE_SIZE * 2, 300, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_LEFT (Rectangle){200, 300 + UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_MIDDLE (Rectangle){200 + UI_TILE_SIZE, 300 + UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_RIGHT (Rectangle){200 + UI_TILE_SIZE * 2, 300 + UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_BOTTOM_LEFT (Rectangle){200, 300 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_BOTTOM (Rectangle){200 + UI_TILE_SIZE, 300 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}
#define BUTTON_PANEL_BOTTOM_RIGHT (Rectangle){200 + UI_TILE_SIZE * 2, 300 + UI_TILE_SIZE * 2, UI_TILE_SIZE, UI_TILE_SIZE}

#define UILayoutDrawing(layout, orient, layout_rect, count, gap)	\
    for (int _break = (ui_layout_stack_push((layout), (orient), (layout_rect), (count), (gap)), 1); _break; \
	 _break = 0, ui_layout_stack_pop(layout))

#define ui_layout_stack_slot(ls) layout_stack_slot_loc(ls, __FILE__, __LINE__)

typedef enum {
    GlassCorner,
    Glass,
    Metal,
    Button,
} PanelType;

typedef enum {
    TOP_LEFT,
    TOP,
    TOP_RIGHT,
    LEFT,
    MIDDLE,
    RIGHT,
    BOTTOM_LEFT,
    BOTTOM,
    BOTTOM_RIGHT,
} TextureIndex;

typedef enum {
    LO_HORI,
    LO_VERT,
} Layout_Orient;

typedef struct {
    Layout_Orient orient;
    Rectangle rect;
    size_t i;
    size_t count;
    float gap;
} Layout;

//void ui_widget(Texture2D texture, Rectangle source, Vector2 vector);
void ui_widget(Rectangle rect, Color color);

void ui_widget_text(Rectangle rect, const char* text, Color color);

void ui_widget_button(Texture2D texture, Rectangle rect, const char *text);

void ui_widget_item(Texture2D texture, Rectangle rect, Rectangle source);

Rectangle ui_make_layout_rec(float x, float y, float w, float h);

Layout ui_make_layout(Layout_Orient orient, Rectangle rect, size_t count, float gap);

void ui_layout_stack_push(Layout **ls, Layout_Orient orient, Rectangle rect, size_t count, float gap);

void ui_layout_stack_pop(Layout **ls);

Rectangle layout_stack_slot_loc(Layout **ls, const char *file_path, int line);

void ui_draw_dialog(Rectangle rec, Texture2D texture, PanelType panel_type, Color color);

#endif // UI_H_
