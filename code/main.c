#include "base.h"

#include "SDL.h"
#include "SDL_opengl.h"

#include <stdio.h>

typedef enum InputKey
{
    InputKey_MoveUp,      // W
    InputKey_MoveLeft,    // A
    InputKey_MoveDown,    // S
    InputKey_MoveRight,   // D
    InputKey_ActionUp,    // R
    InputKey_ActionLeft,  // F
    InputKey_ActionDown,  // E
    InputKey_ActionRight, // Q
    InputKey_Select,      // Tab
    InputKey_Start,       // Esc
    
    InputKey_Count,
} InputKey;

char *InputKeyName(InputKey input_key)
{
    switch(input_key)
    {
        case InputKey_MoveUp:      return "MoveUp";
        case InputKey_MoveLeft:    return "MoveLeft";
        case InputKey_MoveDown:    return "MoveDown";
        case InputKey_MoveRight:   return "MoveRight";   
        case InputKey_ActionUp:    return "ActionUp";
        case InputKey_ActionLeft:  return "ActionLeft";
        case InputKey_ActionDown:  return "ActionDown";
        case InputKey_ActionRight: return "ActionRight";
        case InputKey_Select:      return "Select";
        case InputKey_Start:       return "Start";
        default:                    return "Invalid";
    }
}

typedef struct Input
{
    bool keys_down[InputKey_Count];
} Input;

typedef struct App
{
    bool is_running;
    
    SDL_Window *window;
    bool fullscreen;
    int screen_w;
    int screen_h;
    
    Input input;
} App;

App global_app = {0};

internal void SDLHandleEvent(SDL_Event *event)
{
    switch(event->type)
    {
        case SDL_QUIT:
        {
            global_app.is_running = false;
        } break;
        
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_Keycode kc = event->key.keysym.sym;
            bool is_down = (event->key.state == SDL_PRESSED);
            
            uint16_t mod = event->key.keysym.mod;
            // bool lshift_is_down = (mod & KMOD_LSHIFT);
            // bool ctrl_is_down = (mod & KMOD_CTRL);
            bool alt_is_down = (mod & KMOD_ALT);
            
            if(event->key.repeat == 0)
            {
#define _SDL_PROCESS_KEYBOARD_MESSAGE(keycode, input_key)\
case keycode: { global_app.input.keys_down[(input_key)] = is_down; } break
                switch(kc)
                {
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_w,      InputKey_MoveUp);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_a,      InputKey_MoveLeft);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_s,      InputKey_MoveDown);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_d,      InputKey_MoveRight);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_r,      InputKey_ActionUp);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_f,      InputKey_ActionLeft);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_e,      InputKey_ActionDown);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_q,      InputKey_ActionRight);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_TAB,    InputKey_Select);
                    _SDL_PROCESS_KEYBOARD_MESSAGE(SDLK_ESCAPE, InputKey_Start);
                }
#undef _SDL_PROCESS_KEYBOARD_MESSAGE
            }
            
            if(event->type == SDL_KEYDOWN)
            {
                if((alt_is_down && kc == SDLK_RETURN)
                   || (kc == SDLK_F11))
                {
                    global_app.fullscreen = !global_app.fullscreen;
                    SDL_WindowFlags flags = (global_app.fullscreen
                                             ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                             : 0);
                    SDL_SetWindowFullscreen(global_app.window, flags);
                }
                
                if(alt_is_down && kc == SDLK_F4)
                {
                    global_app.is_running = false;
                }
            }
        } break;
        
        case SDL_WINDOWEVENT:
        {
            switch(event->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    SDL_GetWindowSize(global_app.window,
                                      &global_app.screen_w,
                                      &global_app.screen_h);
                } break;
            }
        } break;
    }
}



int main(void)
{
    global_app.screen_w = 960;
    global_app.screen_h = 540;
    
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
                                          global_app.screen_w,
                                          global_app.screen_h,
                                          window_flags);
    
    if(window != 0)
    {
        global_app.is_running = true;
        global_app.window = window;
        
        SDL_GLContext gl_context = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, gl_context);
        SDL_GL_SetSwapInterval(1); // enable vsync
        
        glViewport(0, 0, global_app.screen_w, global_app.screen_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        
        Input *input = &global_app.input;
        
        while(global_app.is_running)
        {
            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                SDLHandleEvent(&event);
            }
            
            for(unsigned int idx = 0; idx < InputKey_Count; ++idx)
            {
                if(input->keys_down[idx])
                {
                    printf("%s\n", InputKeyName(idx));
                }
            }
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            SDL_GL_SwapWindow(window);
        }
        
        
        
        SDL_GL_DeleteContext(gl_context);
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}