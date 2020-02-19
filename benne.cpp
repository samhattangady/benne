#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/unistd.h>
#include <poll.h>
#include "shaders.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1 * ( EVENT_SIZE + 16 ) )

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

void generateFragShader(char** shader, float inp) {
    GLchar* v1 = "\n\
    uniform float iSize;\n\
    vec2 body1(vec3 pos) {\n\
        return vec2(sdfSphere(pos, vec3(0.0), %f), 1.0);\n\
    }\
    ";
    sprintf(*shader, v1, inp);
    // return resp;
    // GLchar* v2 = "\n\
    // uniform float iSize;\n\
    // vec2 body1(vec3 pos) {\n\
    //     float d = smin(sdfSphere(pos, vec3(0.3, 0.0, 0.0), iSize),\n\
    //                    sdfSphere(pos, vec3(-0.3, 0.0, 0.0), iSize),\n\
    //                    iSize);\n\
    //     return vec2(d, 1.0);\n\
    // }\
    // ";
    // if (inp==0.0) return v1;
    // else return v2;
}

int main(int, char**) {
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
    window = glfwCreateWindow(windowWidth*2.0, windowHeight*2.0, "benne", NULL, NULL);
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
    //      -1.0f,  1.0f,
    //      -1.0f, -1.0f,
    //       1.0f, -1.0f,

    //      -1.0f,  1.0f,
    //       1.0f,  1.0f,
    //       1.0f, -1.0f
          -1.0f,  1.0f,
          -1.0f,  0.0f,
           0.0f,  0.0f,

          -1.0f,  1.0f,
           0.0f,  1.0f,
           0.0f,  0.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    float ballSize = 0.10009;
    GLuint vertexShader;
    compileVertexShader(&vertexShader);
    if (testShaderCompilation(&vertexShader)) {
        return -1;
    }
    GLuint fragmentShader;
    GLchar* shaderSource = (char*) malloc(sizeof(char) * (10000));
    shaderSource[0] = NULL;
    generateFragShader(&shaderSource, ballSize);
    compileFragmentShader(&fragmentShader, &shaderSource);
    if (testShaderCompilation(&fragmentShader)) {
        return -1;
    }
    free(shaderSource);

    GLuint shaderProgram;
    if (createShaderProgram(&shaderProgram, &vertexShader, &fragmentShader)) {
        return -1;
    }

    GLint uniTime;
    GLint uniResolution;
    GLint uniMouse;
    GLint uniSize;
    GLint posAttrib;
    double xpos, ypos, imousex, imousey;

    uniTime = glGetUniformLocation(shaderProgram, "iTime");
    uniSize = glGetUniformLocation(shaderProgram, "iSize");
    uniMouse = glGetUniformLocation(shaderProgram, "iMouse");
    uniResolution = glGetUniformLocation(shaderProgram, "iResolution");
    glUniform3f(uniResolution, windowWidth, windowHeight, 0.0);
    glUniform1f(uniSize, ballSize);
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
    // int fileWatcher = inotify_add_watch(inotifyFileDesciptor, SHADERFILE, IN_MODIFY);
    gettimeofday(&startTime, NULL);

    std::cout << GL_MAX_FRAGMENT_UNIFORM_COMPONENTS << std::endl;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    static int counter = 0;
    int oldBallSize = ballSize;
    int show_another_window = 1;
    ImVec4 clear_color = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Hello, world!");
            ImGui::Text("This is some useful text.");
            if (ImGui::Button("Add another window"))
                show_another_window++;
            ImGui::SliderFloat("float", &ballSize, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Button"))
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        if (show_another_window)
        {
            int i;
            for (i=0; i<show_another_window; i++) {
                char *windowName = "Window ";
                ImGui::Begin(windowName);
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window--;
                ImGui::End();
            }
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        glfwPollEvents();
        gettimeofday(&currentTime, NULL);
        secondsElapsed = (currentTime.tv_sec - startTime.tv_sec) +
                         ((currentTime.tv_usec - startTime.tv_usec) / 1000000.0);
        glUniform1f(uniTime, secondsElapsed);
        glUniform1f(uniSize, ballSize);
        glfwGetCursorPos(window, &xpos, &ypos);
        if (xpos > 0.0 && xpos < windowWidth &&
            ypos > 0.0 && ypos < windowHeight) {
            imousex = xpos;
            imousey = ypos;
        }
        glUniform2f(uniMouse, imousex, imousey);

        // TODO (20 Feb 2020 sam): Figure out the best indicator to recompile shaders
        // don't go around comparing floats kids...
        // if oldBallSize != ballSize... Also I have _no_ idea why this ain't working
        if (oldBallSize<ballSize-0.01 || oldBallSize>ballSize+0.01) {
            oldBallSize = ballSize;
            shaderSource = (char*) malloc(sizeof(char) * (10000));
            shaderSource[0] = NULL;
            generateFragShader(&shaderSource, ballSize);
            compileFragmentShader(&fragmentShader, &shaderSource);
            if (testShaderCompilation(&fragmentShader)) {
                printf("shader compilation failed... continuing.\n");
                continue;
            }
            free(shaderSource);
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

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
