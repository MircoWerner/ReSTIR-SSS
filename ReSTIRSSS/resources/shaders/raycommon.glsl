#ifndef RAYCOMMON_GLSL
#define RAYCOMMON_GLSL

struct HitPayload {
    int objectDescriptorId; // gl_InstanceCustomIndexEXT
    int triangleId; // gl_PrimitiveID
    float t; // distance
    vec2 barycentrics; // encodes vec3(1.0 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y)
    uint face; // gl_HitKindFrontFacingTriangleEXT or gl_HitKindBackFacingTriangleEXT
};

#endif