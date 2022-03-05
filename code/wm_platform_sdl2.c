float SDL2_GetSecondsElapsed(unsigned long int start_counter, unsigned long int end_counter)
{
    unsigned long int counter_elapsed = end_counter - start_counter;
    float result = (float)counter_elapsed / (float)SDL_GetPerformanceFrequency();
    return result;
}

void SDL2_ProcessEvent(SDL_Event *event, SDL_Window *window,
                       Input *input, bool *is_running, bool *fullscreen, bool *mouse_relative)
{
    switch(event->type)
    {
        case SDL_QUIT:
        {
            *is_running = false;
        } break;
        
        case SDL_MOUSEMOTION:
        {
            input->mouse_x = event->motion.x;
            input->mouse_y = event->motion.y;
            input->mouse_rel_x += event->motion.xrel;
            input->mouse_rel_y += event->motion.yrel;
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
            
            *mouse_relative = !alt_is_down;
            
            if(event->key.repeat == 0)
            {
#define _SDL_PROCESS_KEYBOARD_MESSAGE(keycode, input_key)\
case keycode: { input->keys_down[(input_key)] = is_down; } break
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
                    *fullscreen = !(*fullscreen);
                    SDL_WindowFlags flags = (*fullscreen
                                             ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                             : 0);
                    SDL_SetWindowFullscreen(window, flags);
                }
                
                if(alt_is_down && kc == SDLK_F4)
                {
                    *is_running = false;
                }
            }
        } break;
    }
}