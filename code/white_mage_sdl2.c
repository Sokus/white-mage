typedef struct SDL2_Data
{
    SDL_Window *window;
    unsigned int time;
    
    
} SDL2_Data;

SDL2_Data *SDL2_GetBackendData(IO *io)
{
    return (SDL2_Data *)io->platform_backend_data;
}

void SDL2_ProcessEvent(App *app, SDL_Event *event)
{
    switch(event->type)
    {
        case SDL_QUIT:
        {
            app->is_running = false;
        } break;
#if 0
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
case keycode: { app->input.keys_down[(input_key)] = is_down; } break
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
                    app->fullscreen = !app->fullscreen;
                    SDL_WindowFlags flags = (app->fullscreen
                                             ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                             : 0);
                    SDL_SetWindowFullscreen(app->sdl_state.window, flags);
                }
                
                if(alt_is_down && kc == SDLK_F4)
                {
                    app->is_running = false;
                }
            }
        } break;
#endif
    }
}

bool SDL2_Init(IO *io, MemoryArena *memory_arena, SDL_Window *window)
{
    ASSERT(io->platform_backend_data = 0);
    
    SDL2_Data *bd = PUSH_STRUCT(memory_arena, SDL2_Data);
    io->platform_backend_name = "SDL2";
    io->platform_backend_data = (void *)bd;
    
    bd->window = window;
    
    return true;
}

void SDL2_Shutdown(IO *io)
{
    SDL2_Data *bd = SDL2_GetBackendData(io);
    ASSERT(bd != 0);
    
    io->platform_backend_name = 0;
    io->platform_backend_data = 0;
}

void SDL2_NewFrame(IO *io)
{
    SDL2_Data *bd = SDL2_GetBackendData(io);
    ASSERT(bd != 0);
    
    int screen_width, screen_height;
    SDL_GetWindowSize(bd->window, &screen_width, &screen_height);
    io->screen_width = screen_width;
    io->screen_height = screen_height;
    
    // Update Input
}

float SDL2_GetSecondsElapsed(unsigned long int start_counter, unsigned long int end_counter)
{
    unsigned long int counter_elapsed = end_counter - start_counter;
    float result = (float)counter_elapsed / (float)SDL_GetPerformanceFrequency();
    return result;
}