// Platform independent
#include "base.h"    
#include "game_platform.h"       // platform-game communication
#include "handmade_math.h"

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

#include "sdl_gl_platform.h"            // core functionality/helpers

#include "game.c"
#include "gl_impl.c"

internal void DieSDL(char *message)
{
    fprintf(stderr, "%s: %s\n", message, SDL_GetError());
    exit(2);
}

internal void HandleSDLEvent(App *app, SDL_Event *event)
{
    switch(event->type)
    {
        case SDL_QUIT:
        {
            app->is_running = false;
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
        
        case SDL_WINDOWEVENT:
        {
            switch(event->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    int screen_width, screen_height;
                    SDL_GetWindowSize(app->sdl_state.window, &screen_width, &screen_height); 
                    app->screen_width = screen_width;
                    app->screen_height = screen_height;
                    glViewport(0, 0, screen_width, screen_height);
                } break;
            }
        } break;
    }
}

void InitialiseSDL(char *window_title, int screen_width, int screen_height, SDLState *sdl_state)
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
        DieSDL("Couldn't initialize SDL");
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    // SDL_GL_SetSwapInterval(1);
    
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Window *window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          screen_width, screen_height, window_flags);
    if(window == 0)
        DieSDL("Couldn't create a window");
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if(gl_context == 0)
        DieSDL("Failed to create OpenGL context");
    SDL_GL_MakeCurrent(window, gl_context);
    
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    
    sdl_state->window = window;
    sdl_state->gl_context = gl_context;
}

void InitialiseGL(GLState *gl_state)
{
    gl_state->vendor = (char *)glGetString(GL_VENDOR);
    gl_state->renderer = (char *)glGetString(GL_RENDERER);
    gl_state->version = (char *)glGetString(GL_VERSION);
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    // glClearColor(0, 0, 0, 0);
    glClearColor(46.0f/256.0f, 34.0f/256.0f, 47.0f/256.0f, 1.0f);
    
    char *vertex_shader_source = "#version 420 core\n"
        "layout (location = 0) in vec2 a_pos;\n"
        "layout (location = 1) in vec2 a_tex_coord;\n"
        "out vec2 uv;\n"
        "uniform mat4 u_model_to_world;\n"
        "uniform mat4 u_world_to_clip;\n"
        "void main()\n"
        "{\n"
        "    mat4 transformation = u_world_to_clip * u_model_to_world;\n"
        "    gl_Position = transformation * vec4(a_pos, 0.0f, 1.0f);\n"
        "    uv = a_tex_coord;\n"
        "}\0";
    
    char *fragment_shader_source = "#version 420 core\n"
        "out vec4 frag_color;\n"
        "in vec2 uv;\n"
        "uniform sampler2DArray u_textures;\n"
        "uniform int u_layer;\n"
        "uniform bool u_flip;\n"
        "void main()\n"
        "{\n"
        "    vec2 uv_p = uv;"
        "    if(u_flip)\n"
        "        uv_p.x = uv.x * -1.0f + 1.0f;\n"
        "    frag_color = texture(u_textures, vec3(uv_p, u_layer));\n"
        "}\0";
    
    unsigned int program = CreateProgram(vertex_shader_source, fragment_shader_source);
    
    float quad_vertices[] =
    {
        -0.5f, -0.5f, 0.0f, 0.0f, // lower left
        0.5f, -0.5f, 1.0f, 0.0f,  // rower right
        0.5f, 0.5f, 1.0f, 1.0f,   // upper right
        
        -0.5f, -0.5f, 0.0f, 0.0f, // lower right
        0.5f, 0.5f, 1.0f, 1.0f,   // upper right
        -0.5f, 0.5f, 0.0f, 1.0f   // upper left
    };
    
    unsigned int quad_vbo, quad_vao;
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    
    glBindVertexArray(quad_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    gl_state->quad_vao = quad_vao;
    gl_state->program = program;
}

void RenderSprite(GLState *gl_state, vec2 player_p, int sprite_id, bool flip)
{
    mat4 model_to_world = Translate(Mat4d(1.0f), player_p.x, player_p.y, 0.0f);
    
    glBindVertexArray(gl_state->quad_vao);
    glUseProgram(gl_state->program);
    
    SetMat4Uniform(gl_state->program, "u_model_to_world", &model_to_world);
    SetMat4Uniform(gl_state->program, "u_world_to_clip", &gl_state->world_to_clip);
    SetIntUniform(gl_state->program,  "u_layer", sprite_id);
    SetBoolUniform(gl_state->program, "u_flip", flip);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

int main(void)
{
    App app = {0};
    
    app.screen_width = 960;
    app.screen_height = 540;
    InitialiseSDL("Card Game", app.screen_width, app.screen_height, &app.sdl_state);
    InitialiseGL(&app.gl_state);
    
    TextureAtlas texture_atlas = LoadTextureAtlas("../assets/textures.png", 8, 8, 3);
    //TextureAtlas glyph_atlas = LoadTextureAtlas("../assets/glyphs.png", 8, 8, 4);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_atlas.texture_id);
    
    float game_update_hz = 60.0f;
    float target_seconds_per_frame = 1.0f / game_update_hz;
    float dt = target_seconds_per_frame;
    
    Input *input = &app.input;
    InitializeInput(input);
    
    vec2 player_p = Vec2(0.5f, 0.5f);
    float player_move_timer = 0.0f;
    float player_move_time = 0.35f;
    int player_direction = 1.0f;
    
    vec2 camera_p = Vec2(0.0f, 0.0f);
    
    unsigned long int last_counter = SDL_GetPerformanceCounter();
    app.is_running = true;
    while(app.is_running)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
            HandleSDLEvent(&app, &event);
        
        UpdateInput(input, dt);
        
        // Player update
        player_move_timer -= dt;
        int move_x = 0;
        int move_y = 0;
        if(IsDown(input, InputKey_MoveUp))
            move_y += 1;
        if(IsDown(input, InputKey_MoveLeft))
            move_x -= 1;
        if(IsDown(input, InputKey_MoveDown))
            move_y -= 1;
        if(IsDown(input, InputKey_MoveRight))
            move_x += 1;
        
        float units_per_meter = 8.0f;
        float size_mul = 4.0f;
        float scale = units_per_meter * size_mul;
        UpdateWorldToClipTransformation(&app.gl_state,
                                        camera_p.x, camera_p.y,
                                        app.screen_width, app.screen_height,
                                        scale);
        
        // only allow movement in one direction
        if((move_x != 0 && move_y == 0) || (move_x == 0 && move_y != 0))
        {
            if(player_move_timer <= 0.0f)
            {
                if(move_x != 0)
                    player_direction = move_x;
                
                player_p.x += (float)move_x;
                player_p.y += (float)move_y;
                player_move_timer = player_move_time;
            }
        }
        
        //Rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        bool flip = (player_direction < 0);
        RenderSprite(&app.gl_state, player_p, SpriteID_PlayerBlue, flip);
        
        SDL_GL_SwapWindow(app.sdl_state.window);
        
        // Timing
        unsigned long int work_counter = SDL_GetPerformanceCounter();
        float work_in_seconds = SDLGetSecondsElapsed(last_counter, work_counter);
        
        if(work_in_seconds < target_seconds_per_frame)
        {
            float sec_to_sleep = target_seconds_per_frame - work_in_seconds;
            unsigned int ms_to_sleep = (unsigned int)(sec_to_sleep * 1000.0f) + 1;
            SDL_Delay(ms_to_sleep);
        }
        
        last_counter = SDL_GetPerformanceCounter();
    }
    
    SDL_GL_DeleteContext(app.sdl_state.gl_context);
    SDL_DestroyWindow(app.sdl_state.window);
    SDL_Quit();
    
    return 0;
}