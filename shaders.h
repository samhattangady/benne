char* readFile(char *filename);
int compileVertexShader(GLuint* vertexShader);
int compileFragmentShader(GLuint* fragmentShader, GLchar** shaderSource);
int testShaderCompilation(GLuint* shader);
int createShaderProgram(GLuint* shaderProgram, GLuint* vertexShader, GLuint* fragmentShader);
