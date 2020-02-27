#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/unistd.h>
#include <poll.h>
#include <iostream>
#include <fstream>
#include "benne_string.h"
#define HELPERS "helpers.glsl"
#define BASE "base.glsl"

// TODO (27 Feb 2020 sam): Move this somewhere else. Doesn't really make so much
// sense here. Also should probably return a string?
char* read_file(char *filename) {
    // https://stackoverflow.com/a/3464656/5453127
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");
    if (handler) {
        fseek(handler, 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);
        buffer = (char*) malloc(sizeof(char) * (string_size + 5) );
        read_size = fread(buffer, sizeof(char), string_size, handler);
        buffer[string_size] = '\0';
        if (string_size != read_size) {
            free(buffer);
            buffer = NULL;
        }
        fclose(handler);
    }
    return buffer;
}

// Shader sources
const GLchar* vertex_source = "\
    #version 330 core\n\
    in vec2 position;\
    out vec2 fragCoord;\
    uniform vec3 iResolution;\
    void main()\
    {\
        // This all has to be done because I want the result to be \n\
        // compatible with shadertoy, which has things set up this way \n\
        fragCoord.x = ((position.x/2.0) + 0.5)*iResolution.x;\n\
        fragCoord.y = ((position.y/2.0) + 0.5)*iResolution.y;\n\
        // fragCoord.x = ((position.x/2.0) + 0.5)*iResolution.x*2.0;\n\
        // fragCoord.y = position.y*iResolution.y;\n\
        gl_Position = vec4(position, 0.0, 1.0);\
    }\
";
GLchar* fragment_source_header = "\
    #version 330 core\n\
    in vec2 fragCoord;\n\
    out vec4 fragColor;\n\
    uniform float iTime;\n\
    uniform vec2 iMouse;\n\
    uniform vec3 iResolution;\n\
";
GLchar* fragment_source_footer = "\
    void main()\n\
    {\n\
        mainImage(fragColor, fragCoord);\n\
    }\n\
";

int compile_vertex_shader(GLuint* vertex_shader) {
    *vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(*vertex_shader);
    return 0;
}

int compile_fragment_shader(GLuint* fragment_shader, string* shader_source) {
    char* fragment_source_helpers = read_file(HELPERS);
    char* fragment_source_base = read_file(BASE);
    string fragment_source = empty_string();
    append_sprintf(&fragment_source, fragment_source_header);
    append_sprintf(&fragment_source, fragment_source_helpers);
    append_string(&fragment_source, shader_source);
    append_sprintf(&fragment_source, fragment_source_base);
    append_sprintf(&fragment_source, fragment_source_footer);
    *fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragment_shader, 1, &(fragment_source.text), NULL);
    glCompileShader(*fragment_shader);
    free(fragment_source_helpers);
    free(fragment_source_base);
    return 0;
}

int test_shader_compilation(GLuint* shader) {
    GLint status;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(*shader, 512, NULL, buffer);
        fprintf(stderr, "fragment shader failed to compile... %s\n", buffer);
        return -1;
    }
    return 0;
}

int create_shader_program(GLuint* shader_program, GLuint* vertex_shader, GLuint* fragment_shader) {
    *shader_program = glCreateProgram();
    glAttachShader(*shader_program, *vertex_shader);
    glAttachShader(*shader_program, *fragment_shader);
    glBindFragDataLocation(*shader_program, 0, "fragColor");
    glLinkProgram(*shader_program);
    {
        GLint status;
        glGetProgramiv(*shader_program, GL_LINK_STATUS, &status);
        if(status != GL_TRUE) {
            char buffer[512];
            glGetProgramInfoLog(*shader_program, 512, NULL, buffer);
            fprintf(stderr, "shader program failed to compile... %s\n", buffer);
            return -1;
        }
    }
    glUseProgram(*shader_program);
    glDetachShader(*shader_program, *fragment_shader);
    glDeleteShader(*fragment_shader);
    return 0;
}

