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

ReadFileResult Linux_ReadEntireFile(char *path)
{
    ReadFileResult result = {0};
    int fd = open(path, O_RDONLY);
    
    if(fd > 0)
    {
        struct stat file_stat;
        fstat(fd, &file_stat);
        uint32_t size = (uint32_t)file_stat.st_size; // NOTE(sokus): This limits our file size
        void *data = valloc(size);
        read(fd, data, size);
        
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
    //bool fullscreen = false;
    float target_fps = 30.0f;
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
    
    char *standard_vs =
        "#version 420 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aNormal;\n"
        "\n"
        "out vec3 FragPos;\n"
        "out vec3 Normal;\n"
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
        "}\n";
    
    char *standard_fs =
        "#version 420 core\n"
        "out vec4 FragColor;\n"
        "\n"
        "in vec3 Normal;  \n"
        "in vec3 FragPos;  \n"
        "\n"
        "uniform vec3 lightPos; \n"
        "uniform vec3 viewPos; \n"
        "uniform vec3 lightColor;\n"
        "uniform vec3 objectColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    // ambient\n"
        "    float ambientStrength = 0.1;\n"
        "    vec3 ambient = ambientStrength * lightColor;\n"
        "\n"
        "    // diffuse \n"
        "    vec3 norm = normalize(Normal);\n"
        "    vec3 lightDir = normalize(lightPos - FragPos);\n"
        "    float diff = max(dot(norm, lightDir), 0.0);\n"
        "    vec3 diffuse = diff * lightColor;\n"
        "\n"
        "    // specular\n"
        "    float specularStrength = 0.5;\n"
        "    vec3 viewDir = normalize(viewPos - FragPos);\n"
        "    vec3 reflectDir = reflect(-lightDir, norm);  \n"
        "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
        "    vec3 specular = specularStrength * spec * lightColor;  \n"
        "\n"
        "    vec3 result = (ambient + diffuse + specular) * objectColor;\n"
        "    FragColor = vec4(result, 1.0);\n"
        "} \n";
    
    char *light_vs =
        "#version 420 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\n";
    
    char *light_fs =
        "#version 420 core\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0); // set alle 4 vector values to 1.0\n"
        "}\n";
    
    GLuint standard_program = CreateProgram(standard_vs, standard_fs, "standard shader");
    GLuint light_program = CreateProgram(light_vs, light_fs, "light shader");
    
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
    
    
    
    
    MemoryArena memory_arena;
    unsigned char memory_buffer[4096];
    InitializeArena(&memory_arena, memory_buffer, sizeof(memory_buffer));
    
    unsigned long int last_counter = SDL_GetPerformanceCounter();
    is_running = true;
    while(is_running)
    {
        SDL_GetWindowSize(window, &screen_width, &screen_height);
        glViewport(0, 0, screen_width, screen_height);
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
            SDL2_ProcessEvent(&event, &is_running);
        
        glClearColor(46.0f/256.0f, 34.0f/256.0f, 47.0f/256.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        vec3 light_pos = Vec3(5.0f, 0.0f, 3.0f);
        vec3 view_pos = Vec3(1.5f, 1.1f, 4.0f);
        
        glUseProgram(standard_program);
        SetVec3Uniform(standard_program, "objectColor", 1.0f, 0.5f, 0.31f);
        SetVec3Uniform(standard_program, "lightColor", 1.0f, 1.0f, 1.0f);
        SetVec3Uniform(standard_program, "lightPos", light_pos.x, light_pos.y, light_pos.z);
        SetVec3Uniform(standard_program, "viewPos", view_pos.x, view_pos.y, view_pos.z);
        
        // view/projection transformations
        mat4 projection = Perspective(60.0f, (float)screen_width / (float)screen_height, 0.1f, 100.0f);
        mat4 view = Translate(Mat4d(1.0f), -view_pos.x, -view_pos.y, -view_pos.z);
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
        model = Translate(model, light_pos.x, light_pos.y, light_pos.z);
        model = Scale(model, 0.2f, 0.2f, 0.2f);
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