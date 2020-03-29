#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "benne_string.h"
#include "distance_fields.h"

int sphere_distance_field(int id, df_shape* sphere, string* current_source) {
    char* base = "d%i = vec2(sdfSphere(pos, %.3f), %.3f);\n";
    append_sprintf(current_source, base, id,
            sphere->data[9],sphere->data[14]);
    return 0;
}

int rectangle_distance_field(int id, df_shape* shape, string* current_source) {
    char* base = "d%i = vec2(sdfRoundedBox(pos, vec3(%.3f, %.3f, %.3f), %.3f), %.3f);\n";
    append_sprintf(current_source, base, id,
            shape->data[6],shape->data[7],shape->data[8],
            shape->data[9],shape->data[14]);
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
    return -1;
}

int append_depth_position(df_heap* heap, string* source, unsigned int index, unsigned int depth) {
    // This method could probably do with a better name. It is used to mark the position
    // of a node, so that all the children can then be placed relative to their parent
    df_shape* shape = &heap->shapes[index];
    append_sprintf(source,
            "pos=in_pos;\np%i = vec3(%.3f, %.3f, %.3f);\nr%i = vec3(%.3f, %.3f, %.3f);\n",
            depth,
            shape->data[0],shape->data[1],shape->data[2],
            shape->data[3],shape->data[4],shape->data[5]);
    for (int i=0; i<=depth; i++) {
        append_sprintf(source,
                "pos = moveAndRotate(pos, p%i, r%i);\n",
                 i, i);
    }
    return 0;
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
    append_depth_position(heap, source, index, depth);
    append_distance_field(heap, source, index, depth);
    for (int i=0; i<heap->nodes[index].size; i++) {
        unsigned int child_index = heap->nodes[index].children[i];
        distance_field_caller(heap, source, child_index, depth+1); 
    }
    if (depth>0) {
        handle_node(heap, source, index, depth);
    }
    return 0;
}

string generate_frag_shader(df_heap* heap) {
    string frag_shader = empty_string();
    append_sprintf(&frag_shader, "vec4 distanceField(vec3 pos) {\n");
    append_sprintf(&frag_shader, "float d = 10000.0;\nfloat material = 0.0;\nvec3 in_pos=pos;\n");
    // TODO (10 Mar 2020 sam): Figure out a way to get the actual deepest node in tree
    append_sprintf(&frag_shader, "vec2 d0, d1, d2, d3, d4, d5, d6, d7;\n");
    append_sprintf(&frag_shader, "vec3 p0, p1, p2, p3, p4, p5, p6, p7;\n");
    append_sprintf(&frag_shader, "vec3 r0, r1, r2, r3, r4, r5, r6, r7;\n");
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
        // TODO (25 Feb 2020 sam): Currently, I'm hard coding a limit of 32
        // children for each node. Maybe later we can make this dynamically
        // allocated.
        if (parent_node->size == 31) {
            printf("We cannot add further children to the parent. Sorry if you crashed.");
            return shape_index;
        }
        parent_node->children[parent_node->size] = shape_index;
        parent_node->size++;
    }
    return shape_index;
}

int find_parent_index(df_heap* heap, unsigned int index) {
    // Since we are not storing the parent id in the node itself, we need to search
    // the tree to find the parent. This is only required when we are deleting the
    // nodes. It finds the first occurence of the parent. Since the way we use this
    // is to find all the parents and then delete it, then call it again until there
    // are none left, we don't need to return an array of indices (though that may
    // be more efficient.) Also, just storing the parent would be the most efficient
    for (int i=0; i<heap->filled; i++) {
        df_node* node = &heap->nodes[i];
        if (node->size > 0) {
            for (int j=0; j<node->size; j++) {
                if(node->children[j] == index)
                    return i;                
            }
        }
    }
    return -1;
}

int detach_node(df_heap* heap, unsigned int index) {
    // Remove the reference to the index in the parent node
    int parent_index = find_parent_index(heap, index);
    df_node* parent = &heap->nodes[parent_index];
    int node_index;
    for (int i=0; i<parent->size; i++) {
        if (parent->children[i] == index) {
            node_index = i;
            break;
        }
    }
    for (int i=node_index; i<parent->size-1; i++)
        parent->children[i] = parent->children[i+1];
    parent->size--;
    return 0;
}

int dispose_node(df_heap* heap, unsigned int index) {
    // TODO (27 Feb 2020 sam): This needs to be tested. I'm too lazy at this point
    // to actually check and see whether this works as expected one step deeper.
    printf("disposing node %i\n", index);
    df_node* node = &heap->nodes[index];
    for (int i=node->size-1; i>=0; i--)
        dispose_node(heap, node->children[i]);
    detach_node(heap, index);
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
             {x, y, z,
              0.0f, 0.0f, 0.0f,
              0.0f, 0.0f, 0.0f,
              radius,
              0.0f, 0.0f, 0.0f, 0.0f,
              material}}
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
              rx, ry, rz,
              w, b, h,
              radius,
              0.0f, 0.0f, 0.0f,
              material}}
        );
    return index;

}

unsigned int add_shape_to_heap(df_heap* heap, df_shape shape) {
    // FIXME (09 Mar 2020 sam): Grow dynamically once we hit the size...
    // TODO (12 Mar 2020 sam): There might be gaps in the heap from the shapes that
    // were deleted. These must first be utilised before growing the stack.
    unsigned int index = heap->filled;
    heap->shapes[index] = shape;
    heap->filled++;
    return index;
}

int simplify_heap(df_heap* heap) {
    // Remove all the unused nodes, and push the later nodes to use the minumum
    // amount of space.
    // TODO (24 Mar 2020 sam): This just feels so complex. Using so many variables.
    // I wonder if there is a simpler way.
    int heap_size = heap->filled;
    int i, j, k;
    // Check all the children to see which nodes are used.
    int* used = (int*) malloc(sizeof(int) * heap_size);
    for (i=0; i<heap_size; i++) 
        used[i] = 0;
    // 0 is the root node. Don't want to delete that.
    used[0] = 1;
    for (i=0; i<heap_size; i++) {
        for (j=0; j<heap->nodes[i].size; j++) {
            int child = heap->nodes[i].children[j];
            used[child]++; 
        } 
    }
    int new_size = 0;
    int* unused = (int*) malloc(sizeof(int) * heap_size);
    for (int i=0; i<heap_size; i++) {
        if (used[i] > 0)
            new_size++;
    }
    printf("old heap size was %i, new size is %i.\n", heap_size, new_size);
    for (i=0; i<heap_size; i++) {
        if (used[i]==0) {
            printf("removing node %i\n", i);
            for (j=i; j<heap_size-1; j++) {
                heap->shapes[j] = heap->shapes[j+1];
                heap->operations[j] = heap->operations[j+1];
                heap->nodes[j] = heap->nodes[j+1];
                used[j] = used[j+1];
            }
            i--;
        }
    }
    heap->filled = new_size;
    return 0;
}

int save_heap_to_file(df_heap* heap, char* filename) {
    FILE* savefile = fopen(filename, "w");
    if (savefile == NULL) {
        fprintf(stderr, "could not open file to save...\n");
        return -1;
    }
    int i, j;
    unsigned int heap_size = heap->filled;
    fprintf(savefile, "%i\n", heap_size);
    for (i=0; i<heap_size; i++) {
        fprintf(savefile, "%i ", heap->shapes[i].type);
        for (j=0; j<15; j++)
            fprintf(savefile, "%f ", heap->shapes[i].data[j]);
        fprintf(savefile, "\n");
    }
    for (i=0; i<heap_size; i++)
        fprintf(savefile, "%i %f\n", heap->operations[i].operation, heap->operations[i].extent);
    for (i=0; i<heap_size; i++) {
        fprintf(savefile, "%i ", heap->nodes[i].size);
        for (j=0; j<heap->nodes[i].size; j++)
            fprintf(savefile, "%i ", heap->nodes[i].children[j]);
        fprintf(savefile, "\n");
    }
    fclose(savefile);
    return 0;
}

int load_heap_from_file(df_heap* heap, char* filename) {
    printf("-----\nloading savefile : %s\n", filename);
    FILE* savefile = fopen(filename, "r");
    if (savefile == NULL) {
        fprintf(stderr, "could not open file to save...\n");
        return -1;
    }
    int i, j;
    unsigned int heap_size;
    fscanf(savefile, "%i\n", &heap_size);
    printf("found heap size is %i\n", heap_size);
    heap->filled = heap_size;
    for (i=0; i<heap_size; i++) {
        fscanf(savefile, "%i ", &(heap->shapes[i].type));
        for (j=0; j<15; j++)
            fscanf(savefile, "%f ", &(heap->shapes[i].data[j]));
        fscanf(savefile, "\n");
    }
    printf("completed reading shape data\n");
    for (i=0; i<heap_size; i++)
        fscanf(savefile, "%i %f\n", &(heap->operations[i].operation), &(heap->operations[i].extent));
    printf("completed reading operations data\n");
    for (i=0; i<heap_size; i++) {
        fscanf(savefile, "%i ", &(heap->nodes[i].size));
        for (j=0; j<heap->nodes[i].size; j++)
            fscanf(savefile, "%i ", &(heap->nodes[i].children[j]));
        fscanf(savefile, "\n");
    }
    printf("completed reading node data\n-----\n");
    fclose(savefile);
    return 0;
}
