#ifndef UI_H_
#define UI_H_

#include <stddef.h>
#include "raylib.h"

#define LayoutDrawing(layout, orient, layout_rect, count, gap)	\
    for (int _break = (layout_stack_push((layout), (orient), (layout_rect), (count), (gap)), 1); _break; \
	 _break = 0, layout_stack_pop(layout))

#define layout_stack_slot(ls) layout_stack_slot_loc(ls, __FILE__, __LINE__)

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

void layout_item(Texture2D texture, Rectangle rect, Rectangle source);

Rectangle layout_make_rec(float x, float y, float w, float h);

Layout layout_make(Layout_Orient orient, Rectangle rect, size_t count, float gap);

void layout_stack_push(Layout **ls, Layout_Orient orient, Rectangle rect, size_t count, float gap);

void layout_stack_pop(Layout **ls);

Rectangle layout_stack_slot_loc(Layout **ls, const char *file_path, int line);

#endif // UI_H_
