typedef struct OpenGL3_Data
{
    char *vendor;
    char *renderer;
    char *version;
    
    unsigned int shader_handle;
    unsigned int vao_handle;
    unsigned int vbo_handle;
} OpenGL3_Data;

OpenGL3_Data *OpenGL3_GetBackendData(AppIO *io)
{
    return (OpenGL3_Data *)io->backend_renderer_data;
}

bool OpenGL3_Init(AppIO *io, MemoryArena *memory_arena)
{
    ASSERT(io->backend_renderer_data == NULL);
    
    OpenGL3_Data *bd = PUSH_STRUCT(memory_arena, OpenGL3_Data);
    io->backend_renderer_name = "OpenGL3";
    io->backend_renderer_data = (void *)bd;
    
    bd->vendor = (char *)glGetString(GL_VENDOR);
    bd->renderer = (char *)glGetString(GL_RENDERER);
    bd->version = (char *)glGetString(GL_VERSION);
    
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

bool OpenGL3_CreateDeviceObjects(AppIO *io)
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
    
    return true;
}

void OpenGL3_DestroyDeviceObjects(AppIO *io)
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
        bd->shader_handle = 0;
    }
}

void OpenGL3_NewFrame(AppIO *io)
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

void OpenGL3_Shutdown(AppIO *io)
{
    OpenGL3_Data *bd = OpenGL3_GetBackendData(io);
    ASSERT(bd != 0);
    
    OpenGL3_DestroyDeviceObjects(io);
    io->backend_renderer_name = 0;
    io->backend_renderer_data = 0;
}

void OpenGL3_SetupRenderState(AppIO *io, int width, int height)
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
#define MAX_TILE_SIZE 32
TextureAtlas LoadTextureAtlas(char *path, int tile_w, int tile_h, int channels)
{
    TextureAtlas result = {0};
    
    int width;
    int height;
    int source_channels;
    unsigned char *data = stbi_load(path, &width, &height, &source_channels, channels);
    // stbi_set_flip_vertically_on_load(true);
    if(data)
    {
        int tiles_x = width / tile_w;
        int tiles_y = height / tile_h;
        int tile_count = tiles_x * tiles_y;
        int tile_stride = tile_w * channels;
        int row_stride = tile_stride * tiles_x;
        
        if(width % tile_w != 0 || height % tile_h != 0)
        {
            char *message = "Warning! Texture sizes not a multiple of tile width/height! Texture size: %dx%d, Tile size: %dx%d\n";
            fprintf(stderr, message, width, height, tile_w, tile_h);
        }
        
        if(source_channels != channels)
        {
            fprintf(stderr, "Warning! Texture channel conversion!\n");
        }
        
        ASSERT(channels >= 3 && channels <= 4);
        GLint format = (channels == 3 ? GL_RGB :
                        channels == 4 ? GL_RGBA : GL_RGBA);
        
        unsigned int texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float texture_border_color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, texture_border_color);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, tile_w, tile_h, tile_count, 0,
                     (GLenum)format, GL_UNSIGNED_BYTE, 0);
        
        // TODO(sokus): Push tile data into a bigger chunk of
        // memory to allow tiles larger than 32x32
        unsigned char tile_buffer[MAX_TILE_SIZE*MAX_TILE_SIZE*4];
        ASSERT(tile_w > 0 && tile_h > 0 && tile_w <= MAX_TILE_SIZE && tile_h <= MAX_TILE_SIZE);
        
        for(int iy = 0; iy < tiles_y; ++iy)
        {
            for(int ix = 0; ix < tiles_x; ++ix)
            {
                int offset = iy * row_stride * tile_h + ix * tile_stride;
                unsigned char *tile_corner_ptr = data + offset;
                
                for(int tex_y = 0; tex_y < tile_h; ++tex_y)
                {
                    unsigned char *src = tile_corner_ptr + tex_y * row_stride;
                    unsigned char *dst = tile_buffer + (tile_h - tex_y - 1) * tile_w * channels;
                    MEMORY_COPY(dst, src, (unsigned int)(tile_w * channels));
                }
                
                int layer_idx = iy * tiles_x + ix;
                
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0,
                                layer_idx, tile_w, tile_h, 1, (GLenum)format,
                                GL_UNSIGNED_BYTE, tile_buffer);
            }
        }
        
        result.texture_id = texture_id;
        result.width = width;
        result.height = height;
        result.channels = channels;
        result.tile_w = tile_w;
        result.tile_h = tile_h;
        result.tiles_x = tiles_x;
        result.tiles_y = tiles_y;
        result.tile_count = tile_count;
        
    }
    else
    {
        fprintf(stderr, "Could not find or load texture: %s\n", path);
        INVALID_CODE_PATH;
    }
    stbi_image_free(data);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    return result;
}

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