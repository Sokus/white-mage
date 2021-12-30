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
