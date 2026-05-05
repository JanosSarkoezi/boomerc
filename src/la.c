#include "la.h"
#include <math.h>

Vec2f vec2(float x, float y) {
    return (Vec2f){x, y};
}

Vec2f vec2_mul_scalar(Vec2f a, float s) {
    return vec2(a.x * s, a.y * s);
}

Vec2f vec2_div_scalar(Vec2f a, float s) {
    return vec2(a.x / s, a.y / s);
}

Vec2f vec2_mul(Vec2f a, Vec2f b) {
    return vec2(a.x * b.x, a.y * b.y);
}

Vec2f vec2_div(Vec2f a, Vec2f b) {
    return vec2(a.x / b.x, a.y / b.y);
}

Vec2f vec2_sub(Vec2f a, Vec2f b) {
    return vec2(a.x - b.x, a.y - b.y);
}

Vec2f vec2_add(Vec2f a, Vec2f b) {
    return vec2(a.x + b.x, a.y + b.y);
}

float vec2_length(Vec2f a) {
    return sqrtf(a.x * a.x + a.y * a.y);
}

Vec2f vec2_normalize(Vec2f a) {
    float len = vec2_length(a);
    if (len == 0.0f) {
        return vec2(0.0f, 0.0f);
    }
    return vec2_div_scalar(a, len);
}
