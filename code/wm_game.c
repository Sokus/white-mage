typedef struct Camera
{
    vec3 pos;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;
    
    float yaw;
    float pitch;
    
    float sensitivity;
    float zoom;
} Camera;

void UpdateCameraVectors(Camera *camera)
{
    float yaw_rad = ToRadians(camera->yaw);
    float pitch_rad = ToRadians(camera->pitch);
    
    vec3 front;
    front.x = CosF(yaw_rad) * CosF(pitch_rad);
    front.y = SinF(pitch_rad);
    front.z = SinF(yaw_rad) * CosF(pitch_rad);
    camera->front = NormalizeVec3(front);
    
    camera->right = NormalizeVec3(Cross(camera->front, camera->world_up));
    camera->up = NormalizeVec3(Cross(camera->right, camera->front));
}

void InitializeCamera(Camera *camera,
                      float pos_x, float pos_y, float pos_z,
                      float yaw, float pitch,
                      float sensitivity, float zoom)
{
    camera->pos = Vec3(pos_x, pos_y, pos_z);
    camera->world_up = Vec3(0.0f, 1.0f, 0.0f);
    
    camera->yaw = yaw;
    camera->pitch = pitch;
    
    camera->sensitivity = sensitivity;
    camera->zoom = zoom;
    
    UpdateCameraVectors(camera);
}

mat4 GetCameraViewMatrix(Camera *camera)
{
    vec3 center = AddVec3(camera->pos, camera->front);
    mat4 result = LookAt(camera->pos, center, camera->up);
    return result;
}

void MoveCameraRelative(Camera *camera, float front_movement, float side_movement, float dt)
{
    vec3 front_movement_vec = MultiplyVec3f(camera->front, front_movement * dt);
    vec3 side_movement_vec = MultiplyVec3f(camera->right, side_movement * dt);
    vec3 combined_movement_vec = AddVec3(front_movement_vec, side_movement_vec);
    camera->pos = AddVec3(camera->pos, combined_movement_vec);
}

void ProcessMouse(Camera *camera, float relative_x, float relative_y)
{
    float scaled_relative_x = relative_x * camera->sensitivity;
    float scaled_relative_y = -relative_y * camera->sensitivity;
    
    camera->yaw += scaled_relative_x;
    camera->pitch = CLAMP(-89.0f, camera->pitch + scaled_relative_y, 89.0f);
    
    UpdateCameraVectors(camera);
}

void InitializeInput(Input *input)
{
    MEMORY_SET(&input->keys_down_duration, -1.0f, sizeof(input->keys_down_duration));
}

void UpdateInput(Input *input, float delta_time)
{
    input->mouse_x_prev = input->mouse_x;
    input->mouse_y_prev = input->mouse_y;
    
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
