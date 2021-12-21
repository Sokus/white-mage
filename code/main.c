#include "SDL.h"
#include "SDL_opengl.h"

#include <stdbool.h>
#include <stdio.h>

bool global_running;

int main(void)
{
    int width = 960;
    int height = 540;
    
    // Setup SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Window *window = SDL_CreateWindow("Card Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          width, height,
                                          window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // enable vsync
    
    global_running = true;
    
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    while(global_running)
    {
        SDL_Event event;
        while( SDL_PollEvent( &event ) ) {
            switch( event.type ) {
                case SDL_KEYDOWN:
                {
                    SDL_Keysym *keysym = &event.key.keysym;
                    
                    switch( keysym->sym ) {
                        case SDLK_ESCAPE:
                        {
                            global_running = false;
                        } break;
                        
                        default: break;
                    }
                } break;
                
                case SDL_QUIT:
                {
                    global_running = false;
                } break;
            }
        }
        
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        SDL_GL_SwapWindow(window);
    }
    
    
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}