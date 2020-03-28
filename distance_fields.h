#ifndef DISTANCE_FIELDS_DEFINED
#define DISTANCE_FIELDS_DEFINED
#include "benne_string.h"

// TODO (23 Feb 2020 sam): Should non blend operations be separate options?
typedef enum DISTANCE_FIELD_OPERATIONS {
    DISTANCE_FIELD_BLEND_ADD,
    DISTANCE_FIELD_BLEND_SUBTRACT,
    DISTANCE_FIELD_BLEND_UNION,
} df_operation_enum;

typedef struct df_operation {
    df_operation_enum operation;
    float extent;
} df_operation;

typedef enum df_shape_enum {
    EMPTY,
    SPHERE,
    ROUNDED_RECTANGLE,
} df_shape_enum;

typedef struct df_shape {
    // TODO (10 Mar 2020 sam): There might be some merit in taking type out of struct
    // and having a separate array for just the enums. Might improve performance
    // characteristics because of the array derefencing etc.
    df_shape_enum type;
    float data[15];
} df_shape;

typedef struct df_node {
    // TODO (12 Mar 2020 sam): We probably need to add the filled field here as well.
    // Currently number of children has just been hardcoded to 32.
    unsigned int size;
    unsigned int children[31];
} df_node;

typedef struct df_heap {
    unsigned int size;
    unsigned int filled;
    // TODO (10 Mar 2020 sam): Figure out how we can make these flush with each other
    // in memory. In the JAI talk (https://www.youtube.com/watch?v=TH9VCN6UkyQ) Jon Blow
    // talks about a thing where we allocate a single heap, and then calculate offsets
    // within that itself. So that is something we should be able to apply
    df_shape* shapes;
    df_operation* operations;
    df_node* nodes;
} df_heap;

string generate_frag_shader(df_heap* heap);
unsigned int attach_node (df_heap* heap,
                       unsigned int shape_index, 
                       df_operation operation,
                       unsigned int parent);
void print_node(df_heap* heap, unsigned int root);
int dispose_node(df_heap* heap, unsigned int index);
int detach_node(df_heap* heap, unsigned int index);
unsigned int generate_sphere(df_heap* heap, float x, float y, float z, float radius, float material);
unsigned int generate_rectangle(df_heap* heap, float x, float y, float z, float radius, float material);
unsigned int generate_rectangle(df_heap* heap,
        float x, float y, float z,    // position
        float w, float b, float h,    // size
        float rx, float ry, float rz, // rotation
        float radius, float material);
unsigned int add_shape_to_heap(df_heap* heap, df_shape shape);
int simplify_heap(df_heap* heap);
int save_heap_to_file(df_heap* heap, char* filename);
int load_heap_from_file(df_heap* heap, char* filename);
#endif
