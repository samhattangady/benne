#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "benne_string.h"
#include "distance_fields.h"

// Global variable to store the id for the node. This is the simplest way to work
// with it for now. Once I understand the problem space better, maybe we can figure
// out what a better implementation would entail.
int node_count= 0;
int get_node_id() {
    extern int node_count;
    return node_count++;
}

// TODO (26 Feb 2020 sam): PERFORMANCE. This needn't return a string. It could just take
// the existing string, and append to that and return a success int.
string sphere_distance_field(int id, Sphere sphere) {
    char* base = "vec2 d%i = vec2(sdfSphere(pos, vec3(%.3f, %.3f, %.3f), %.3f), %.3f);\n";
    string field = empty_string();
    append_sprintf(&field, base, id, sphere.x, sphere.y, sphere.z, sphere.r, sphere.m);
    return field;
}

int distance_field_functions(BenneNode* node, string* current_source) {
    string temp = sphere_distance_field(node->id, node->sphere);
    append_string(current_source, &temp);
    dispose_string(&temp);
    for (int i=0; i<node->number_of_children; i++) {
        distance_field_functions(node->children[i], current_source);
    }
    return 0;
}

int handle_node(int parent_id, int child_id, float material,
                DistanceFieldOperation operation, string* source) {
    if (operation.operation == DISTANCE_FIELD_BLEND_ADD) {
        append_sprintf(source, "if(d%i.x<d%i.x) material = d%i.y;\nd%i.x = smin(d%i.x, d%i.x, %f);\n\n",
                       child_id, parent_id, child_id,
                       parent_id, parent_id, child_id, operation.extent);
    } else if (operation.operation == DISTANCE_FIELD_BLEND_SUBTRACT) {
        append_sprintf(source, "if(d%i.x<d%i.x) material = d%i.y;\nd%i.x = smax(d%i.x, -d%i.x, %f);\n\n",
                       child_id, parent_id, child_id,
                       parent_id, parent_id, child_id, operation.extent);
    } else if (operation.operation == DISTANCE_FIELD_BLEND_UNION) {
        append_sprintf(source, "if(d%i.x<d%i.x) material = d%i.y;\nd%i.x = smax(d%i.x, d%i.x, %f);\n\n",
                       child_id, parent_id, child_id,
                       parent_id, parent_id, child_id, operation.extent);
    }
    // TODO (26 Feb 2020 sam): Add support for other blend operations.
    return 0;
}  

int distance_field_caller(BenneNode* node, string* source) {
    for (int i=0; i<node->number_of_children; i++) {
        distance_field_caller(node->children[i], source);
        handle_node(node->id, node->children[i]->id,
                    node->children[i]->sphere.m,
                    node->children[i]->operation, source);
    }
    return 0;
}

string generate_frag_shader(BenneNode* node) {
    string frag_shader = empty_string();
    append_sprintf(&frag_shader, "vec4 distanceField(vec3 pos) {\n");
    distance_field_functions(node, &frag_shader);
    append_sprintf(&frag_shader, "float d = 10000.0;\nfloat material = 0.0;\n");
    distance_field_caller(node, &frag_shader);
    append_sprintf(&frag_shader, "return vec4(d0.x, material, 0.0, 0.0);\n}\n");
    return frag_shader;
}

BenneNode* attach_node (DistanceFieldOperation operation,
                       float x, float y, float z, 
                       float r, float m,
                       BenneNode* parent) {
    int node_id = get_node_id();
    BenneNode* node = (BenneNode*) malloc(sizeof(BenneNode));
    *node = {
        node_id,
        operation,
        {x, y, z, r, m},
        parent,
        0,
        NULL
    };
    if (parent != NULL) {
        if (parent->children == NULL) {
            // TODO (25 Feb 2020 sam): Currently, I'm hard coding a limit of 32
            // children for each node. Maybe later we can make this dynamically
            // allocated.
            parent->children = (BenneNode**) malloc(sizeof(BenneNode*) * 32);
        }
        parent->children[parent->number_of_children] = node;
        parent->number_of_children++;
    }
    return node;
}

BenneNode* generate_base_node() {
    // BenneNode base_node = attach_node(
    return attach_node(
            { DISTANCE_FIELD_BLEND_ADD, 0.0 },
            0.0, 0.0, 0.0,
            0.5, 1.0,
            NULL);
    // return base_node;
}

int detach_node(BenneNode* node) {
    // We need to find the index that the node is at in the parent's children
    // Then we need to slide all the other children up by 1, and decrement num
    BenneNode* parent = node->parent;     
    int node_index;
    for (int i=0; i<parent->number_of_children; i++) {
        if (parent->children[i] == node) {
            node_index = i;
            break;
        }
    }
    for (int i=node_index+1; i<parent->number_of_children; i++) {
        parent->children[i-1] = parent->children[i];
    }
    parent->number_of_children--;
    return 0;
}

int dispose_node(BenneNode* node) {
    // TODO (27 Feb 2020 sam): This needs to be tested. I'm too lazy at this point
    // to actually check and see whether this works as expected one step deeper.
    // NOTE (27 Feb 2020 sam): Since we detach each node, we need to dispose the node
    // at 0 at every iteration, as the indices of the children are changing with every
    // detach.
    detach_node(node);
    for (int i=0; i<node->number_of_children; i++)
        dispose_node(node->children[0]);
    if (node->children != NULL)
        free(node->children);
    return 0;
}

void print_node(BenneNode* node) {
    printf("node %i has %i childrens -> ", node->id, node->number_of_children);
    for (int i=0; i<node->number_of_children; i++) {
        printf("node %i(%i), ", node->children[i]->id,
                                node->children[i]->number_of_children);
    }
    printf("\n");
    for (int i=0; i<node->number_of_children; i++) {
        print_node(node->children[i]);
    }
}
