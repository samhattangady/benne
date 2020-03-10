#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "benne_string.h"
#include "distance_fields.h"

int sphere_distance_field(int id, df_shape* sphere, string* current_source) {
    char* base = "d%i = vec2(sdfSphere(pos, vec3(%.3f, %.3f, %.3f), %.3f), %.3f);\n";
    append_sprintf(current_source, base, id, 
            sphere->data[0],sphere->data[1],sphere->data[2],
            sphere->data[3],sphere->data[4]);
    return 0;
}

int rectangle_distance_field(int id, df_shape* sphere, string* current_source) {
    char* base = "d%i = vec2(sdfRoundBoxRotated(pos, vec3(%.3f, %.3f, %.3f), vec3(%.3f, %.3f, %.3f), vec3(%.3f, %.3f, %.3f), %.3f), %.3f);\n";
    append_sprintf(current_source, base, id, 
            sphere->data[0],sphere->data[1],sphere->data[2],
            sphere->data[3],sphere->data[4],sphere->data[5],
            sphere->data[6],sphere->data[7],sphere->data[8],
            sphere->data[9],sphere->data[10]);
    return 0;
}

int append_distance_field(df_heap* heap, string* current_source, unsigned int index, int depth) {
    switch (heap->shapes[index].type) {
        case EMPTY:
            return 0;
        case SPHERE:
            sphere_distance_field(depth, &heap->shapes[index], current_source);
            return 0;
        case ROUNDED_RECTANGLE:
            rectangle_distance_field(depth, &heap->shapes[index], current_source);
            return 0;
    }
}

int handle_node(df_heap* heap, string* source, unsigned int index, unsigned int depth) {
    df_operation operation = heap->operations[index];
    if (operation.operation == DISTANCE_FIELD_BLEND_ADD) {
        append_sprintf(source,
                "if(d%i.x<d%i.x) material = d%i.y;\nd%i.x = smin(d%i.x, d%i.x, %f);\n\n",
                       depth, depth-1, depth,
                       depth-1, depth-1, depth, operation.extent);
    } else if (operation.operation == DISTANCE_FIELD_BLEND_SUBTRACT) {
        append_sprintf(source,
                "if(d%i.x<d%i.x) material = d%i.y;\nd%i.x = smax(d%i.x, -d%i.x, %f);\n\n",
                       depth, depth-1, depth,
                       depth-1, depth-1, depth, operation.extent);
    } else if (operation.operation == DISTANCE_FIELD_BLEND_UNION) {
        append_sprintf(source,
                "if(d%i.x<d%i.x) material = d%i.y;\nd%i.x = smax(d%i.x, d%i.x, %f);\n\n",
                       depth, depth-1, depth,
                       depth-1, depth-1, depth, operation.extent);
    }
    // TODO (26 Feb 2020 sam): Add support for other blend operations.
    return 0;
}  

int distance_field_caller(df_heap* heap, string* source, unsigned int index, unsigned int depth) {
    // We currently always assume the root of the tree is at index 0. I don't know
    // whether there is a better way to do this, but I think its a fair assumption
    append_distance_field(heap, source, index, depth);
    for (int i=0; i<heap->nodes[index].size; i++) {
        unsigned int child_index = heap->nodes[index].children[i];
        distance_field_caller(heap, source, child_index, depth+1); 
    }
    if (depth>0) {
        handle_node(heap, source, index, depth);
    }
    // for (int i=0; i<node->number_of_children; i++) {
        // distance_field_caller(node->children[i], heap, source);
        // TODO (09 Mar 2020 sam): Figure out how to access the material
        // handle_node(node->shape_index, node->children[i]->shape_index,
        //             heap->shapes[node->children[i]->shape_index].data[4],
        //             node->children[i]->operation, source);
    // }
    return 0;
}

string generate_frag_shader(df_heap* heap) {
    string frag_shader = empty_string();
    append_sprintf(&frag_shader, "vec4 distanceField(vec3 pos) {\n");
    append_sprintf(&frag_shader, "float d = 10000.0;\nfloat material = 0.0;\n");
    // TODO (10 Mar 2020 sam): Figure out a way to get the actual deepest node in tree
    append_sprintf(&frag_shader, "vec2 d0, d1, d2, d3, d4, d5, d6, d7;\n");
    append_sprintf(&frag_shader, "vec3 p0, p1, p2, p3, p4, p5, p6, p7;\n");
    distance_field_caller(heap, &frag_shader, 0, 0);
    append_sprintf(&frag_shader, "return vec4(d0.x, material, 0.0, 0.0);\n}\n");
    return frag_shader;
}

// To set the node to have no parent, set the parent index same as shape index
// For the first node created, passing a NULL will automatically achieve this.
unsigned int attach_node (df_heap* heap,
                        unsigned int shape_index,
                        df_operation operation,
                        unsigned int parent_index) {
    // TODO (10 Mar 2020 sam): Some thought really has to go into the design of
    // this API specifically... I don't really like how we are returning the
    // shape index that we are getting. It feels a little weird.
    heap->operations[shape_index] = operation;
    if (parent_index != shape_index) {
        df_node* parent_node = &heap->nodes[parent_index];
        if (parent_node->children == NULL) {
            // TODO (25 Feb 2020 sam): Currently, I'm hard coding a limit of 32
            // children for each node. Maybe later we can make this dynamically
            // allocated.
            parent_node->children = (unsigned int*) malloc(sizeof(unsigned int) * 32);
        }
        parent_node->children[parent_node->size] = shape_index;
        parent_node->size++;
    }
    return shape_index;
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

void print_node(df_heap* heap, unsigned int root) {
    df_node* node = &heap->nodes[root];
    printf("node %i has %i childrens -> ", root, node->size);
    for (int i=0; i<node->size; i++) {
        printf("node %i(%i), ", node->children[i],
                                heap->nodes[node->children[i]].size);
    }
    printf("\n");
    for (int i=0; i<node->size; i++) {
        print_node(heap, node->children[i]);
    }
}

unsigned int generate_sphere(df_heap* heap, float x, float y, float z, float radius, float material) {
    int index = add_shape_to_heap(heap, 
        { SPHERE,
             {x, y, z, radius, material,
              0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
              0.0f, 0.0f, 0.0f, 0.0f, 0.0f}}
        );
    return index;
}

unsigned int generate_rectangle(df_heap* heap,
        float x, float y, float z,    // position
        float w, float b, float h,    // size
        float rx, float ry, float rz, // rotation
        float radius, float material) {
    int index = add_shape_to_heap(heap, 
        { ROUNDED_RECTANGLE,
             {x, y, z,
              w, b, h,
              rx, ry, rz,
              radius, material,
              0.0f, 0.0f, 0.0f, 0.0f}}
        );
    return index;

}

unsigned int add_shape_to_heap(df_heap* heap, df_shape shape) {
    // FIXME (09 Mar 2020 sam): Grow dynamically once we hit the size...
    unsigned int index = heap->filled;
    heap->shapes[index] = shape;
    heap->filled++;
    return index;
}
