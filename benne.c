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

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1 * ( EVENT_SIZE + 16 ) )
#define SHADERFILE "main.glsl"

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

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

int compileFragmentShader(GLuint* fragmentShader) {
    char* fragmentSourceFromFile = readFile(SHADERFILE);
    const GLchar* fragmentSource[3] = {fragmentSourceHeader,
                                       fragmentSourceFromFile,
                                       fragmentSourceFooter};
    *fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragmentShader, 3, &fragmentSource, NULL);
    glCompileShader(*fragmentShader);
    free(fragmentSourceFromFile);
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

int main(void) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* window;
    int windowHeight = 450;
    int windowWidth = 800;
    window = glfwCreateWindow(windowWidth, windowHeight, "benne", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glViewport(0,0,windowWidth,windowHeight);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return -1;
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    GLfloat vertices[] = {
         -1.0f,  1.0f,
         -1.0f, -1.0f,
          1.0f, -1.0f,

         -1.0f,  1.0f,
          1.0f,  1.0f,
          1.0f, -1.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    {
        GLint status;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            char buffer[512];
            glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
            fprintf(stderr, "vertex shader failed to compile... %s\n", buffer);
            return -1;
        }
    }
    GLuint fragmentShader;
    compileFragmentShader(&fragmentShader);
    if (testShaderCompilation(&fragmentShader)) {
        return -1;
    }

    GLuint shaderProgram;
    if (createShaderProgram(&shaderProgram, &vertexShader, &fragmentShader)) {
        return -1;
    }

    GLint uniTime;
    GLint uniResolution;
    GLint uniMouse;
    GLint posAttrib;
    double xpos, ypos, imousex, imousey;

    uniTime = glGetUniformLocation(shaderProgram, "iTime");
    uniMouse = glGetUniformLocation(shaderProgram, "iMouse");
    uniResolution = glGetUniformLocation(shaderProgram, "iResolution");
    glUniform3f(uniResolution, windowWidth, windowHeight, 0.0);
    glUniform2f(uniMouse, windowWidth/2.0, 0.0);
    posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    struct timeval startTime;
    struct timeval currentTime;
    char buffer[BUF_LEN];
    int eventsLength, pollReturn;
    float secondsElapsed = 0.0f;
    int inotifyFileDesciptor = inotify_init();
    // TODO (08 Feb 2020 sam): See what exactly the response of this is supposed to
    // be and how it is used / stored..
    int fileWatcher = inotify_add_watch(inotifyFileDesciptor, SHADERFILE, IN_MODIFY);
    gettimeofday(&startTime, NULL);
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        glfwPollEvents();
        gettimeofday(&currentTime, NULL);
        secondsElapsed = (currentTime.tv_sec - startTime.tv_sec) +
                         ((currentTime.tv_usec - startTime.tv_usec) / 1000000.0);
        glUniform1f(uniTime, secondsElapsed);
        glfwGetCursorPos(window, &xpos, &ypos);
        if (xpos > 0.0 && xpos < windowWidth &&
            ypos > 0.0 && ypos < windowHeight) {
            imousex = xpos;
            imousey = ypos;
        }
        glUniform2f(uniMouse, imousex, imousey);

        struct pollfd pollWatcher = {inotifyFileDesciptor, POLLIN, 0};
        pollReturn = poll(&pollWatcher, 1, 50);
        if (pollReturn < -1) {
            fprintf(stderr, "some error in waiting for file changes...\n");
        } else if (pollReturn > 0) {
            printf("file change detected. compiling...\n");
            eventsLength = read(inotifyFileDesciptor, buffer, BUF_LEN);
            if (eventsLength < 0) {
                fprintf(stderr, "some error in inotify read\n");
            }
            fileWatcher = inotify_add_watch(inotifyFileDesciptor, SHADERFILE, IN_MODIFY);
            compileFragmentShader(&fragmentShader);
            if (testShaderCompilation(&fragmentShader)) {
                printf("shader compilation failed... continuing.\n");
                continue;
            }
            printf("compiled fragment shader...\n");
            if (createShaderProgram(&shaderProgram, &vertexShader, &fragmentShader)) {
                printf("shader compilation failed... continuing.\n");
                continue;
            }
            printf("compiled shader program...\n");
            uniTime = glGetUniformLocation(shaderProgram, "iTime");
            uniResolution = glGetUniformLocation(shaderProgram, "iResolution");
            glUniform3f(uniResolution, windowWidth, windowHeight, 0.0);
            glUniform2f(uniMouse, imousex, imousey);
            posAttrib = glGetAttribLocation(shaderProgram, "position");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
    }

    glfwTerminate();
    return 0;
}
