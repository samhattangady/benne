#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cb_string.h"
#include "distance_fields.h"

int main(int, char**) {
    unsigned int heap_size = 128;
    df_shape* shapes_mem = (df_shape*) malloc(sizeof(df_shape) * heap_size);
    df_operation* ops_mem = (df_operation*) malloc(sizeof(df_operation) * heap_size);
    df_node* nodes_mem = (df_node*) malloc(sizeof(df_node) * heap_size);
    df_heap heap = { heap_size, 0, shapes_mem, ops_mem, nodes_mem };
    load_heap_from_file(&heap, "savefile.txt");

    /*
    unsigned int base = attach_node(&heap,
        generate_rectangle(
            &heap,
            0.0, 0.0, 0.0,
            0.3, 0.3, 0.3,
            0.0, 0.0, 0.0,
            0.03, 1.0
        ),
        { DISTANCE_FIELD_BLEND_ADD, 0.0 },
        0);
    unsigned int attachment = attach_node(&heap,
        generate_sphere(
            &heap,
            0.3, 0.0, 0.0,
            0.1, 1.0
        ),
        { DISTANCE_FIELD_BLEND_ADD, 0.1 },
        base
    );
    unsigned int a = attach_node(&heap,
        generate_sphere(
            &heap,
            -0.3, 0.1, -0.0,
            0.1, 3.0
        ),
        { DISTANCE_FIELD_BLEND_ADD, 0.1 },
        attachment
    );
    unsigned int b = attach_node(&heap,
        generate_sphere(
            &heap,
            0.3, 0.1, 0.0,
            0.15, 1.0
        ),
        { DISTANCE_FIELD_BLEND_SUBTRACT, 0.01 },
        attachment
    );
    unsigned int c = attach_node(&heap,
        generate_sphere(
            &heap,
            0.1, 0.4, 0.3,
            0.4, 1.0
        ),
        { DISTANCE_FIELD_BLEND_ADD, 0.3 },
        b
    );
    */

    print_node(&heap, 0);
    simplify_heap(&heap);
    print_node(&heap, 0);
    // save_heap_to_file(&heap, "savefile.txt");
    // df_shape* shapes_mem2 = (df_shape*) malloc(sizeof(df_shape) * heap_size);
    // df_operation* ops_mem2 = (df_operation*) malloc(sizeof(df_operation) * heap_size);
    // df_node* nodes_mem2 = (df_node*) malloc(sizeof(df_node) * heap_size);
    // df_heap heap2 = { heap_size, 0, shapes_mem2, ops_mem2, nodes_mem2 };
    //load_heap_from_file(&heap, "savefile.txt");
    //save_heap_to_file(&heap, "savefile2.txt");
    return 0;
}
