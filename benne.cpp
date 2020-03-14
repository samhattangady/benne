#include "imgui_pch.h"
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
#include "distance_fields.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1 * ( EVENT_SIZE + 16 ) )

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Errors: %s\n", description);
}

void add_child(df_heap* heap, unsigned int parent_index) {
    attach_node(heap,
        generate_sphere(
            heap,
            0.3, 0.0, 0.0,
            0.1, 1.0
        ),
        { DISTANCE_FIELD_BLEND_ADD, 0.1 },
        parent_index
    );
}

int get_op_id(df_operation_enum op) {
    switch (op) {
        case DISTANCE_FIELD_BLEND_ADD: return 0;
        case DISTANCE_FIELD_BLEND_SUBTRACT: return 1;
        case DISTANCE_FIELD_BLEND_UNION: return 2;
    }
}

void draw_node_editor(df_heap* heap, unsigned int index) {
    string name = string_from("Shape");
    append_sprintf(&name, " %i", index);
    df_node* node = &heap->nodes[index];
    df_operation* operation = &heap->operations[index];
    if (ImGui::TreeNode(name.text)) {
        int op_id = get_op_id(operation->operation);
        ImGui::Combo("Op", &op_id, "Add\0Subtract\0Union\0");
        switch (op_id) {
            case 0: operation->operation = DISTANCE_FIELD_BLEND_ADD; break;
            case 1: operation->operation = DISTANCE_FIELD_BLEND_SUBTRACT; break;
            case 2: operation->operation = DISTANCE_FIELD_BLEND_UNION; break;
        }
        float* shape_data = heap->shapes[index].data;
        ImGui::SliderFloat("Ex", &(operation->extent), -0.0, 1.0);
        ImGui::SliderFloat3("Position", &shape_data[0], -1.0, 1.0);
        switch(heap->shapes[index].type) {
            case SPHERE:
                ImGui::SliderFloat("Radius", &shape_data[3], -0.0, 2.0);
                break;
            case ROUNDED_RECTANGLE:
                ImGui::SliderFloat3("Size", &shape_data[3], 0.0, 1.0);
                ImGui::SliderFloat3("Angle", &(shape_data[6]), -4.0, 4.0);
                ImGui::SliderFloat("Radius", &(shape_data[9]), -0.0, 2.0);
                break;
        }
        for (int i=0; i<node->size; i++)
            draw_node_editor(heap, node->children[i]);
        if (ImGui::Button("Add shape"))
            add_child(heap, index);
        ImGui::SameLine();
        if (index > 0) {
            if (ImGui::Button("Delete shape"))
                dispose_node(heap, index);
        }
        ImGui::TreePop();
    }
    dispose_string(&name);
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

    unsigned int heap_size = 128;
    df_shape* shapes_mem = (df_shape*) malloc(sizeof(df_shape) * heap_size);
    df_operation* ops_mem = (df_operation*) malloc(sizeof(df_operation) * heap_size);
    df_node* nodes_mem = (df_node*) malloc(sizeof(df_node) * heap_size);
    df_heap heap = { heap_size, 0, shapes_mem, ops_mem, nodes_mem };
    load_heap_from_file(&heap, "savefile.txt");

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
    GLuint fragment_shader;
    GLuint shader_program;

    compile_vertex_shader(&vertex_shader);
    if (test_shader_compilation(&vertex_shader))
        return -1;
    string shader_source = generate_frag_shader(&heap);
    compile_fragment_shader(&fragment_shader, &shader_source);
    if (test_shader_compilation(&fragment_shader))
        return -1;
    dispose_string(&shader_source);
    if (create_shader_program(&shader_program, &vertex_shader, &fragment_shader))
        return -1;

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
        ImGui::ShowDemoWindow();

        ImGui::Begin("Edit shapes.");
        if (ImGui::Button("print shader")) {
            string shader = generate_frag_shader(&heap);
            printf(shader.text);
            dispose_string(&shader);
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        draw_node_editor(&heap, 0);
        ImGui::End();

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

        shader_source = generate_frag_shader(&heap);
        compile_fragment_shader(&fragment_shader, &shader_source);
        if (test_shader_compilation(&fragment_shader))
            return -1;
        dispose_string(&shader_source);
        if (create_shader_program(&shader_program, &vertex_shader, &fragment_shader))
            return -1;
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
    save_heap_to_file(&heap, "savefile.txt");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
