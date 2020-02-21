// TODO (20 Feb 2020 sam): This should probably be in the cpp class? I'm not sure really.
struct Sphere {
    float id;
    float x;
    float y;
    float z;
    float r;
    float m;  // material
};

char* distance_field_functions(struct Sphere* sphere, int length);
char* distance_field_caller(int length);
