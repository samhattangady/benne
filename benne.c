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
#include "ui.h"
#include "cb_ui.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1 * ( EVENT_SIZE + 16 ) )

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Errors: %s\n", description);
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* window;
    int window_height = 900;
    int window_width = 1600;
    window = glfwCreateWindow(window_width, window_height, "benne", NULL, NULL);
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

    GLuint vbo;
    glGenBuffers(1, &vbo);
    GLfloat vertices[] = {
        -1.0f,  3.0f,
        -1.0f, -1.0f,
         3.0f, -1.0f,
    };

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    shader_source_data shader_source_d;

    init_shader_source_data(&shader_source_d);
    compile_vertex_shader(&vertex_shader, &shader_source_d);
    if (test_shader_compilation(&vertex_shader))
        return -1;
    generate_frag_shader(&heap, &shader_source_d);
    compile_fragment_shader(&fragment_shader, &shader_source_d);
    if (test_shader_compilation(&fragment_shader))
        return -1;
    if (create_shader_program(&shader_program, &vertex_shader, &fragment_shader))
        return -1;
    printf("creating instance?\n...");

    GLint uni_time;
    GLint uni_resolution;
    GLint uni_mouse;
    GLint position_attribute;
    double xpos, ypos, imousex, imousey;

    clock_t start_time;
    clock_t current_time;
    clock_t previous_time;
    struct timeval c;
    double frame_time = 0.0;
    char buffer[BUF_LEN];
    int events_length, poll_return;
    float seconds_elapsed = 0.0f;
    int inotify_file_descriptor = inotify_init();
    start_time = clock();
    previous_time = start_time;
    char fps_counter[20];
    int mouse_button_state;
    mouse_state_struct mouse_state = {0, 0.0, 0.0};
    float mouse_dx, mouse_dy;

    // ui_state state = {&heap, 0};

    cb_ui_state benne_ui;
    init_ui(&benne_ui);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        gettimeofday(&c, NULL);

        generate_frag_shader(&heap, &shader_source_d);
        compile_fragment_shader(&fragment_shader, &shader_source_d);
        if (test_shader_compilation(&fragment_shader))
            return -1;
        if (create_shader_program(&shader_program, &vertex_shader, &fragment_shader))
            return -1;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        glfwGetCursorPos(window, &xpos, &ypos);
        imousex = xpos;
        imousey = ypos;

        // gettimeofday(&frag_start, NULL);
        // gettimeofday(&frag_end, NULL);
        // frag_time = (frag_end.tv_sec - frag_start.tv_sec) +
        //             ((frag_end.tv_usec - frag_start.tv_usec) / 1000000.0);
        // printf("took %f seconds to compile fragment shader\n", frag_time);

        mouse_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (mouse_button_state == GLFW_PRESS) {
            if (mouse_state.down == 0)
                mouse_state.down = 1;
            else {
                // rotate the heap[0] object.
                mouse_dx = xpos - mouse_state.down_x;
                mouse_dy = ypos - mouse_state.down_y;
                heap.shapes[0].data[4] -= mouse_dx/window_width*3.1415;
                heap.shapes[0].data[3] -= mouse_dy/window_height*3.1415;
                printf("took %f seconds to compile fragment shader\n", frame_time);
            }
            mouse_state.down_x = xpos;
            mouse_state.down_y = ypos;
        }
        if (mouse_button_state == GLFW_RELEASE)
            mouse_state.down = 0;

        glUseProgram(shader_program);
        glLinkProgram(shader_program);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.2, 0.2, 0.3, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        uni_time = glGetUniformLocation(shader_program, "iTime");
        uni_mouse = glGetUniformLocation(shader_program, "iMouse");
        uni_resolution = glGetUniformLocation(shader_program, "iResolution");
        glUniform1f(uni_time, seconds_elapsed);
        // glUniform2f(uni_mouse, imousex, imousey);
        glUniform2f(uni_mouse, window_width/2.0, window_height/2.0);
        glUniform3f(uni_resolution, window_width, window_height, 0.0);
        position_attribute = glGetAttribLocation(shader_program, "position");
        glEnableVertexAttribArray(position_attribute);
        glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        current_time = clock();
        frame_time = (double) (current_time-previous_time) / CLOCKS_PER_SEC;
        previous_time = current_time;
        seconds_elapsed += frame_time;

        sprintf(&fps_counter, "FPS: %f", 1.0/frame_time);
        cb_ui_render_text(&benne_ui, fps_counter, 16.0, window_height-26.0);

        glfwSwapBuffers(window);
    }

    printf("saving to file\n");
    // simplify_heap(&heap);
    save_heap_to_file(&heap, "savefile.txt");
    free(heap.shapes);
    free(heap.operations);
    free(heap.nodes);
    printf("closing glfw\n");
    glfwDestroyWindow(window);

    printf("terminating glfw\n");
    glfwTerminate();
    printf("exitting benne\n");
    return 0;
}
