typedef struct OpenGL3_Texture
{
    bool is_loaded;
    GLuint id;
    
    uint8_t *data;
    int width;
    int height;
    int channels;
    int tile_width;
    int tile_height;
    int tile_count_x;
    int tile_count_y;
    int tile_count;
} OpenGL3_Texture;

typedef struct OpenGL3_Data
{
    char *vendor;
    char *version;
    char *renderer;
    
    unsigned int shader_handle;
    unsigned int vao_handle;
    unsigned int vbo_handle;
    
    OpenGL3_Texture textures[TextureID_Count];
} OpenGL3_Data;

bool OpenGL3_Init(OpenGL3_Data *state)
{
    state->vendor = (char *)glGetString(GL_VENDOR);
    state->renderer = (char *)glGetString(GL_RENDERER);
    state->version = (char *)glGetString(GL_VERSION);
    
    return true;
}

bool CheckShader(GLuint handle, char *description)
{
    int success;
    char info_log[512];
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        fprintf(stderr, 
                "ERROR: OpenGL3_CreateDeviceObjects: "
                "failed to compile %s!\n", description);
        glGetShaderInfoLog(handle, 512, 0, info_log);
        fprintf(stderr, "%s\n", info_log);
    }
    return success;
}

bool CheckProgram(GLuint handle, char *description)
{
    int success;
    char info_log[512];
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if(!success)
    {
        fprintf(stderr, 
                "ERROR: OpenGL3_CreateDeviceObjects: "
                "failed to link %s!\n", description);
        glGetShaderInfoLog(handle, 512, 0, info_log);
        fprintf(stderr, "%s\n", info_log);
    }
    return success;
}

bool OpenGL3_CreateTexture(OpenGL3_Texture *texture,
                           MemoryArena *scratch_arena,
                           uint8_t *data, int width, int height,
                           int channels, int opt_tile_width, int opt_tile_height)
{
    if(texture->is_loaded)
    {
        fprintf(stderr, "ERROR: OpenGL3_Texture already loaded!\n");
        return false;
    }
    
    int tile_width = (opt_tile_width > 0 ? opt_tile_width : width);
    int tile_height = (opt_tile_height > 0 ? opt_tile_height : height);
    int tiles_x = width / tile_width;
    int tiles_y = height / tile_height;
    int tile_count = tiles_x * tiles_y;
    ASSERT(tile_width > 0 && tile_height > 0 && tile_count > 0);
    
    if(width % tile_width != 0)
    {
        fprintf(stderr,
                "ERROR: Texture width (%d) not divisible by tile width (%d)!\n",
                width, tile_width);
        return false;
    }
    
    if(height % tile_height != 0)
    {
        fprintf(stderr,
                "ERROR: Texture height (%d) not divisible by tile height (%d)!\n",
                height, tile_height);
        return false;
    }
    
    GLint format = (channels == 3 ? GL_RGB :
                    channels == 4 ? GL_RGBA : 0);
    
    if(format == 0)
    {
        fprintf(stderr, "ERROR: Channel count (%d) not supported!\n", channels);
        return false;
    }
    
    int size_needed = tile_width * tile_height * channels;
    int size_available = scratch_arena->size - scratch_arena->used;
    
    if(size_needed > size_available)
    {
        fprintf(stderr,
                "ERROR: Not enough space for texture tile in temporary arena.\n"
                "  Available: %d  Needed: %d\n",
                size_available, size_needed);
        return false;
    }
    
    uint8_t *tile_buffer = (uint8_t *)MemoryArenaPushSize(scratch_arena, size_needed);
    
    unsigned int gl_texture_id;
    glGenTextures(1, &gl_texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    
    float texture_border_color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, texture_border_color);
    
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, tile_width, tile_height,
                 tile_count, 0, (GLenum)format, GL_UNSIGNED_BYTE, 0);
    
    int tile_w_stride = channels * tile_width;
    int row_stride = tile_w_stride * tile_count_x;
    for(int tile_y_idx = 0; tile_y_idx < tile_count_y; ++tile_y_idx)
    {
        for(int tile_x_idx = 0; tile_x_idx < tile_count_x; ++tile_x_idx)
        {
            int offset = tile_y_idx * row_stride * tile_height + tile_x_idx * tile_w_stride;
            unsigned char *tile_corner_ptr = data + offset;
            
            for(int tile_pixel_y = 0; tile_pixel_y < tile_height; ++tile_pixel_y)
            {
                unsigned char *src = tile_corner_ptr + tile_pixel_y * row_stride;
                int inverse_tile_pixeL_y = (tile_height - tile_pixel_y - 1);
                unsigned char *dst = tile_buffer + inverse_tile_pixeL_y * tile_w_stride;
                MEMORY_COPY(dst, src, (unsigned int)(tile_w_stride));
            }
            
            int layer_idx = tile_y_idx * tile_count_x + tile_x_idx;
            
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer_idx,
                            tile_width, tile_height, 1, (GLenum)format,
                            GL_UNSIGNED_BYTE, tile_buffer);
        }
    }
    
    MemoryArenaPopSize(scratch_arena, size_needed);
    
    OpenGL3_Texture *gl_texture = OpenGL3_GetTexture(io, texture_id);
    gl_texture->is_loaded = true;
    gl_texture->id = gl_texture_id;
    gl_texture->texture = texture;
}

bool OpenGL3_DestroyTextures(IO *io)
{
    for(int texture_id = 0; texture_id < TextureID_Count; ++texture_id)
    {
        OpenGL3_Texture *gl_texture= OpenGL3_GetTexture(io, texture_id);
        glDeleteTextures(1, &gl_texture->id);
        gl_texture->is_loaded = false;
        gl_texture->id = 0;
        gl_texture->texture = 0;
    }
}

bool OpenGL3_CreateDeviceObjects(IO *io)
{
    OpenGL3_Data *bd = OpenGL3_GetBackendData(io);
    
    const GLchar *vertex_shader = 
        "#version 420 core\n"
        "layout (location = 0) in vec2 a_pos;\n"
        "layout (location = 1) in vec2 a_tex_coord;\n"
        "out vec2 uv;\n"
        "uniform mat4 u_model_to_world;\n"
        "uniform mat4 u_camera_to_clip;\n"
        "void main()\n"
        "{\n"
        "    mat4 transformation = u_camera_to_clip * u_model_to_world;\n"
        "    gl_Position = transformation * vec4(a_pos, 0.0f, 1.0f);\n"
        "    gl_Position += vec4(1.0f, 1.0f, 0.0f, 0.0f);\n"
        "    uv = a_tex_coord;\n"
        "}\0";
    
    const GLchar *fragment_shader = 
        "#version 420 core\n"
        "out vec4 frag_color;\n"
        "in vec2 uv;\n"
        "uniform sampler2DArray u_textures;\n"
        "uniform int u_layer;\n"
        "uniform bool u_flip;\n"
        "void main()\n"
        "{\n"
        "    vec2 uv_p = uv;"
        "    if(u_flip)\n"
        "        uv_p.x = uv.x * -1.0f + 1.0f;\n"
        "    frag_color = texture(u_textures, vec3(uv_p, u_layer));\n"
        "}\0";
    
    GLuint vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_handle, 1, &vertex_shader, 0);
    glCompileShader(vertex_shader_handle);
    CheckShader(vertex_shader_handle, "vertex shader");
    
    GLuint fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_handle, 1, &fragment_shader, 0);
    glCompileShader(fragment_shader_handle);
    CheckShader(fragment_shader_handle, "fragment shader");
    
    bd->shader_handle = glCreateProgram();
    glAttachShader(bd->shader_handle, vertex_shader_handle);
    glAttachShader(bd->shader_handle, fragment_shader_handle);
    glLinkProgram(bd->shader_handle);
    CheckProgram(bd->shader_handle, "shader program");
    
    glDetachShader(bd->shader_handle, vertex_shader_handle);
    glDetachShader(bd->shader_handle, fragment_shader_handle);
    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);
    
    glGenVertexArrays(1, &bd->vao_handle);
    glGenBuffers(1, &bd->vbo_handle);
    
    glBindVertexArray(bd->vao_handle);
    glBindBuffer(GL_ARRAY_BUFFER, bd->vbo_handle);
    
    float quad_vertices[] =
    {
        -0.5f, -0.5f, 0.0f, 0.0f, // lower left
        0.5f, -0.5f, 1.0f, 0.0f,  // rower right
        0.5f, 0.5f, 1.0f, 1.0f,   // upper right
        
        -0.5f, -0.5f, 0.0f, 0.0f, // lower right
        0.5f, 0.5f, 1.0f, 1.0f,   // upper right
        -0.5f, 0.5f, 0.0f, 1.0f   // upper left
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    OpenGL3_CreateTextures(io);
    
    return true;
}

void OpenGL3_DestroyDeviceObjects(IO *io)
{
    OpenGL3_Data *bd = OpenGL3_GetBackendData(io);
    
    if(bd->vao_handle)
    {
        glDeleteBuffers(1, &bd->vao_handle);
        bd->vao_handle = 0;
    }
    
    if(bd->vbo_handle)
    {
        glDeleteBuffers(1, &bd->vbo_handle);
        bd->vbo_handle = 0;
    }
    
    if(bd->shader_handle)
    {
        glDeleteProgram(bd->shader_handle);
        2bd->shader_handle = 0;
    }
}

void OpenGL3_NewFrame(IO *io)
{
    OpenGL3_Data *bd = OpenGL3_GetBackendData(io);
    if(bd->shader_handle == 0)
        OpenGL3_CreateDeviceObjects(io);
}

void SetBoolUniform(unsigned int program, char *name, bool value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1i(uniform_location, (int)value);
}

void SetIntUniform(unsigned int program, char *name, int value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1i(uniform_location, value);
}

void SetFloatUniform(unsigned int program, char *name, float value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1f(uniform_location, value);
}

void SetMat4Uniform(unsigned int program, char *name, mat4 *matrix)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &matrix->elements[0][0]);
}

void OpenGL3_Shutdown(IO *io)
{
    OpenGL3_Data *bd = OpenGL3_GetBackendData(io);
    ASSERT(bd != 0);
    
    OpenGL3_DestroyDeviceObjects(io);
    io->renderer_backend_name = 0;
    io->renderer_backend_data = 0;
}

void OpenGL3_SetupRenderState(IO *io, int width, int height)
{
    OpenGL3_Data *bd = OpenGL3_GetBackendData(io);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // glEnable(GL_SCISSOR_TEST);
    
    glViewport(0, 0, width, height);
    mat4 orthographic = Orthographic(0.0f, (float)width,
                                     0.0f, (float)height,
                                     -1.0f, 1.0f);
    glUseProgram(bd->shader_handle);
    SetMat4Uniform(bd->shader_handle, "u_camera_to_clip", &orthographic);
    glBindVertexArray(bd->vao_handle);
}

#if 0

void RenderSprite(GLState *gl_state, vec2 position, int sprite_id, bool flip)
{
    mat4 model_to_world = Translate(Mat4d(1.0f), position.x, position.y, 0.0f);
    
    glBindVertexArray(gl_state->quad_vao);
    glUseProgram(gl_state->program);
    
    SetMat4Uniform(gl_state->program, "u_model_to_world", &model_to_world);
    SetMat4Uniform(gl_state->program, "u_camera_to_clip", &gl_state->camera_to_clip);
    SetIntUniform(gl_state->program,  "u_layer", sprite_id);
    SetBoolUniform(gl_state->program, "u_flip", flip);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glUseProgram(0);
}
#endif