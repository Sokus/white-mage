typedef enum RenderTaskType
{
    RenderTask_PushMatrix,
    RenderTask_PopMatrix,
    RenderTask_Primitive,
    RenderTask_Count,
} RenderTaskType;

typedef enum RenderTaskMatrixType
{
    RenderTaskMatrix_Raw,
    RenderTaskMatrix_Translate,
    RenderTaskMatrix_Rotate,
    RenderTaskMatrix_Scale,
    RenderTaskMatrix_Orthographic,
    RenderTaskMatrix_Perspective,
    RenderTaskMatrix_Count
} RenderTaskMatrixType;

typedef struct RenderTaskEmpty
{
    RenderTaskType type;
} RenderTaskEmpty;

typedef struct RenderTaskMatrixRaw
{
    RenderTaskType type;
    bool multiply;
    mat4 matrix;
} RenderTaskMatrixRaw;

typedef struct RenderTaskMatrixTransform
{
    RenderTaskType type;
    bool multiply;
    union
    {
        vec3 translate;
        vec3 rotate;
        vec3 scale;
    }
} RenderTaskMatrixTransform;

typedef struct RenderTaskMatrixOrthographic
{
    RenderTaskType type;
    bool multiply;
    float left, right, bottom, top, near, far;
} RenderTaskMatrixOrthographic;

typedef struct RenderTaskMatrixProjection
{
    RenderTaskType type;
    bool multiply;
    float fov, aspect_ratio, near, far;
} RenderTaskMatrixProjection;
