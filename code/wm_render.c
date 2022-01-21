
typedef struct RenderTaskList
{
    MemoryArena *vertex_memory_arena_ptr;
    MemoryArena list_memory_arena;
} RenderTaskList;

typedef struct VertexColored
{
    vec3 pos;
    vec4 color;
} VertexColored;

typedef enum RenderTaskType
{
    RenderTask_Primitive,
    
    RenderTask_PushMatrix,
    RenderTask_PopMatrix,
    RenderTask_Count,
} RenderTaskType;

typedef enum RenderTaskPrimitiveType
{
    RenderTaskPrimitive_Triangles,
    
    RenderTaskPrimitive_Count
} RenderTaskPrimitiveType;

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

typedef struct RenderTaskHeader
{
    RenderTaskType type;
    
    union
    {
        RenderTaskPrimitiveType primitive_type;
        RenderTaskMatrixType    matrix_type;
    };
} RenderTaskHeader;

typedef struct RenderTaskPrimitive
{
    RenderTaskHeader header;
    size_t vertex_offset;
    size_t vertex_count;
} RenderTaskPrimitive;

typedef struct RenderTaskMatrixRaw
{
    RenderTaskHeader header;
    bool multiply;
    mat4 matrix;
} RenderTaskMatrixRaw;

typedef struct RenderTaskMatrixTransform
{
    RenderTaskHeader header;
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
    RenderTaskHeader header;
    bool multiply;
    float left, right, bottom, top, near, far;
} RenderTaskMatrixOrthographic;

typedef struct RenderTaskMatrixProjection
{
    RenderTaskHeader header;
    bool multiply;
    float fov, aspect_ratio, near, far;
} RenderTaskMatrixProjection;

void RenderTriangle(RenderTaskList *render_task_list,
                    float x0, float y0, float z0,
                    float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float r, float g, float b, float a)
{
    size_t vtx_count = 3;
    size_t vtx_size  = sizeof(VertexColored);
    
    MemoryArena *vertex_memory_arena = render_task_list->vertex_memory_arena_ptr;
    MemoryArena *list_memory_arena = &render_task_list->list_memory_arena;
    
    bool can_fit_task = MemoryArenaCanFit(list_memory_arena, sizeof(RenderTaskPrimitive));
    bool can_fit_vertices = MemoryArenaCanFit(vertex_memory_arena, vtx_count * vtx_size);
    
    if(can_fit_task && can_fit_vertices)
    {
        RenderTaskPrimitive *task = PUSH_STRUCT(list_memory_arena, RenderTaskPrimitive);
        task->header.type = RenderTask_Primitive;
        task->header.primitive_type = RenderTaskPrimitive_Triangles;
        task->vertex_offset = vertex_memory_arena->used;
        task->vertex_count = vtx_count;
        
        VertexColored *vtxs = PUSH_ARRAY(vertex_memory_arena, Vertex, vtx_count);
        vtxs[0].pos = Vec3(x0, y0, z0);
        vtxs[1].pos = Vec3(x1, y1, z1);
        vtxs[2].pos = Vec3(x2, y2, z2);
        
        Vec4 color = Vec4(r, g, b, a);
        vtxs[0].color = color;
        vtxs[1].color = color;
        vtxs[2].color = color;
    }
}

void RenderRectangle2D(RenderTaskList *render_task_list,
                       float x0, float y0,
                       float x1, float y1,
                       float r, float g, float b, float a)
{
    size_t vtx_count = 6;
    size_t vtx_size  = sizeof(VertexColored);
    
    MemoryArena *vertex_memory_arena = render_task_list->vertex_memory_arena_ptr;
    MemoryArena *list_memory_arena = &render_task_list->list_memory_arena;
    
    bool can_fit_task = MemoryArenaCanFit(list_memory_arena, sizeof(RenderTaskPrimitive));
    bool can_fit_vertices = MemoryArenaCanFit(vertex_memory_arena, vtx_count * vtx_size);
    
    if(can_fit_task && can_fit_vertices)
    {
        RenderTaskPrimitive *task = PUSH_STRUCT(list_memory_arena, RenderTaskPrimitive);
        task->header.type = RenderTask_Primitive;
        task->header.primitive_type = RenderTaskPrimitive_Triangles;
        task->vertex_offset = vertex_memory_arena->used;
        task->vertex_count = vtx_count;
        
        VertexColored *vtxs = PUSH_ARRAY(vertex_memory_arena, Vertex, vtx_count);
        // first triangle
        vtxs[0].pos = Vec3(x0, y0, 0);
        vtxs[1].pos = Vec3(x1, y0, 0);
        vtxs[2].pos = Vec3(x2, y2, 0);
        
        // second triangle
        vtxs[3].pos = Vec3(x0, y0, 0);
        vtxs[4].pos = Vec3(x1, y1, 0);
        vtxs[5].pos = Vec3(x0, y1, 0);
        
        Vec4 color = Vec4(r, g, b, a);
        vtxs[0].color = color;
        vtxs[1].color = color;
        vtxs[2].color = color;
        vtxs[3].color = color;
        vtxs[4].color = color;
        vtxs[5].color = color;
    }
}