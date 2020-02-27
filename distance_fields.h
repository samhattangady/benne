#ifndef DISTANCE_FIELDS_DEFINED
#define DISTANCE_FIELDS_DEFINED
#include "benne_string.h"

// TODO (20 Feb 2020 sam): This should probably be in the cpp class? I'm not sure really.
typedef struct Sphere {
    float x;
    float y;
    float z;
    float r;
    float m;  // material
} Sphere;

// TODO (23 Feb 2020 sam): Should non blend operations be separate options?
typedef enum DISTANCE_FIELD_OPERATIONS {
    DISTANCE_FIELD_BLEND_ADD,
    DISTANCE_FIELD_BLEND_SUBTRACT,
    DISTANCE_FIELD_BLEND_UNION,
} DistanceFieldOperationEnum;

typedef struct DistanceFieldOperation {
    DistanceFieldOperationEnum operation;
    float extent;
} DistanceFieldOperation;

typedef struct BenneNode {
    int id;
    DistanceFieldOperation operation;
    Sphere sphere;
    BenneNode* parent;
    int number_of_children;
    // TODO (26 Feb 2020 sam): should probably be BenneNode* children[]
    // but I don't understand C well enough to do it the idiomatic way, so
    // I'm doing it the scam way... Basically it is an array of BenneNode*
    // TODO (26 Feb 2020 sam): this could also be children[32]. Anyway I have
    // set a hard limit (temporarily hopefully), and it might be easier that way
    BenneNode** children;
} BenneNode;

int distance_field_functions(BenneNode* node, string* source);
int distance_field_caller(BenneNode* node, string* source);
string generate_frag_shader(BenneNode* node);
BenneNode* attach_node (DistanceFieldOperation operation,
                       float x, float y, float z,
                       float r, float m,
                       BenneNode* parent);
BenneNode* generate_base_node();
void print_node(BenneNode* node);
int dispose_node(BenneNode* node);
int detach_node(BenneNode* node);
#endif
