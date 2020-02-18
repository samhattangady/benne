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
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/unistd.h>
#include <poll.h>
#include "shaders.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1 * ( EVENT_SIZE + 16 ) )
#define SHADERFILE "main.glsl"

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
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

    GLuint vertexShader;
    compileVertexShader(&vertexShader);
    if (testShaderCompilation(&vertexShader)) {
        return -1;
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    int show_another_window = 1;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");
            ImGui::Text("This is some useful text.");
            if (ImGui::Button("Add another window"))
                show_another_window++;
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
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
