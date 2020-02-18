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
#define SHADERFILE "main.glsl"

char* readFile(char *filename) {
    // https://stackoverflow.com/a/3464656/5453127
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");
    if (handler) {
        fseek(handler, 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);
        buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
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
const GLchar* vertexSource = "\
    #version 330 core\n\
    in vec2 position;\
    out vec2 fragCoord;\
    uniform vec3 iResolution;\
    void main()\
    {\
        // This all has to be done because I want the result to be \n\
        // compatible with shadertoy, which has things set up this way \n\
        fragCoord.x = ((position.x/2.0) + 0.5)*iResolution.x;\
        fragCoord.y = ((position.y/2.0) + 0.5)*iResolution.y;\
        gl_Position = vec4(position, 0.0, 1.0);\
    }\
";
const GLchar* fragmentSourceHeader = "\
    #version 330 core\n\
    in vec2 fragCoord;\n\
    out vec4 fragColor;\n\
    uniform float iTime;\n\
    uniform vec2 iMouse;\n\
    uniform vec3 iResolution;\n\
";
const GLchar* fragmentSourceFooter = "\
    void main()\n\
    {\n\
        mainImage(fragColor, fragCoord);\n\
    }\n\
";

int compileVertexShader(GLuint* vertexShader) {
    *vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vertexShader, 1, &vertexSource, NULL);
    glCompileShader(*vertexShader);
    return 0;
}

int compileFragmentShader(GLuint* fragmentShader) {
    char* fragmentSourceFromFile = readFile(SHADERFILE);
    int stringSize = 1;
    stringSize += strlen(fragmentSourceHeader);
    stringSize += strlen(fragmentSourceFromFile);
    stringSize += strlen(fragmentSourceFooter);
    GLchar* rawFragmentSource = (char*) malloc(sizeof(char) * (stringSize));
    strcat(rawFragmentSource, fragmentSourceHeader);
    strcat(rawFragmentSource, fragmentSourceFromFile);
    strcat(rawFragmentSource, fragmentSourceFooter);
    const GLchar* fragmentSource = rawFragmentSource;
    *fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(*fragmentShader);
    free(fragmentSourceFromFile);
    free(rawFragmentSource);
    return 0;
}

int testShaderCompilation(GLuint* shader) {
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

int createShaderProgram(GLuint* shaderProgram, GLuint* vertexShader, GLuint* fragmentShader) {
    *shaderProgram = glCreateProgram();
    glAttachShader(*shaderProgram, *vertexShader);
    glAttachShader(*shaderProgram, *fragmentShader);
    glBindFragDataLocation(*shaderProgram, 0, "fragColor");
    glLinkProgram(*shaderProgram);
    {
        GLint status;
        glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &status);
        if(status != GL_TRUE) {
            char buffer[512];
            glGetProgramInfoLog(*shaderProgram, 512, NULL, buffer);
            fprintf(stderr, "shader program failed to compile... %s\n", buffer);
            return -1;
        }
    }
    glUseProgram(*shaderProgram);
    return 0;
}

