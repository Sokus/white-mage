/* date = December 25th 2021 6:11 pm */

#ifndef WM_MATH_H
#define WM_MATH_H

#include "math.h"    // sinf, cosf ...
#include "stdbool.h" // comparisons

// macros for easy CRT substitution
#ifndef SINF
#define SINF sinf
#endif

#ifndef COSF
#define COSF cosf
#endif

#ifndef TANF
#define TANF tanf
#endif

#ifndef SQRTF
#define SQRTF sqrtf
#endif

#ifndef EXPF
#define EXPF expf
#endif

#ifndef LOGF
#define LOGF logf
#endif

#ifndef ACOSF
#define ACOSF acosf
#endif

#ifndef ATANF
#define ATANF atanf
#endif

#ifndef ATAN2F
#define ATAN2F atan2f
#endif

// constants
#define PI32 3.14159265359f

// simple helpers

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

#define MOD(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define SQUARE(x) ((x) * (x))

// types
typedef union vec2
{
    float elements[2];
    struct { float x, y; };
    struct { float u, v; };
    struct { float width, height; };
} vec2;

typedef union vec3
{
    float elements[3];
    struct { float x, y, z; };
    struct { float u, v, w; };
    struct { float r, g, b; };
    struct { vec2 xy; float ignored0; };
    struct { float ingored1; vec2 yz; };
} vec3;

typedef union vec4
{
    float elements[4];
    
    struct
    {
        union
        {
            vec3 xyz;
            struct { float x, y, z; };
        };
        float w;
    };
    
    struct
    {
        union
        {
            vec3 rgb;
            struct { float r, g, b; };
        };
        float a;
    };
} vec4;

typedef union mat4
{
    float elements[4][4];
    vec4 columns[4];
} mat4;

// simple functions

float SinF(float radians)
{
    float result = SINF(radians);
    return result;
}

float CosF(float radians)
{
    float result = COSF(radians);
    return result;
}

float TanF(float radians)
{
    float result = TANF(radians);
    return result;
}

float ACosF(float radians)
{
    float result = ACOSF(radians);
    return result;
}

float ATanF(float radians)
{
    float result = ATANF(radians);
    return result;
}

float ATan2F(float left, float right)
{
    float result = ATAN2F(left, right);
    return result;
}

float ExpF(float x)
{
    float result = EXPF(x);
    return result;
}

float LogF(float x)
{
    float result = LOGF(x);
    return result;
}

float SquareRootF(float x)
{
    float result = SQRTF(x);
    return result;
}

float RSquareRootF(float x)
{
    float result = 1.0f / SquareRootF(x);
    return result;
}

float Power(float base, int exponent)
{
    float result = 1.0f;
    float mul = exponent < 0 ? 1.0f / base : base;
    int x = exponent < 0 ? -exponent : exponent;
    while(x)
    {
        if(x & 1)
            result *= mul;
        
        mul *= mul;
        x >>= 1;
    }
    return result;
}

float PowerF(float base, float exponent)
{
    float result = EXPF(exponent * LOGF(base));
    return result;
}

float ToRadians(float degrees)
{
    float result = degrees * (PI32 / 180.0f);
    return result;
}

float Lerp(float a, float x, float b)
{
    float result = (1.0f - x) * a + x * b;
    return result;
}

// vector initialization

vec2 Vec2(float x, float y)
{
    vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

vec2 Vec2i(int x, int y)
{
    vec2 result;
    result.x = (float)x;
    result.y = (float)y;
    return result;
}

vec3 Vec3(float x, float y, float z)
{
    vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

vec3 Vec3i(int x, int y, int z)
{
    vec3 result;
    result.x = (float)x;
    result.y = (float)y;
    result.z = (float)z;
    return result;
}

vec3 Vec3v(vec2 xy, float z)
{
    vec3 result;
    result.x = xy.x;
    result.y = xy.y;
    result.z = z;
    return result;
}

vec4 Vec4(float x, float y, float z, float w)
{
    vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

vec4 Vec4i(int x, int y, int z, int w)
{
    vec4 result;
    result.x = (float)x;
    result.y = (float)y;
    result.z = (float)z;
    result.w = (float)w;
    return result;
}

vec4 Vec4v(vec3 xyz, float w)
{
    vec4 result;
    result.xyz = xyz;
    result.w = w;
    return result;
}

// binary vector operations

vec2 AddVec2(vec2 a, vec2 b)
{
    vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

vec3 AddVec3(vec3 a, vec3 b)
{
    vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

vec4 AddVec4(vec4 a, vec4 b)
{
    vec4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

vec2 SubtractVec2(vec2 a, vec2 b)
{
    vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

vec3 SubtractVec3(vec3 a, vec3 b)
{
    vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

vec4 SubtractVec4(vec4 a, vec4 b)
{
    vec4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

vec2 MultiplyVec2(vec2 a, vec2 b)
{
    vec2 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}

vec2 MultiplyVec2f(vec2 a, float b)
{
    vec2 result;
    result.x = a.x * b;
    result.y = a.y * b;
    return result;
}

vec3 MultiplyVec3(vec3 a, vec3 b)
{
    vec3 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}

vec3 MultiplyVec3f(vec3 a, float b)
{
    vec3 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    return result;
}

vec4 MultiplyVec4(vec4 a, vec4 b)
{
    vec4 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;
    return result;
}

vec4 MultiplyVec4f(vec4 a, float b)
{
    vec4 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    result.w = a.w * b;
    return result;
}

vec2 DivideVec2(vec2 a, vec2 b)
{
    vec2 result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    return result;
}

vec2 DivideVec2f(vec2 a, float b)
{
    vec2 result;
    result.x = a.x / b;
    result.y = a.y / b;
    return result;
}

vec3 DivideVec3(vec3 a, vec3 b)
{
    vec3 result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    return result;
}

vec3 DivideVec3f(vec3 a, float b)
{
    vec3 result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    return result;
}

vec4 DivideVec4(vec4 a, vec4 b)
{
    vec4 result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    result.w = a.w / b.w;
    return result;
}

vec4 DivideVec4f(vec4 a, float b)
{
    vec4 result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.w = a.w / b;
    return result;
}

bool EqualsVec2(vec2 a, vec2 b)
{
    bool result = (a.x == b.x && a.y == b.y);
    return result;
}

bool EqualsVec3(vec3 a, vec3 b)
{
    bool result = (a.x == b.x && a.y == b.y && a.z == b.z);
    return result;
}

bool EqualsVec4(vec4 a, vec4 b)
{
    bool result = (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
    return result;
}

float DotVec2(vec2 a, vec2 b)
{
    float result = (a.x * b.x) + (a.y * b.y);
    return result;
}

float DotVec3(vec3 a, vec3 b)
{
    float result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return result;
}

float DotVec4(vec4 a, vec4 b)
{
    float result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
    return result;
}

vec3 Cross(vec3 a, vec3 b)
{
    vec3 result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);
    return result;
}

// unary vector operations

float LengthSquaredVec2(vec2 a)
{
    float result = DotVec2(a, a);
    return result;
}

float LengthSquaredVec3(vec3 a)
{
    float result = DotVec3(a, a);
    return result;
}

float LengthSquaredVec4(vec4 a)
{
    float result = DotVec4(a, a);
    return result;
}

float LengthVec2(vec2 a)
{
    float result = SquareRootF(LengthSquaredVec2(a));
    return result;
}

float LengthVec3(vec3 a)
{
    float result = SquareRootF(LengthSquaredVec3(a));
    return result;
}

float LengthVec4(vec4 a)
{
    float result = SquareRootF(LengthSquaredVec4(a));
    return result;
}

vec2 NormalizeVec2(vec2 a)
{
    vec2 result = {0};
    float length = LengthVec2(a);
    if(length != 0.0f)
        result = DivideVec2f(a, length);
    return result;
}

vec3 NormalizeVec3(vec3 a)
{
    vec3 result = {0};
    float length = LengthVec3(a);
    if(length != 0.0f)
        result = DivideVec3f(a, length);
    return result;
}

vec4 NormalizeVec4(vec4 a)
{
    vec4 result = {0};
    float length = LengthVec4(a);
    if(length != 0.0f)
        result = DivideVec4f(a, length);
    return result;
}

// matrix functions
mat4 Mat4(void)
{
    mat4 result = {0};
    return result;
}

mat4 Mat4d(float diagonal)
{
    mat4 result = Mat4();
    result.elements[0][0] = diagonal;
    result.elements[1][1] = diagonal;
    result.elements[2][2] = diagonal;
    result.elements[3][3] = diagonal;
    return result;
}

mat4 Transpose(mat4 matrix)
{
    mat4 result;
    for(int col_idx = 0; col_idx < 4; ++col_idx)
    {
        for(int row_idx = 0; row_idx < 4; ++row_idx)
        {
            result.elements[row_idx][col_idx] = matrix.elements[col_idx][row_idx];
        }
    }
    return result;
}

mat4 AddMat4(mat4 a, mat4 b)
{
    mat4 result;
    for(int col_idx = 0; col_idx < 4; ++col_idx)
    {
        for(int row_idx = 0; row_idx < 4; ++row_idx)
        {
            result.elements[col_idx][row_idx] =
            (a.elements[col_idx][row_idx] + b.elements[col_idx][row_idx]);
        }
    }
    return result;
}

mat4 SubtractMat4(mat4 a, mat4 b)
{
    mat4 result;
    for(int col_idx = 0; col_idx < 4; ++col_idx)
    {
        for(int row_idx = 0; row_idx < 4; ++row_idx)
        {
            result.elements[col_idx][row_idx] =
            (a.elements[col_idx][row_idx] - b.elements[col_idx][row_idx]);
        }
    }
    return result;
}

mat4 MultiplyMat4(mat4 a, mat4 b)
{
    mat4 result;
    for(int col_idx = 0; col_idx < 4; ++col_idx)
    {
        for(int row_idx = 0; row_idx < 4; ++row_idx)
        {
            float sum = 0;
            for(int mat_idx = 0; mat_idx < 4; ++mat_idx)
            {
                sum += a.elements[mat_idx][row_idx] * b.elements[col_idx][mat_idx];
            }
            result.elements[col_idx][row_idx] = sum;
        }
    }
    return result;
}

mat4 MultiplyMat4f(mat4 a, float b)
{
    mat4 result;
    for(int col_idx = 0; col_idx < 4; ++col_idx)
    {
        for(int row_idx = 0; row_idx < 4; ++row_idx)
        {
            result.elements[col_idx][row_idx] = a.elements[col_idx][row_idx] * b;
        }
    }
    return result;
}

vec4 MultiplyMat4ByVec4(mat4 matrix, vec4 vector)
{
    vec4 result;
    for(int row_idx = 0; row_idx < 4; ++row_idx)
    {
        float sum = 0.0f;
        for(int col_idx = 0; col_idx < 4; ++col_idx)
        {
            sum += matrix.elements[col_idx][row_idx] * vector.elements[col_idx];
        }
        result.elements[row_idx] = sum;
    }
    return result;
}

mat4 DivideMat4f(mat4 a, float b)
{
    mat4 result;
    for(int col_idx = 0; col_idx < 4; ++col_idx)
    {
        for(int row_idx = 0; row_idx < 4; ++row_idx)
        {
            result.elements[col_idx][row_idx] = a.elements[col_idx][row_idx] / b;
        }
    }
    return result;
}

// common graphics transformations

mat4 Orthographic(float left, float right, float bottom, float top, float near, float far)
{
    mat4 result = Mat4();
    result.elements[0][0] = 2.0f / (right - left);
    result.elements[1][1] = 2.0f / (top - bottom);
    result.elements[2][2] = 2.0f / (near - far);
    result.elements[3][3] = 1.0f;
    
    result.elements[3][0] = (left + right) / (left - right);
    result.elements[3][1] = (bottom + top) / (bottom - top);
    result.elements[3][2] = (far + near) / (near - far);
    
    return result;
}

mat4 Perspective(float fov, float aspect_ratio, float near, float far)
{
    mat4 result = Mat4();
    
    float cotangent = 1.0f / TanF(fov * (PI32 / 360.0f));
    result.elements[0][0] = cotangent / aspect_ratio;
    result.elements[1][1] = cotangent;
    result.elements[2][3] = -1.0f;
    result.elements[2][2] = (near + far) / (near - far);
    result.elements[3][2] = (2.0f * near * far) / (near - far);
    result.elements[3][3] = 0.0f;
    
    return result;
}

mat4 Translate(mat4 matrix, float x, float y, float z)
{
    mat4 translate = Mat4d(1.0f);
    translate.elements[3][0] = x;
    translate.elements[3][1] = y;
    translate.elements[3][2] = z;
    
    mat4 result = MultiplyMat4(translate, matrix);
    return result;
}

mat4 TranslateVec3(mat4 matrix, vec3 translation)
{
    mat4 result = Translate(matrix, translation.x, translation.y, translation.z);
    return result;
}

mat4 Rotate(mat4 matrix, float angle, float axis_x, float axis_y, float axis_z)
{
    mat4 rotate = Mat4d(1.0f);
    
    float mag_sq = SQUARE(axis_x) + SQUARE(axis_y) + SQUARE(axis_z);
    float mag = SQRTF(mag_sq);
    float axis_x_n = (mag > 0.0f) ? axis_x / SQRTF(mag) : axis_x;
    float axis_y_n = (mag > 0.0f) ? axis_y / SQRTF(mag) : axis_y;
    float axis_z_n = (mag > 0.0f) ? axis_z / SQRTF(mag) : axis_z;
    
    float sin_theta = SinF(ToRadians(angle));
    float cos_theta = CosF(ToRadians(angle));
    float cos_value = 1.0f - cos_theta;
    
    rotate.elements[0][0] = (axis_x_n * axis_x_n * cos_value) + cos_theta;
    rotate.elements[0][1] = (axis_x_n * axis_y_n * cos_value) + (axis_z_n * sin_theta);
    rotate.elements[0][2] = (axis_x_n * axis_z_n * cos_value) - (axis_y_n * sin_theta);
    
    rotate.elements[1][0] = (axis_y_n * axis_x_n * cos_value) - (axis_z_n * sin_theta);
    rotate.elements[1][1] = (axis_y_n * axis_y_n * cos_value) + cos_theta;
    rotate.elements[1][2] = (axis_y_n * axis_z_n * cos_value) + (axis_x_n * sin_theta);
    
    rotate.elements[2][0] = (axis_z_n * axis_x_n * cos_value) + (axis_y_n * sin_theta);
    rotate.elements[2][1] = (axis_z_n * axis_y_n * cos_value) - (axis_x_n * sin_theta);
    rotate.elements[2][2] = (axis_z_n * axis_z_n * cos_value) + cos_theta;
    
    mat4 result = MultiplyMat4(rotate, matrix);
    return result;
}

mat4 RotateVec3(mat4 matrix, float angle, vec3 axis)
{
    mat4 result = Rotate(matrix, angle, axis.x, axis.y, axis.z);
    return result;
}

mat4 Scale(mat4 matrix, float x, float y, float z)
{
    mat4 scale = Mat4d(1.0f);
    scale.elements[0][0] = x;
    scale.elements[1][1] = y;
    scale.elements[2][2] = z;
    
    mat4 result = MultiplyMat4(scale, matrix);
    return result;
}

mat4 ScaleVec3(mat4 matrix, vec3 scale)
{
    mat4 result = Scale(matrix, scale.x, scale.y, scale.z);
    return result;
}

mat4 LookAt(vec3 eye, vec3 center, vec3 up)
{
    mat4 result;
    vec3 f = NormalizeVec3(SubtractVec3(center, eye));
    vec3 s = NormalizeVec3(Cross(f, up));
    vec3 u = Cross(s, f);
    
    result.elements[0][0] = s.x;
    result.elements[0][1] = u.x;
    result.elements[0][2] = -f.x;
    result.elements[0][3] = 0.0f;
    
    result.elements[1][0] = s.y;
    result.elements[1][1] = u.y;
    result.elements[1][2] = -f.y;
    result.elements[1][3] = 0.0f;
    
    result.elements[2][0] = s.z;
    result.elements[2][1] = u.z;
    result.elements[2][2] = -f.z;
    result.elements[2][3] = 0.0f;
    
    result.elements[3][0] = -DotVec3(s, eye);
    result.elements[3][1] = -DotVec3(u, eye);
    result.elements[3][2] = DotVec3(f, eye);
    result.elements[3][3] = 1.0f;
    
    return result;
}

#endif //WM_MATH_H
