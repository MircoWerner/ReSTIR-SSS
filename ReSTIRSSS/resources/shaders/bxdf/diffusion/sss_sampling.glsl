#ifndef SSS_SAMPLING_GLSL
#define SSS_SAMPLING_GLSL

struct SSSInfo {
    vec3 position;

    int objectDescriptorId;

    vec3 scatterDistance;

    uint intersection;
};

struct SSSSample {
    int objectDescriptorId;
    int triangleId;
    vec2 barycentrics;

    vec3 position;
    vec3 geometricNormal;
    vec3 normal;

    uint intersection;
};

bool sss_sampling(inout uint rngState, in const SSSInfo sssInfo, out SSSSample sssSample, out float pdf);

#endif