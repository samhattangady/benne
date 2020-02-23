// TODO (20 Feb 2020 sam): This should probably be in the cpp class? I'm not sure really.
struct Sphere {
    float id;
    float x;
    float y;
    float z;
    float r;
    float m;  // material
};

struct Node {
    Sphere sphere;
    struct Node* children;
    int number_of_children;
    int operation;
};

char* distance_field_functions(struct Sphere* spheres, int length);
char* distance_field_caller(int length);
char* generate_frag_shader(struct Sphere* spheres, int length);
