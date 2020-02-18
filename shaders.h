char* readFile(char *filename);
int compileVertexShader(GLuint* vertexShader);
int compileFragmentShader(GLuint* fragmentShader);
int testShaderCompilation(GLuint* shader);
int createShaderProgram(GLuint* shaderProgram, GLuint* vertexShader, GLuint* fragmentShader);
