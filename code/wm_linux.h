/* date = December 30th 2021 4:52 pm */

#ifndef WM_LINUX_H
#define WM_LINUX_H

typedef struct IO
{
    float target_frames_per_second;
    float delta_time;
    
    int screen_width;
    int screen_height;
} IO;

typedef struct App
{
    bool is_running;
    bool fullscreen;
    
    IO io;
} App;

typedef struct ReadFileResult
{
    void *data;
    uint32_t size;
} ReadFileResult;

typedef struct Image
{
    uint8_t *data;
    int width;
    int height;
    int channels;
} Image;



#endif //WM_LINUX_H
