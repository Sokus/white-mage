/* date = December 22nd 2021 2:33 pm */

#ifndef WHITE_MAGE_PLATFORM_H
#define WHITE_MAGE_PLATFORM_H

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
    float keys_down_duration[InputKey_Count];
    float keys_down_duration_previous[InputKey_Count];
} Input;

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

typedef struct TextureAtlas
{
    unsigned int texture_id;
    int width;
    int height;
    int channels;
    int tile_w;
    int tile_h;
    int tiles_x;
    int tiles_y;
    int tile_count;
} TextureAtlas;

#endif //WHITE_MAGE_PLATFORM_H
