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
    unsigned int size;
    unsigned int* children;
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

typedef struct BenneNode {
    df_operation operation;
    unsigned int shape_index;
    BenneNode* parent;
    unsigned int number_of_children;
    // TODO (26 Feb 2020 sam): should probably be BenneNode* children[]
    // but I don't understand C well enough to do it the idiomatic way, so
    // I'm doing it the scam way... Basically it is an array of BenneNode*
    // TODO (26 Feb 2020 sam): this could also be children[32]. Anyway I have
    // set a hard limit (temporarily hopefully), and it might be easier that way
    BenneNode** children;
} BenneNode;

string generate_frag_shader(df_heap* heap);
unsigned int attach_node (df_heap* heap,
                       unsigned int shape_index, 
                       df_operation operation,
                       unsigned int parent);
void print_node(df_heap* heap, unsigned int root);
int dispose_node(BenneNode* node);
int detach_node(BenneNode* node);
unsigned int generate_sphere(df_heap* heap, float x, float y, float z, float radius, float material);
unsigned int generate_rectangle(df_heap* heap, float x, float y, float z, float radius, float material);
unsigned int generate_rectangle(df_heap* heap,
        float x, float y, float z,    // position
        float w, float b, float h,    // size
        float rx, float ry, float rz, // rotation
        float radius, float material);
unsigned int add_shape_to_heap(df_heap* heap, df_shape shape);
#endif
