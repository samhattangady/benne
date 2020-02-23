char* read_file(char *filename);
int compile_vertex_shader(GLuint* vertexShader);
int compile_fragment_shader(GLuint* fragment_shader, GLchar** shader_source);
int test_shader_compilation(GLuint* shader);
int create_shader_program(GLuint* shaderProgram, GLuint* vertexShader, GLuint* fragmentShader);
