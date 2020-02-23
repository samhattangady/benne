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
    int window_height = 450;
    int window_width = 800;
    window = glfwCreateWindow(window_width*2.0, window_height*2.0, "benne", NULL, NULL);
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

    GLuint vertex_shader;
    compile_vertex_shader(&vertex_shader);
    if (test_shader_compilation(&vertex_shader)) {
        return -1;
    }
    char* shader_source = generate_frag_shader(shapes, number_of_spheres);
    GLuint fragment_shader;
    compile_fragment_shader(&fragment_shader, &shader_source);
    if (test_shader_compilation(&fragment_shader)) {
        return -1;
    }
    free(shader_source);

    GLuint shader_program;
    if (create_shader_program(&shader_program, &vertex_shader, &fragment_shader)) {
        return -1;
    }

    GLint uni_time;
    GLint uni_resolution;
    GLint uni_mouse;
    GLint position_attribute;
    double xpos, ypos, imousex, imousey;

    uni_time = glGetUniformLocation(shader_program, "iTime");
    uni_mouse = glGetUniformLocation(shader_program, "iMouse");
    uni_resolution = glGetUniformLocation(shader_program, "iResolution");
    glUniform3f(uni_resolution, window_width, window_height, 0.0);
    glUniform2f(uni_mouse, window_width/2.0, 0.0);
    position_attribute = glGetAttribLocation(shader_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

    struct timeval start_time;
    struct timeval current_time;
    char buffer[BUF_LEN];
    int events_length, poll_return;
    float seconds_elapsed = 0.0f;
    int inotify_file_descriptor = inotify_init();
    gettimeofday(&start_time, NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    ImVec4 clear_color = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Create a Sphere...");
            ImGui::Text("number of spheres = %i", number_of_spheres);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
            ImGui::SliderFloat("x", &shapes[i].x, -3.0f, 3.0f);
            ImGui::SliderFloat("y", &shapes[i].y, -3.0f, 3.0f);
            ImGui::SliderFloat("z", &shapes[i].z, -3.0f, 3.0f);
            ImGui::SliderFloat("r", &shapes[i].r, 0.0f, 2.0f);
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
        gettimeofday(&current_time, NULL);
        seconds_elapsed = (current_time.tv_sec - start_time.tv_sec) +
                         ((current_time.tv_usec - start_time.tv_usec) / 1000000.0);
        glUniform1f(uni_time, seconds_elapsed);
        glfwGetCursorPos(window, &xpos, &ypos);
        if (xpos > 0.0 && xpos < window_width &&
            ypos > 0.0 && ypos < window_height) {
            imousex = xpos;
            imousey = ypos;
        }
        glUniform2f(uni_mouse, imousex, imousey);

        shader_source = generate_frag_shader(shapes, number_of_spheres);
        compile_fragment_shader(&fragment_shader, &shader_source);
        if (test_shader_compilation(&fragment_shader)) {
            printf("shader compilation failed... continuing.\n");
            continue;
        }
        if (create_shader_program(&shader_program, &vertex_shader, &fragment_shader)) {
            printf("shader compilation failed... continuing.\n");
            continue;
        }
        free(shader_source);
        uni_time = glGetUniformLocation(shader_program, "iTime");
        uni_resolution = glGetUniformLocation(shader_program, "iResolution");
        glUniform3f(uni_resolution, window_width, window_height, 0.0);
        glUniform2f(uni_mouse, imousex, imousey);
        position_attribute = glGetAttribLocation(shader_program, "position");
        glEnableVertexAttribArray(position_attribute);
        glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

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
