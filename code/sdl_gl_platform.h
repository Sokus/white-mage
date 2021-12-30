/* date = December 30th 2021 4:52 pm */

#ifndef SDL_GL_PLATFORM_H
#define SDL_GL_PLATFORM_H

typedef struct SDLState
{
    SDL_Window *window;
    SDL_GLContext gl_context;
} SDLState;

typedef struct GLState
{
    char *vendor;
    char *renderer;
    char *version;
    
    mat4 camera_to_clip;
    
    unsigned int quad_vao;
    
    unsigned int program;
} GLState;

typedef struct App
{
    bool is_running;
    
    bool fullscreen;
    int screen_w;
    int screen_h;
    
    Input input;
    
    SDLState sdl_state;
    GLState gl_state;
} App;

#endif //SDL_GL_PLATFORM_H
