/* date = December 30th 2021 4:52 pm */

#ifndef LINUX_PLATFORM_H
#define LINUX_PLATFORM_H

typedef struct AppIO
{
    float target_frames_per_second;
    float delta_time;
    
    int screen_width;
    int screen_height;
    
    char *backend_platform_name;
    char *backend_renderer_name;
    void *backend_platform_data;
    void *backend_renderer_data;
    
} AppIO;

typedef struct App
{
    bool is_running;
    bool fullscreen;
    
    AppIO io;
} App;



#endif //LINUX_PLATFORM_H
