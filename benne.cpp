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
#include "distance_fields.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1 * ( EVENT_SIZE + 16 ) )

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

char* generate_frag_shader(struct Sphere* spheres, int length) {
    char* body_functions = distance_field_functions(spheres, length);
    char* body_checks = distance_field_caller(length);
    int string_size = 10+strlen(body_functions)+strlen(body_checks);
    char* fragment_shader = (char*) malloc(sizeof(char)* (string_size));
    fragment_shader[0] = '\0';
    strcat(fragment_shader, body_functions);
    strcat(fragment_shader, body_checks);
    strcat(fragment_shader, "\0");
    free(body_functions);
    free(body_checks);
    return fragment_shader;
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

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return -1;
    }

    int number_of_spheres = 1;
    struct Sphere *shapes = (struct Sphere*) malloc(sizeof(struct Sphere) * 100);
    shapes[0] = {0.0, 0.0, 0.0, 0.0, 0.2, 1.0};
    // shapes[0] = NULL;

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

    float ballSize = 0.10009;
    GLuint vertexShader;
    compileVertexShader(&vertexShader);
    if (testShaderCompilation(&vertexShader)) {
        return -1;
    }
    char* shader_source = generate_frag_shader(shapes, number_of_spheres);
    GLuint fragmentShader;
    compile_fragment_shader(&fragmentShader, &shader_source);
    if (testShaderCompilation(&fragmentShader)) {
        return -1;
    }
    free(shader_source);

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
    gettimeofday(&startTime, NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    int oldBallSize = ballSize;
    ImVec4 clear_color = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Create a Sphere...");
            ImGui::Text("number of spheres = %i", number_of_spheres);
            if (ImGui::Button("Click to create sphere")) {
                shapes[number_of_spheres].id = number_of_spheres * 1.0;
                shapes[number_of_spheres].x = 1.0;
                shapes[number_of_spheres].y = 1.0 - 0.01*number_of_spheres;
                shapes[number_of_spheres].z = 1.0;
                shapes[number_of_spheres].r = 0.5;
                shapes[number_of_spheres].m = 1.0;
                number_of_spheres++;
            }
            ImGui::End();
        }

        for (int i=0; i<number_of_spheres; i++) {
            char windowName[30];
            sprintf(windowName, "Sphere %i", i);
            ImGui::Begin(windowName);
            ImGui::SliderFloat("x", &shapes[i].x, -5.0f, 5.0f);
            ImGui::SliderFloat("y", &shapes[i].y, -5.0f, 5.0f);
            ImGui::SliderFloat("z", &shapes[i].z, -5.0f, 5.0f);
            ImGui::SliderFloat("r", &shapes[i].r, -5.0f, 5.0f);
            ImGui::End();
        }
        // {
        //     ImGui::Begin("Hello, world!");
        //     ImGui::Text("This is some useful text.");
        //     if (ImGui::Button("Add another window"))
        //         show_another_window++;
        //     ImGui::SliderFloat("float", &ballSize, 0.0f, 1.0f);
        //     ImGui::ColorEdit3("clear color", (float*)&clear_color);
        //     if (ImGui::Button("Button"))
        //         counter++;
        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);
        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //     ImGui::End();
        // }

        // if (show_another_window)
        // {
        //     int i;
        //     for (i=0; i<show_another_window; i++) {
        //         char *windowNameTemplate = "Window %i";
        //         char *windowName = (char*) malloc(sizeof(char) * (strlen(windowNameTemplate)+20));
        //         sprintf(windowName, windowNameTemplate, i);
        //         ImGui::Begin(windowName);
        //         ImGui::Text("Hello from another window!");
        //         if (ImGui::Button("Close Me"))
        //             show_another_window--;
        //         ImGui::End();
        //     }
        // }
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        float scaling = 1.5;
        glViewport(0, display_h*(1.0-(1.0/scaling)),
                   display_w/scaling, display_h/scaling);
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

        // TODO (20 Feb 2020 sam): Figure out the best indicator to recompile shaders
        // don't go around comparing floats kids...
        // if oldBallSize != ballSize... Also I have _no_ idea why this ain't working
        if (oldBallSize<ballSize-0.01 || oldBallSize>ballSize+0.01) {
            oldBallSize = ballSize;
            shader_source = generate_frag_shader(shapes, number_of_spheres);
            compile_fragment_shader(&fragmentShader, &shader_source);
            if (testShaderCompilation(&fragmentShader)) {
                printf("shader compilation failed... continuing.\n");
                continue;
            }
            if (createShaderProgram(&shaderProgram, &vertexShader, &fragmentShader)) {
                printf("shader compilation failed... continuing.\n");
                continue;
            }
            free(shader_source);
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
        // break;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
