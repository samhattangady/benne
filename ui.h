#include "imgui_pch.h"
#include "distance_fields.h"
#include <stdio.h>

typedef struct ui_state {
    df_heap* heap;
    unsigned int active_index;
} ui_state;

void draw_node_editor(ui_state* state);
void draw_shape_selector(ui_state* state, unsigned int index);
