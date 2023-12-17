/* -*- compile-command: "make -C .. libplug" -*- */
#include "raylib.h"
#include "layout.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"

#define DRAW_TEXTURE_REC(texture, source, rec, color) DrawTextureRec((texture), (source), (Vector2){(rec).x + x, (rec).y + y}, (color))

Rectangle layout_make_rec(float x, float y, float w, float h) {
    Rectangle r = {
	.x = x,
	.y = y,
	.width = w,
	.height = h,
    };
    return r;
}

//void ui_widget(Rectangle rect, Color color) {
//    Rectangle rr = {
//	ceilf(rect.x), ceilf(rect.y), ceilf(rect.width), ceilf(rect.height)
//    };
//    if (!CheckCollisionPointRec(GetMousePosition(), rr)) {
//	color = Fade(color, 0.5f);
//    }
//    DrawRectangleRec(rr, color);
//}
//
//void ui_widget_button(Texture2D texture, Rectangle rect, const char *text) {
//    Rectangle rr = {
//	ceilf(rect.x), ceilf(rect.y), ceilf(rect.width), ceilf(rect.height)
//    };
//    Color color = WHITE;
//
//    Rectangle recs[9] = {
//	BUTTON_PANEL_TOP_LEFT,
//	BUTTON_PANEL_TOP,
//	BUTTON_PANEL_TOP_RIGHT,
//	BUTTON_PANEL_LEFT,
//	BUTTON_PANEL_MIDDLE,
//	BUTTON_PANEL_RIGHT,
//	BUTTON_PANEL_BOTTOM_LEFT,
//	BUTTON_PANEL_BOTTOM,
//	BUTTON_PANEL_BOTTOM_RIGHT,
//    };
//
//    size_t tile_width = UI_TILE_SIZE * (int)(rr.width / UI_TILE_SIZE);
//    rr.x = rr.x + (rr.width - tile_width) / 2;
//    rr.width = tile_width;
//    size_t tile_height = UI_TILE_SIZE * (int)(rr.height / UI_TILE_SIZE);
//    rr.y = rr.y + (rr.height - tile_height) / 2;
//    rr.height = tile_height;
//
//    if (!CheckCollisionPointRec(GetMousePosition(), rr)) {
//	color = Fade(color, 0.5f);
//    }
//
//    for (size_t y = 0; y < tile_height; y += UI_TILE_SIZE) {
//	for (size_t x = 0; x < tile_width; x += UI_TILE_SIZE) {
//	    if (x == 0 && y == 0) DRAW_TEXTURE_REC(texture, recs[TOP_LEFT], rr, color);
//	    else if (x == 0 && y == tile_height - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[BOTTOM_LEFT], rr, color);
//	    else if (x == tile_width - UI_TILE_SIZE && y == 0) DRAW_TEXTURE_REC(texture, recs[TOP_RIGHT], rr, color);
//	    else if (x == tile_width - UI_TILE_SIZE && y == tile_height - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[BOTTOM_RIGHT], rr, color);
//	    else if (y == 0) DRAW_TEXTURE_REC(texture, recs[TOP], rr, color);
//	    else if (x == 0) DRAW_TEXTURE_REC(texture, recs[LEFT], rr, color);
//	    else if (y == tile_height - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[BOTTOM], rr, color);
//	    else if (x == tile_width - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[RIGHT], rr, color);
//	    else DRAW_TEXTURE_REC(texture, recs[MIDDLE], rr, color);
//	}
//    }
//
//    Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 3);
//    Vector2 text_pos = {
//	.x = rr.x + (tile_width - text_size.x) / 2,
//	.y = rr.y + (tile_height - text_size.y) / 2,
//    };
//    DrawTextEx(GetFontDefault(), text, text_pos, 20, 3, color);
//}

void layout_item(Texture2D texture, Rectangle rect, Rectangle source) {
    Color color = WHITE;
    if (!CheckCollisionPointRec(GetMousePosition(), rect)) {
	color = Fade(color, 0.5f);
    }

    DrawTexturePro(texture, source, rect, (Vector2){0, 0}, 0, color);
}

//void ui_widget_text(Rectangle rect, const char *text, Color color) {
//    Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 3);
//    Vector2 text_pos = {
//	.x = rect.x + (rect.width - text_size.x) / 2,
//	.y = rect.y + (rect.height - text_size.y) / 2,
//    };
//    DrawTextEx(GetFontDefault(), text, text_pos, 20, 3, color);
//}

Layout layout_make(Layout_Orient orient, Rectangle rect, size_t count, float gap) {
    Layout result = {
	.orient = orient,
	.rect = rect,
	.count = count,
	.gap = gap,
    };
    return result;
}

static Rectangle layout_slot(Layout *l, const char *file_path, int line) {
    if (l->i >= l->count) {
	fprintf(stderr, "%s:%d: ERROR: Layout overflow\n", file_path, line);
	exit(1);
    }

    Rectangle r = {0};
    switch (l->orient) {
    case LO_HORI:
	r.width = l->rect.width/l->count;
	r.height = l->rect.height;
	r.x = l->rect.x + l->i*r.width;
	r.y = l->rect.y;

	if (l->i == 0) { // First
	    r.width -= l->gap/2;
	} else if (l->i >= l->count -1) { // Last
	    r.x += l->gap/2;
	    r.width -= l->gap/2;
	} else { // Middle
	    r.x += l->gap/2;
	    r.width -= l->gap;
	}
	break;
    case LO_VERT:
	r.width = l->rect.width;
	r.height = l->rect.height/l->count;
	r.x = l->rect.x;
	r.y = l->rect.y + l->i*r.height;

	if (l->i == 0) { // First
	    r.height -= l->gap/2;
	} else if (l->i >= l->count -1) { // Last
	    r.y += l->gap/2;
	    r.height -= l->gap/2;
	} else { // Middle
	    r.y += l->gap/2;
	    r.height -= l->gap;
	}

	break;
    default:
	assert(0 && "Unreachable");
    }

    l->i += 1;

    return r;
}

void layout_stack_push(Layout **ls, Layout_Orient orient, Rectangle rect, size_t count, float gap) {
    Layout l = layout_make(orient, rect, count, gap);
    array_push(*ls, l);
}

void layout_stack_pop(Layout **ls) {
    assert(array_size(*ls) > 0);
    array_pop_last(*ls);
}

Rectangle layout_stack_slot_loc(Layout **ls, const char *file_path, int line) {
    assert(array_size(*ls) > 0);
    return layout_slot(&array_last(*ls), file_path, line);
}

//void ui_draw_dialog(Rectangle rec, Texture2D texture, PanelType panel_type, Color color) {
//
//    Rectangle recs[9] = {
//	GLASS_PANEL_TOP_LEFT,
//	GLASS_PANEL_TOP,
//	GLASS_PANEL_TOP_RIGHT,
//	GLASS_PANEL_LEFT,
//	GLASS_PANEL_MIDDLE,
//	GLASS_PANEL_RIGHT,
//	GLASS_PANEL_BOTTOM_LEFT,
//	GLASS_PANEL_BOTTOM,
//	GLASS_PANEL_BOTTOM_RIGHT,
//    };
//
//    switch (panel_type) {
//    case GlassCorner:
//	recs[TOP_LEFT] = GLASS_PANEL_CORNER_TOP_LEFT;
//	recs[TOP_RIGHT] = GLASS_PANEL_CORNER_TOP_RIGHT;
//	recs[BOTTOM_LEFT] = GLASS_PANEL_CORNER_BOTTOM_LEFT;
//	recs[BOTTOM_RIGHT] = GLASS_PANEL_CORNER_BOTTOM_RIGHT;
//	break;
//    case Metal:
//	recs[TOP_LEFT] = METAL_PANEL_TOP_LEFT;
//	recs[TOP] = METAL_PANEL_TOP;
//	recs[TOP_RIGHT] = METAL_PANEL_TOP_RIGHT;
//	recs[LEFT] = METAL_PANEL_LEFT;
//	recs[MIDDLE] = METAL_PANEL_MIDDLE;
//	recs[RIGHT] = METAL_PANEL_RIGHT;
//	recs[BOTTOM_LEFT] = METAL_PANEL_BOTTOM_LEFT;
//	recs[BOTTOM] = METAL_PANEL_BOTTOM;
//	recs[BOTTOM_RIGHT] = METAL_PANEL_BOTTOM_RIGHT;
//	break;
//    default:
//    }
//
//    size_t tile_height = UI_TILE_SIZE * (int)(rec.height / UI_TILE_SIZE);
//    size_t tile_width = UI_TILE_SIZE * (int)(rec.width / UI_TILE_SIZE);
//
//    for (size_t y = 0; y < tile_height; y += UI_TILE_SIZE) {
//	for (size_t x = 0; x < tile_width; x += UI_TILE_SIZE) {
//	    if (x == 0 && y == 0) DRAW_TEXTURE_REC(texture, recs[TOP_LEFT], rec, color);
//	    else if (x == 0 && y == tile_height - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[BOTTOM_LEFT], rec, color);
//	    else if (x == tile_width - UI_TILE_SIZE && y == 0) DRAW_TEXTURE_REC(texture, recs[TOP_RIGHT], rec, color);
//	    else if (x == tile_width - UI_TILE_SIZE && y == tile_height - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[BOTTOM_RIGHT], rec, color);
//	    else if (y == 0) DRAW_TEXTURE_REC(texture, recs[TOP], rec, color);
//	    else if (x == 0) DRAW_TEXTURE_REC(texture, recs[LEFT], rec, color);
//	    else if (y == tile_height - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[BOTTOM], rec, color);
//	    else if (x == tile_width - UI_TILE_SIZE) DRAW_TEXTURE_REC(texture, recs[RIGHT], rec, color);
//	    else DRAW_TEXTURE_REC(texture, recs[MIDDLE], rec, color);
//	}
//    }
//}
