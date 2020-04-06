#ifndef BENNE_SHADERS_DEFINED
#define BENNE_SHADERS_DEFINED
#include "benne_string.h"

char* read_file(char *filename);
int compile_vertex_shader(GLuint* vertexShader);
int compile_fragment_shader(GLuint* fragment_shader, string* shader_source);
int test_shader_compilation(GLuint* shader);
int create_shader_program(GLuint* shaderProgram, GLuint* vertexShader, GLuint* fragmentShader);
int compile_and_link_text_shader(uint* vertex_shader, uint* fragment_shader, uint* shader_program);

#endif
