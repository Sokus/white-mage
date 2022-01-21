/* date = December 30th 2021 4:52 pm */

#ifndef WM_LINUX_H
#define WM_LINUX_H

typedef struct IO
{
    float target_frames_per_second;
    float delta_time;
    
    int screen_width;
    int screen_height;
    
    char *platform_backend_name;
    char *renderer_backend_name;
    void *platform_backend_data;
    void *renderer_backend_data;
    
    Texture textures[TextureID_Count];
} IO;

typedef struct App
{
    bool is_running;
    bool fullscreen;
    
    IO io;
} App;



#endif //WM_LINUX_H
