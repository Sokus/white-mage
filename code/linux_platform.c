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
        glGetShaderInfoLog(vertex_shader, 512, 0, info_log);
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
    
    global_app.screen_w = 129;
    global_app.screen_h = 129;
    
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
    
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable( GL_LINE_SMOOTH );
    //glEnable( GL_POLYGON_SMOOTH );
    //glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    //glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    
    glClearColor(0, 1, 0, 1);
    // glClearColor(46.0f/256.0f, 34.0f/256.0f, 47.0f/256.0f, 1.0f);
    
    char *vertex_shader_source = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(aPos, 1.0);\n"
        "    TexCoord = aTexCoord;\n"
        "}\0";
    
    char *fragment_shader_source = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture0;\n"
        "void main()\n"
        "{\n"
        "FragColor = texture(texture0, TexCoord);\n"
        "//FragColor = vec4(1, 1, 1, 1);\n"
        "}\0";
    
    
    // create and bind texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set parameters (sampling/border)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float texture_border_color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color);
    GLint filtering = 1 ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    // load the texture data
    int texture_w, texture_h, texture_ch;
    stbi_set_flip_vertically_on_load(true);  
    unsigned char *texture_data = stbi_load("../assets/trinity.png", &texture_w, &texture_h, &texture_ch, 0);
    if(texture_data)
    {
        glTexImage2D(GL_TEXTURE_2D,
                     0,                // mipmap level
                     GL_RGB,           // texture store format
                     texture_w,
                     texture_h,
                     0,
                     GL_RGB,           // texture read format
                     GL_UNSIGNED_BYTE,
                     texture_data);
        // glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        fprintf(stderr, "Failed to load texture.\n");
    }
    stbi_image_free(texture_data);
    
    unsigned int program = CreateProgram(vertex_shader_source, fragment_shader_source);
    
    float pos_x = 0.0f;
    float pos_y = 0.0f;
    float width = 64.0f;
    float height = 64.0f;
    
    vec4 p0 = Vec4(pos_x,             pos_y,             0.0f, 0.0f);
    vec4 p1 = Vec4(pos_x+width,  pos_y,             0.0f, 0.0f);
    vec4 p2 = Vec4(pos_x+width,  pos_y+height, 0.0f, 0.0f);
    vec4 p3 = Vec4(pos_x,             pos_y+height, 0.0f, 0.0f);
    
    float screen_w = (float)global_app.screen_w;
    float screen_h = (float)global_app.screen_h;
    
    mat4 pixel_align = Translate(Vec3(0.0f, 0.0f, 0.0f));
    mat4 orthographic = Orthographic(0, screen_w, 0, screen_h, 0.0f, 100.0f);
    
    mat4 matrix = MultiplyMat4(orthographic, pixel_align);
    
    vec4 p0p = MultiplyMat4ByVec4(matrix, Vec4v(p0.xyz, 1.0f));
    vec4 p1p = MultiplyMat4ByVec4(matrix, Vec4v(p1.xyz, 1.0f));
    vec4 p2p = MultiplyMat4ByVec4(matrix, Vec4v(p2.xyz, 1.0f));
    vec4 p3p = MultiplyMat4ByVec4(matrix, Vec4v(p3.xyz, 1.0f));
    
    float tile_size = 8.0f;
    float texel_w = 1.0f/(float)texture_w;
    float texel_h = 1.0f/(float)texture_h;
    float tile_x = 0; // 10
    float tile_y = 8; // 8
    
    float offset_x = -0.0f * texel_w;
    float offset_y = 0.0f * texel_h;
    
    vec2 t0 = Vec2(tile_x * tile_size * texel_w + offset_x,     tile_y * tile_size * texel_h + offset_y);
    vec2 t1 = Vec2((tile_x+1) * tile_size * texel_w + offset_x, (tile_y+1) * tile_size * texel_h + offset_y);
    
    float vertices [] =
    {
        p0p.x, p0p.y, 0.0f,    t0.x, t0.y,
        p1p.x, p1p.y, 0.0f,    t1.x, t0.y,
        p2p.x, p2p.y, 0.0f,    t1.x, t1.y,
        
        p0p.x, p0p.y, 0.0f,    t0.x, t0.y,
        p2p.x, p2p.y, 0.0f,    t1.x, t1.y,
        p3p.x, p3p.y, 0.0f,    t0.x, t1.y
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
    SetIntUniform(program, "texture0", 0);
    
    // Input *input = &global_app.input;
    
    global_app.is_running = true;
    while(global_app.is_running)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            HandleSDLEvent(&event);
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        UseProgram(program);
        glBindVertexArray(VAO);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        SDL_GL_SwapWindow(window);
    }
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}