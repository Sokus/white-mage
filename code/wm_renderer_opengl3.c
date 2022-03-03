GLuint CreateProgram(char *vertex_shader_source, char *fragment_shader_source, char *debug_name)
{
    int success;
    char info_log[512];
    
    GLuint vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_handle, 1, (const char * const *)&vertex_shader_source, 0);
    glCompileShader(vertex_shader_handle);
    glGetShaderiv(vertex_shader_handle, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        fprintf(stderr, "ERROR: %s (vertex compile)\n", debug_name);
        glGetShaderInfoLog(vertex_shader_handle, 512, 0, info_log);
        fprintf(stderr, "%s\n", info_log);
    }
    
    GLuint fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_handle, 1, (const char * const *)&fragment_shader_source, 0);
    glCompileShader(fragment_shader_handle);
    glGetShaderiv(fragment_shader_handle, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        fprintf(stderr, "ERROR: %s (fragment compile)\n", debug_name);
        glGetShaderInfoLog(fragment_shader_handle, 512, 0, info_log);
        fprintf(stderr, "%s\n", info_log);
    }
    
    unsigned int program_handle = glCreateProgram();
    glAttachShader(program_handle, vertex_shader_handle);
    glAttachShader(program_handle, fragment_shader_handle);
    glLinkProgram(program_handle);
    glGetProgramiv(program_handle, GL_LINK_STATUS, &success);
    if(!success)
    {
        fprintf(stderr, "ERROR: %s (program link)\n", debug_name);
        glGetShaderInfoLog(program_handle, 512, 0, info_log);
        fprintf(stderr, "%s\n", info_log);
    }
    
    glDetachShader(program_handle, vertex_shader_handle);
    glDeleteShader(vertex_shader_handle);
    glDetachShader(program_handle, fragment_shader_handle);
    glDeleteShader(fragment_shader_handle);
    
    return program_handle;
    
}

void SetBoolUniform(GLuint program, char *name, bool value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1i(uniform_location, (int)value);
}

void SetIntUniform(GLuint program, char *name, int value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1i(uniform_location, value);
}

void SetFloatUniform(GLuint program, char *name, float value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1f(uniform_location, value);
}

void SetVec3Uniform(GLuint program, char *name, float x, float y, float z)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform3f(uniform_location, x, y, z);
}

void SetMat4Uniform(GLuint program, char *name, mat4 *matrix)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &matrix->elements[0][0]);
}