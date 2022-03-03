bool OpenGL3_CreateTexture(OpenGL3_Texture *texture,
                           MemoryArena *scratch_arena,
                           Image *image,
                           int opt_tile_width, int opt_tile_height)
{
    if(texture->is_loaded)
    {
        fprintf(stderr, "ERROR: OpenGL3_Texture already loaded!\n");
        return false;
    }
    
    uint8_t *data = image->data;
    int width = image->width;
    int height = image->height;
    int channels = image->channels;
    
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
    
    size_t size_needed = (size_t)(tile_width * tile_height * channels);
    size_t size_available = scratch_arena->size - scratch_arena->used;
    
    if(size_needed > size_available)
    {
        fprintf(stderr,
                "ERROR: Not enough space for texture tile in temporary arena.\n"
                "  Available: %u  Needed: %u\n",
                (unsigned int)size_available, (unsigned int)size_needed);
        return false;
    }
    
    uint8_t *tile_buffer = (uint8_t *)MemoryArenaPushSize(scratch_arena, size_needed);
    
    unsigned int gl_texture_id;
    glGenTextures(1, &gl_texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, gl_texture_id);
    
    float texture_border_color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, texture_border_color);
    
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, tile_width, tile_height,
                 tile_count, 0, (GLenum)format, GL_UNSIGNED_BYTE, 0);
    
    int tile_w_stride = channels * tile_width;
    int row_stride = tile_w_stride * tiles_x;
    for(int tile_y_idx = 0; tile_y_idx < tiles_y; ++tile_y_idx)
    {
        for(int tile_x_idx = 0; tile_x_idx < tiles_x; ++tile_x_idx)
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
            
            int layer_idx = tile_y_idx * tiles_x + tile_x_idx;
            
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer_idx,
                            tile_width, tile_height, 1, (GLenum)format,
                            GL_UNSIGNED_BYTE, tile_buffer);
        }
    }
    
    MemoryArenaPopSize(scratch_arena, size_needed);
    
    texture->is_loaded = true;
    texture->id = gl_texture_id;
    texture->width = width;
    texture->height = height;
    texture->channels = channels;
    texture->tile_width = tile_width;
    texture->tile_height = tile_height;
    texture->tile_count_x = tiles_x;
    texture->tile_count_y = tiles_y;
    texture->tile_count = tile_count;
    
    return true;
}

void OpenGL3_DestroyTextures(OpenGL3_Data *data)
{
    for(int texture_id = 0; texture_id < TextureID_Count; ++texture_id)
    {
        OpenGL3_Texture *texture = data->textures + texture_id;
        glDeleteTextures(1, &texture->id);
    }
}

typedef struct Image
{
    uint8_t *data;
    int width;
    int height;
    int channels;
} Image;

Image LoadImageEx(char *path, int opt_force_channels)
{
    Image result = {0};
    
    int width, height, channels, src_channels = 0;
    uint8_t *data = stbi_load(path, &width, &height, &src_channels, opt_force_channels);
    
    if(data)
    {
        channels = (opt_force_channels != 0 ? opt_force_channels : src_channels);
        result.data = data;
        result.width = width;
        result.height = height;
        result.channels = channels;
    }
    else
    {
        fprintf(stderr, "ERROR: Could not load texture: %s\n", path);
    }
    
    return result;
}

void UnloadImage(Image *image)
{
    stbi_image_free(image->data);
    MEMORY_SET(image, 0, sizeof(Image));
}
