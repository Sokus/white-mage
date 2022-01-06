// Platform independent
#include "base.h"    
#include "white_mage_platform.h"       // platform-game communication
#include "white_mage_math.h"

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

#include "linux_platform.h"            // core functionality/helpers

#include "white_mage.c"
#include "white_mage_sdl2.c"
#include "white_mage_opengl3.c"

float SDLGetSecondsElapsed(unsigned long int start_counter, unsigned long int end_counter)
{
    unsigned long int counter_elapsed = end_counter - start_counter;
    float result = (float)counter_elapsed / (float)SDL_GetPerformanceFrequency();
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
        float work_in_seconds = SDLGetSecondsElapsed(last_counter, work_counter);
        
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