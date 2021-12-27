// Platform independent
#include "base.h"                // core functionality/helpers
#include "game_platform.h"       // platform-game communication
#include "my_math.h"

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

typedef struct App
{
    bool is_running;
    
    SDL_Window *window;
    bool fullscreen;
    int screen_w;
    int screen_h;
    
    Input input;
} App;

App global_app;

internal void DieSDL(char *message)
{
    fprintf(stderr, "%s: %s\n", message, SDL_GetError());
    exit(2);
}

internal void HandleSDLEvent(SDL_Event *event)
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
                    glViewport(0, 0, global_app.screen_w, global_app.screen_h);
                } break;
            }
        } break;
    }
}

unsigned int CreateProgram(char *vertex_shader_source,
                           char *fragment_shader_source)
{
    int success;
    char info_log[512];
    unsigned int vertex_shader, fragment_shader;
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar **)&vertex_shader_source, 0);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, 0, info_log);
        fprintf(stderr, "glCompileShader(vertex): %s\n", info_log);
        INVALID_CODE_PATH;
    }
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1,(const GLchar **) &fragment_shader_source, 0);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, 0, info_log);
        fprintf(stderr, "glCompileShader(fragment): %s\n", info_log);
        INVALID_CODE_PATH;
    }
    
    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader_program, 512, 0, info_log);
        fprintf(stderr, "glLinkProgram(): %s\n", info_log);
        INVALID_CODE_PATH;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return shader_program;
}

void UseProgram(unsigned int program)
{
    glUseProgram(program);
}

void SetBoolUniform(unsigned int program, char *name, bool value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1i(uniform_location, (int)value);
}

void SetIntUniform(unsigned int program, char *name, int value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1i(uniform_location, value);
}

void SetFloatUniform(unsigned int program, char *name, float value)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniform1f(uniform_location, value);
}

void SetMat4Uniform(unsigned int program, char *name, mat4 *matrix)
{
    GLint uniform_location = glGetUniformLocation(program, name);
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &matrix->elements[0][0]);
}

mat4 Mat4TileUV(int tile_x, int tile_y,
                int texture_width, int texture_height,
                int pixels_per_tile)
{
    float tile_pct_w = (float)pixels_per_tile / (float)texture_width;
    float tile_pct_h = (float)pixels_per_tile / (float)texture_height;
    
    mat4 translate = Translate(Vec3((float)tile_x, (float)tile_y, 0.0f));
    mat4 scale = Scale(Vec3(tile_pct_w, tile_pct_h, 1.0f));
    
    mat4 result = MultiplyMat4(scale, translate);
    return result;
}

typedef enum SpriteID
{
    SpriteID_PlayerRed    = 0,
    SpriteID_PlayerGreen  = 1,
    SpriteID_PlayerBlue   = 2,
    SpriteID_Dwarf_0      = 3,
    SpriteID_Dwarf_1      = 6,
    SpriteID_Dwarf_2      = 9,
    SpriteID_Skeleton_0   = 12,
    SpriteID_Skeleton_1   = 15,
    SpriteID_Wolf         = 18,
    
    SpriteID_Bricks       = 6*16,
    SpriteID_Bush         = 6*16 + 1*3,
    SpriteID_Boulder      = 6*16 + 2*3,
    
    SpriteID_Grass_0      = 8*16,
    SpriteID_Grass_1      = 8*16 + 1*3,
    SpriteID_Grass_2      = 8*16 + 2*3,
    SpriteID_Water_0      = 8*16 + 3*3,
    SpriteID_Water_1      = 8*16 + 4*3,
    SpriteID_Water_2      = 8*16 + 5*3,
    
    SpriteID_Door_0       = 12*16,
    SpriteID_Door_1       = 12*16 + 1*3,
    SpriteID_Chest        = 12*16 + 2*3,
    SpriteID_Table_0      = 12*16 + 3*3,
    SpriteID_Table_1      = 12*16 + 4*3,
    SpriteID_Table_2      = 12*16 + 5*3,
    SpriteID_Chair        = 13*16 + 0*3 + 2,
    SpriteID_Barrel       = 13*16 + 1*3 + 2,
    SpriteID_Bookshelf_0  = 13*16 + 2*3 + 2,
    SpriteID_BookShelf_1  = 13*16 + 3*3 + 2,
    SpriteID_Grave        = 13*16 + 4*3 + 2
} SpriteID;

void InitializeInput(Input *input)
{
    MEMORY_SET(&input->keys_down_duration, -1.0f, sizeof(input->keys_down_duration));
}

void UpdateInput(Input *input, float delta_time)
{
    MEMORY_COPY(input->keys_down_duration_previous,
                input->keys_down_duration,
                sizeof(input->keys_down_duration));
    
    for(int key_idx = 0; key_idx < InputKey_Count; ++key_idx)
    {
        float old_duration = input->keys_down_duration_previous[key_idx];
        bool is_down = input->keys_down[key_idx];
        bool was_down = old_duration >= 0.0f;
        float new_duration = -1.0f;
        if(is_down)
        {
            if(was_down)
                new_duration = old_duration + delta_time;
            else
                new_duration = 0.0f;
        }
        input->keys_down_duration[key_idx] = new_duration;
    }
}

bool IsDown(Input *input, int key_index)
{
    ASSERT(key_index >= 0 && key_index < InputKey_Count);
    bool result = input->keys_down[key_index];
    return result;
}

bool WasDown(Input *input, int key_index)
{
    ASSERT(key_index >= 0 && key_index < InputKey_Count);
    bool result = input->keys_down_duration_previous[key_index] >= 0.0f;
    return result;
}

bool Pressed(Input *input, int key_index)
{
    ASSERT(key_index >= 0 && key_index < InputKey_Count);
    bool result = IsDown(input, key_index) && !WasDown(input, key_index);
    return result;
}

float SDLGetSecondsElapsed(unsigned long int start_counter, unsigned long int end_counter)
{
    unsigned long int counter_elapsed = end_counter - start_counter;
    float result = (float)counter_elapsed / (float)SDL_GetPerformanceFrequency();
    return result;
}

int main(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
        DieSDL("Couldn't initialize SDL");
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    // SDL_GL_SetSwapInterval(1);
    
    global_app.screen_w = 960;
    global_app.screen_h = 540;
    
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Window *window = SDL_CreateWindow("Card Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          global_app.screen_w,
                                          global_app.screen_h,
                                          window_flags);
    
    if(window == 0)
        DieSDL("Couldn't create a window");
    global_app.window = window;
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if(gl_context == 0)
        DieSDL("Failed to create OpenGL context");
    SDL_GL_MakeCurrent(window, gl_context);
    
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    // printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    // printf("Renderer: %s\n", glGetString(GL_RENDERER));
    // printf("Version:  %s\n", glGetString(GL_VERSION));
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // glClearColor(0, 0, 0, 0);
    glClearColor(46.0f/256.0f, 34.0f/256.0f, 47.0f/256.0f, 1.0f);
    
    char *vertex_shader_source = "#version 420 core\n"
        "layout (location = 0) in vec3 a_pos;\n"
        "layout (location = 1) in vec2 a_tex_coord;\n"
        "out vec2 uv;\n"
        "uniform mat4 u_model;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_projection;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0f);\n"
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
    
    
    // create and bind texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    // set parameters (sampling/border)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float texture_border_color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, texture_border_color);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load the texture data
    int texture_w, texture_h, texture_ch;
    // stbi_set_flip_vertically_on_load(true);  
    unsigned char *texture_data = stbi_load("../assets/trinity.png", &texture_w, &texture_h, &texture_ch, 0);
    if(texture_data)
    {
        int tile_w = 8;
        int tile_h = 8;
        int channels = 3;
        
        int tiles_x = 16;
        int tiles_y = 16;
        int tile_count = tiles_x * tiles_y;
        
        unsigned char buffer[8*8*3];
        if(ARRAY_SIZE(buffer) < (unsigned int)(tile_w * tile_h * channels))
        {
            fprintf(stderr, "Insufficient temporary tile buffersize!\n");
            INVALID_CODE_PATH;
        }
        
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB,
                     tile_w, tile_h, tile_count, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, 0);
        
        int tile_stride = tile_w * channels;
        int row_stride = tile_stride * tiles_x;
        
        for(int iy = 0; iy < tiles_y; ++iy)
        {
            for(int ix = 0; ix < tiles_x; ++ix)
            {
                int offset = iy * row_stride * tile_h + ix * tile_stride;
                unsigned char *tile_corner_ptr = texture_data + offset;
                
                for(int tex_y = 0; tex_y < tile_h; ++tex_y)
                {
                    unsigned char *src = tile_corner_ptr + tex_y * row_stride;
                    unsigned char *dst = buffer + (tile_h - tex_y - 1) * tile_w * channels;
                    MEMORY_COPY(dst, src, (unsigned int)(tile_w * channels));
                }
                
                int layer_idx = iy * tiles_x + ix;
                
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0,
                                layer_idx, tile_w, tile_h, 1, GL_RGB,
                                GL_UNSIGNED_BYTE, buffer);
            }
        }
        
        
    }
    else
    {
        fprintf(stderr, "Failed to load texture.\n");
    }
    stbi_image_free(texture_data);
    
    unsigned int program = CreateProgram(vertex_shader_source, fragment_shader_source);
    
    float vertices[] =
    {
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,    1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,    1.0f, 1.0f,
        
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,
        0.5f, 0.5f, 0.0f,    1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f,    0.0f, 1.0f
    };
    
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    UseProgram(program);
    SetIntUniform(program, "u_textures", 0);
    
    float game_update_hz = 60.0f;
    float target_seconds_per_frame = 1.0f / game_update_hz;
    float dt = target_seconds_per_frame;
    
    Input *input = &global_app.input;
    InitializeInput(input);
    
    vec2 player_p = Vec2(0.5f, 0.5f);
    float player_move_timer = 0.0f;
    float player_move_time = 0.35f;
    int player_direction = 1.0f;
    
    unsigned long int last_counter = SDL_GetPerformanceCounter();
    global_app.is_running = true;
    while(global_app.is_running)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            HandleSDLEvent(&event);
        }
        
        UpdateInput(input, dt);
        
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
        
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        UseProgram(program);
        
        float model_size_in_pixels = 8.0f;
        float model_scale = 4.0f;
        vec3 scaling_vector = Vec3(model_size_in_pixels * model_scale,
                                   model_size_in_pixels * model_scale,
                                   1.0f);
        mat4 model = Scale(scaling_vector);
        
        vec3 player_p_vec3 = Vec3(player_p.x, player_p.y, 0.0f);
        mat4 view = Translate(MultiplyVec3(player_p_vec3, scaling_vector));
        
        mat4 projection = Orthographic(0, (float)global_app.screen_w,
                                       0, (float)global_app.screen_h,
                                       -1.0f, 1.0f);
        
        SetMat4Uniform(program, "u_model", &model);
        SetMat4Uniform(program, "u_view", &view);
        SetMat4Uniform(program, "u_projection", &projection);
        
        SetIntUniform(program, "u_layer", SpriteID_PlayerBlue);
        SetBoolUniform(program, "u_flip", player_direction < 0);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        SDL_GL_SwapWindow(window);
        
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
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}