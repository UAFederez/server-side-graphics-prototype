#ifndef SSR_VECTOR3_H
#define SSR_VECTOR3_H

#include <math.h>

typedef struct {
    float x;
    float y;
    float z;
} Vector3f;

float length_squared_v3f(const Vector3f* v)
{
    return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

float length_v3f(const Vector3f* v)
{
    return sqrt(length_squared_v3f(v));
}

Vector3f mulf_v3f(const Vector3f v, const float f)
{
    Vector3f result = { v.x * f, v.y * f, v.z * f };
    return result;
}

Vector3f normalize_v3f(const Vector3f* v)
{
    const float length  = length_v3f(v);
    Vector3f normalized = { v->x / length, v->y / length, v->z / length };
    return normalized;
}

Vector3f add_v3f(const Vector3f v1, const Vector3f v2)
{
    Vector3f sum = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    return sum;
}

Vector3f sub_v3f(const Vector3f v1, const Vector3f v2)
{
    Vector3f diff = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    return diff;
}

float dot_v3f(const Vector3f v1, const Vector3f v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3f lerp_v3f(const Vector3f* v1, const Vector3f* v2, const float f)
{
    Vector3f lerp = {
        v1->x * (1 - f) + f * v2-> x,
        v1->y * (1 - f) + f * v2-> y,
        v1->z * (1 - f) + f * v2-> z,
    };
    return lerp;
}


#endif // vector3f.h
