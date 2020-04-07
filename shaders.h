#ifndef BENNE_SHADERS_DEFINED
#define BENNE_SHADERS_DEFINED
#include "cb_string.h"

typedef struct {
    string vertex_source;
    string fragment_source;
    string fragment_header;
    string fragment_computed;
    string fragment_footer;
} shader_source_data;

string read_file(char *filename);

int init_shader_source_data(shader_source_data *source);
int compile_vertex_shader(uint* vertexShader, shader_source_data* source);
int compile_fragment_shader(uint* fragment_shader, shader_source_data* source);
int test_shader_compilation(uint* shader);
int create_shader_program(uint* shaderProgram, uint* vertexShader, uint* fragmentShader);
int compile_and_link_text_shader(uint* vertex_shader, uint* fragment_shader, uint* shader_program);

#endif
