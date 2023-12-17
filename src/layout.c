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

void layout_item(bool selected, Texture2D texture, Rectangle rect, Rectangle source) {
    Color color = WHITE;
    if (!CheckCollisionPointRec(GetMousePosition(), rect) && !selected) {
	color = Fade(color, 0.5f);
    }

    DrawTexturePro(texture, source, rect, (Vector2){0, 0}, 0, color);
}

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
