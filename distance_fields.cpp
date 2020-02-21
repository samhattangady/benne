#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "distance_fields.h"

char* distance_field_functions(struct Sphere* spheres, int length) {
    char* sdfSphere = "\
    vec2 body%i(vec3 pos) {\n\
        return vec2(sdfSphere(pos, vec3(%f,  %f,  %f),  %f),  %f);\n\
    }\n\
    ";
    int string_length = (sizeof(char) * (strlen(sdfSphere) + 50));
    char* function = (char *) malloc(string_length * length);
    function[0] = '\0';
    char* temp_string = (char *) malloc(string_length);
    temp_string[0] = '\0';
    for (int i=0; i<length; i++) {
        sprintf(temp_string, sdfSphere, (int) spheres[i].id, spheres[i].x, spheres[i].y, spheres[i].z,  spheres[i].r, spheres[i].m);
        strcat(function, temp_string);
    }
    strcat(function, "\0");
    free(temp_string);
    return function;
}

char* distance_field_caller(int length) {
    char* single = "\n\
        d1 = body%i(position);\n\
        if (d1.x < d.x)    d = d1;\n\
    ";
    char* header = "\n\
    vec4 distanceField(vec3 position) {\n\
        vec2 d, d1;\n\
        d = vec2(10000.0, 0.0);\n\
    ";
    char* footer = "\n\
        return vec4(d, 0.0, 0.0);\n\
    }\n\
    ";
    int string_length = 1+(sizeof(char) * (strlen(single)+30));
    string_length += sizeof(char) * strlen(header);
    string_length += sizeof(char) * strlen(footer);
    char* caller = (char*) malloc(length * string_length);
    caller[0] = '\0';
    char* temp_string = (char*) malloc(string_length);
    temp_string[0] = '\0';
    strcat(caller, header);
    for (int i=0; i<length; i++) {
        sprintf(temp_string, single, i);
        strcat(caller, temp_string);
    }
    strcat(caller, footer);
    strcat(caller, "\0");
    free(temp_string);
    return caller;
}
