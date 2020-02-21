char* read_file(char *filename);
int compileVertexShader(GLuint* vertexShader);
int compile_fragment_shader(GLuint* fragment_shader, GLchar** shader_source);
int testShaderCompilation(GLuint* shader);
int createShaderProgram(GLuint* shaderProgram, GLuint* vertexShader, GLuint* fragmentShader);
