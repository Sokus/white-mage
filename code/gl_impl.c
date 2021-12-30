unsigned int CreateProgram(char *vertex_shader_source,
                           char *fragment_shader_source)
{
    int success;
    char info_log[512];
    unsigned int vertex_shader, fragment_shader;
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar **)&vertex_shader_source, 0);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, 0, info_log);
        fprintf(stderr, "glCompileShader(vertex): %s\n", info_log);
        INVALID_CODE_PATH;
    }
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1,(const GLchar **) &fragment_shader_source, 0);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, 0, info_log);
        fprintf(stderr, "glCompileShader(fragment): %s\n", info_log);
        INVALID_CODE_PATH;
    }
    
    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader_program, 512, 0, info_log);
        fprintf(stderr, "glLinkProgram(): %s\n", info_log);
        INVALID_CODE_PATH;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return shader_program;
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

float SDLGetSecondsElapsed(unsigned long int start_counter, unsigned long int end_counter)
{
    unsigned long int counter_elapsed = end_counter - start_counter;
    float result = (float)counter_elapsed / (float)SDL_GetPerformanceFrequency();
    return result;
}

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

void UpdateWorldToClipTransformation(GLState *gl_state,
                                     float camera_x, float camera_y,
                                     int screen_width, int screen_height,
                                     float scaling_factor)
{
    mat4 projection = Translate(Mat4d(1.0f), -camera_x, -camera_y, 0.0f);
    projection = Scale(projection, scaling_factor, scaling_factor, 1.0f);
    
    mat4 orthographic = Orthographic(0.0f, (float)screen_width,
                                     0.0f, (float)screen_height,
                                     -1.0f, 1.0f);
    projection = MultiplyMat4(orthographic, projection);
    gl_state->world_to_clip = projection;
}


