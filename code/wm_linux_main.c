// Platform independent
#include "base.h"    
#include "wm_platform.h"       // platform-game communication
#include "wm_math.h"

// External
#include "SDL2/SDL.h"            // window/context creation
#include "glad/glad.h"           // GL loader

// stb_image is giving us bad time...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wconversion"
#define STB_IMAGE_IMPLEMENTATION // required by stb_image
#include "stb/stb_image.h"       // reading images
#pragma GCC diagnostic pop

// Platform specific/temporary
#include <stdio.h>

#include "wm_linux.h"

#include "wm_game.c"
#include "wm_platform_sdl2.c"
#include "wm_renderer_opengl3.c"

bool LoadTextureAtlas(Texture *texture, char *path,
                      int tile_w, int tile_h, int channels)
{
    int width;
    int height;
    int source_channels;
    unsigned char *data = stbi_load(path, &width, &height, &source_channels, channels);
    
    if(data)
    {
        int tiles_x = width / tile_w;
        int tiles_y = height / tile_h;
        int tile_count = tiles_x * tiles_y;
        
        if(width % tile_w != 0 || height % tile_h != 0)
        {
            fprintf(stderr,
                    "WARNING: %s\n"
                    "Texture size not a multiple of tile width/height!\n"
                    "Texture size: %dx%d, Tile size: %dx%d\n",
                    path, width, height, tile_w, tile_h);
        }
        
        if(channels != 0 && source_channels != channels)
        {
            fprintf(stderr,
                    "WARNING: %s\n"
                    "Texture conversion from %d to %d channels!\n",
                    path, source_channels, channels);
        }
        
        texture->is_loaded = true;
        texture->data = data;
        texture->width = width;
        texture->height = height;
        texture->channels = channels;
        texture->tile_w = tile_w;
        texture->tile_h = tile_h;
        texture->tile_count = tile_count;
    }
    else
    {
        fprintf(stderr, "ERROR: Could not load texture: %s\n", path);
        texture->is_loaded = false;
        return false;
    }
    
    return true;
}

void UnloadTexture(Texture *texture)
{
    stbi_image_free(texture->data);
    texture->is_loaded = false;
}

Texture *GetTexture(IO *io, TextureID texture_id)
{
    ASSERT(texture_id >= 0 && texture_id < TextureID_Count);
    Texture *result = (io->textures + TextureID_Count);
    return result;
}

int main(void)
{
    App app = {0};
    
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "error: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL
                                                     | SDL_WINDOW_RESIZABLE
                                                     | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Card Game",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          960, 540,
                                          window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    // SDL_GL_SetSwapInterval(1);
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    
    MemoryArena memory_arena;
    unsigned char memory_buffer[4096];
    InitializeArena(&memory_arena, memory_buffer, sizeof(memory_buffer));
    
    float target_frames_per_seconds = 60.0f;
    app.io.target_frames_per_second = target_frames_per_seconds;
    app.io.delta_time = 1.0f / target_frames_per_seconds;
    
    {
        Texture *sprites_texture = GetTexture(&app.io, TextureID_Sprites);
        LoadTextureAtlas(sprites_texture, "../assets/sprites.png", 8, 8, 3);
        
        Texture *glyphs_texture = GetTexture(&app.io, TextureID_Glyphs);
        LoadTextureAtlas(sprites_texture, "../assets/glyphs.png", 8, 8, 4);
    }
    
    SDL2_Init(&app.io, &memory_arena, window);
    OpenGL3_Init(&app.io, &memory_arena);
    
    unsigned long int last_counter = SDL_GetPerformanceCounter();
    app.is_running = true;
    while(app.is_running)
    {
        SDL2_NewFrame(&app.io);
        OpenGL3_NewFrame(&app.io);
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
            SDL2_ProcessEvent(&app, &event);
        
        glClearColor(46.0f/256.0f, 34.0f/256.0f, 47.0f/256.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        SDL_GL_SwapWindow(window);
        
        // Timing
        unsigned long int work_counter = SDL_GetPerformanceCounter();
        float work_in_seconds = SDL2_GetSecondsElapsed(last_counter, work_counter);
        
        if(work_in_seconds < app.io.delta_time)
        {
            float sec_to_sleep = app.io.delta_time - work_in_seconds;
            unsigned int ms_to_sleep = (unsigned int)(sec_to_sleep * 1000.0f) + 1;
            SDL_Delay(ms_to_sleep);
        }
        
        last_counter = SDL_GetPerformanceCounter();
    }
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}