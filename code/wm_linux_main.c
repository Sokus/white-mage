// Platform independent
#include "wm_helpers.h"    
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

#include <fcntl.h> // file control
#include <errno.h>
#include <string.h> // strerror
#include <sys/stat.h>
#include <unistd.h>

#include "wm_linux.h"

#include "wm_game.c"
#include "wm_platform_sdl2.c"
#include "wm_renderer_opengl3.c"

ReadFileResult Linux_ReadEntireFile(char *path)
{
    ReadFileResult result = {0};
    int fd = open(path, O_RDONLY);
    
    if(fd > 0)
    {
        struct stat file_stat;
        fstat(fd, &file_stat);
        uint32_t size = (uint32_t)file_stat.st_size; // NOTE(sokus): This limits our file size
        void *data = valloc(size);
        read(fd, data, size);
        
        result.data = data;
        result.size = size;
    }
    else
    {
        fprintf(stderr, "ERROR: Could not read file %s: %s\n", path, strerror(errno));
    }
    
    return result;
}

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

int main(void)
{
    App app = {0};
    SDL2_Data sdl2_data = {0};
    OpenGL3_Data opengl3_data = {0};
    
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
        OpenGL3_Texture *texture;
        texture = OpenGL3_GetTexture(&opengl3_data, TextureID_Sprites);
        Image sprites = LoadImageEx("../assets/sprites.png", 3);
        OpenGL3_CreateTexture(texture, &memory_arena, &sprites, 8, 8);
        
        texture = OpenGL3_GetTexture(&opengl3_data, TextureID_Glyphs);
        Image glyphs = LoadImageEx("../assets/glyphs.png", 4);
        OpenGL3_CreateTexture(texture, &memory_arena, &glyphs, 8, 8);
    }
    
    SDL2_Init(&sdl2_data, window);
    OpenGL3_Init(&opengl3_data);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    unsigned long int last_counter = SDL_GetPerformanceCounter();
    app.is_running = true;
    while(app.is_running)
    {
        SDL2_NewFrame(&sdl2_data, &app.io);
        OpenGL3_NewFrame(&opengl3_data);
        
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