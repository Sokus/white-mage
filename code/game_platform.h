/* date = December 22nd 2021 2:33 pm */

#ifndef GAME_PLATFORM_H
#define GAME_PLATFORM_H

typedef enum InputKey
{
    InputKey_MoveUp,      // W
    InputKey_MoveLeft,    // A
    InputKey_MoveDown,    // S
    InputKey_MoveRight,   // D
    InputKey_ActionUp,    // R
    InputKey_ActionLeft,  // F
    InputKey_ActionDown,  // E
    InputKey_ActionRight, // Q
    InputKey_Select,      // Tab
    InputKey_Start,       // Esc
    
    InputKey_Count,
} InputKey;

char *InputKeyName(InputKey input_key)
{
    switch(input_key)
    {
        case InputKey_MoveUp:      return "MoveUp";
        case InputKey_MoveLeft:    return "MoveLeft";
        case InputKey_MoveDown:    return "MoveDown";
        case InputKey_MoveRight:   return "MoveRight";   
        case InputKey_ActionUp:    return "ActionUp";
        case InputKey_ActionLeft:  return "ActionLeft";
        case InputKey_ActionDown:  return "ActionDown";
        case InputKey_ActionRight: return "ActionRight";
        case InputKey_Select:      return "Select";
        case InputKey_Start:       return "Start";
        default:                    return "Invalid";
    }
}

typedef struct Input
{
    bool keys_down[InputKey_Count];
} Input;

#endif //GAME_PLATFORM_H
