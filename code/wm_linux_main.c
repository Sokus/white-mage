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

ReadFileResult Linux_ReadEntireFile(char *path, bool end_with_zero)
{
    ReadFileResult result = {0};
    int fd = open(path, O_RDONLY);
    
    if(fd > 0)
    {
        struct stat file_stat;
        fstat(fd, &file_stat);
        uint32_t size = (uint32_t)file_stat.st_size; // NOTE(sokus): This limits our file size
        if(end_with_zero)
            size++;
        void *data = valloc(size);
        read(fd, data, size);
        if(end_with_zero)
            *((char *)data + size - 1) = 0;
        
        result.data = data;
        result.size = size;
    }
    else
    {
        fprintf(stderr, "ERROR: Could not read file %s: %s\n", path, strerror(errno));
    }
    
    return result;
}

int main(void)
{
    bool is_running = false;
    bool fullscreen = false;
    bool mouse_relative = true;
    float target_fps = 60.0f;
    float dt = 1.0f / target_fps;
    
    int screen_width = 0;
    int screen_height = 0;
    
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
    SDL_Window *window = SDL_CreateWindow("White Mage",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          960, 540,
                                          window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    // SDL_GL_SetSwapInterval(1);
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
    
    GLuint standard_program, light_program;
    {
        ReadFileResult standard_vs = Linux_ReadEntireFile("../code/shaders/standard.vs", true);
        ReadFileResult standard_fs = Linux_ReadEntireFile("../code/shaders/standard.fs", true);
        standard_program = CreateProgram((char *)standard_vs.data, (char *)standard_fs.data, "standard shader");
        free(standard_vs.data); free(standard_fs.data);
        
        ReadFileResult light_vs = Linux_ReadEntireFile("../code/shaders/light.vs", true);
        ReadFileResult light_fs = Linux_ReadEntireFile("../code/shaders/light.fs", true);
        light_program = CreateProgram((char *)light_vs.data, (char *)light_fs.data, "light shader");
        free(light_vs.data); free(light_fs.data);
    }
    
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    
    
    GLuint cube_vao, vbo;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(cube_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    GLuint light_vao;
    glGenVertexArrays(1, &light_vao);
    glBindVertexArray(light_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    
    Input input = {0};
    InitializeInput(&input);
    
    Camera camera = {0};
    InitializeCamera(&camera, 1.5f, 1.1f, 5.0f, -90.0f, 0.0f, 0.2f, 1.0f);
    
    MemoryArena memory_arena;
    unsigned char memory_buffer[4096];
    InitializeArena(&memory_arena, memory_buffer, sizeof(memory_buffer));
    
    unsigned long int last_counter = SDL_GetPerformanceCounter();
    is_running = true;
    while(is_running)
    {
        SDL_GetWindowSize(window, &screen_width, &screen_height);
        glViewport(0, 0, screen_width, screen_height);
        
        input.mouse_rel_x = 0;
        input.mouse_rel_y = 0;
        SDL_Event event;
        while(SDL_PollEvent(&event))
            SDL2_ProcessEvent(&event, window, &input, &is_running, &fullscreen, &mouse_relative);
        SDL_SetRelativeMouseMode(mouse_relative);
        
        UpdateInput(&input, dt);
        
        if(mouse_relative)
            ProcessMouse(&camera, (float)input.mouse_rel_x, (float)input.mouse_rel_y);
        
        vec3 light_pos = Vec3(1.5f, 2.0f, 1.0f);
        int move_x = (int)IsDown(&input, InputKey_MoveRight) - (int)IsDown(&input, InputKey_MoveLeft);
        int move_z = (int)IsDown(&input, InputKey_MoveUp) - (int)IsDown(&input, InputKey_MoveDown);
        
        float move_speed = 2.0f;
        float move_amount_x = (float)move_x * move_speed;
        float move_amount_z = (float)move_z * move_speed;
        
        MoveCameraRelative(&camera, move_amount_z, move_amount_x, dt);
        
        //glClearColor(46.0f/256.0f, 34.0f/256.0f, 47.0f/256.0f, 1.0f);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(standard_program);
        SetVec3Uniform(standard_program, "objectColor", 1.0f, 0.5f, 0.31f);
        SetVec3Uniform(standard_program, "lightColor", 1.0f, 1.0f, 1.0f);
        SetVec3Uniform(standard_program, "lightPos", light_pos.x, light_pos.y, light_pos.z);
        
        // view/projection transformations
        mat4 view = GetCameraViewMatrix(&camera);
        mat4 projection = Perspective(40.0f, (float)screen_width / (float)screen_height, 0.1f, 100.0f);
        SetMat4Uniform(standard_program, "projection", &projection);
        SetMat4Uniform(standard_program, "view", &view);
        
        // world transformation
        mat4 model = Mat4d(1.0f);
        SetMat4Uniform(standard_program, "model", &model);
        
        // render the cube
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        
        // also draw the lamp object
        glUseProgram(light_program);
        SetMat4Uniform(light_program, "projection", &projection);
        SetMat4Uniform(light_program, "view", &view);
        model = Mat4d(1.0f);
        model = Scale(model, 0.2f, 0.2f, 0.2f);
        model = Translate(model, light_pos.x, light_pos.y, light_pos.z);
        SetMat4Uniform(light_program, "model", &model);
        
        glBindVertexArray(light_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        
        SDL_GL_SwapWindow(window);
        
        // Timing
        unsigned long int work_counter = SDL_GetPerformanceCounter();
        float work_in_seconds = SDL2_GetSecondsElapsed(last_counter, work_counter);
        
        if(work_in_seconds < dt)
        {
            float sec_to_sleep = dt - work_in_seconds;
            unsigned int ms_to_sleep = (unsigned int)(sec_to_sleep * 1000.0f) + 1;
            SDL_Delay(ms_to_sleep);
        }
        
        last_counter = SDL_GetPerformanceCounter();
    }
    
    glDeleteBuffers(1, &cube_vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(standard_program);
    glDeleteProgram(light_program);
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}