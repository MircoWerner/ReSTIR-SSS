#ifndef RESTIR_SPATIAL_KERNEL_GLSL
#define RESTIR_SPATIAL_KERNEL_GLSL

#include "../../utility/random.glsl"
#include "../../utility/constants.glsl"
#include "../../bxdf/diffusion/sss_diffusion_profile.glsl"

ivec2 restirsss_spatial_kernel_uniform(inout uint rngState, in const float maxRadius) {
    const float radius = maxRadius * sqrt(nextFloat(rngState));
    const float angle = 2.0 * PI * nextFloat(rngState);

    const vec2 offset = vec2(radius * cos(angle), radius * sin(angle));

    return ivec2(round(offset));
}

float restirsss_spatial_kernel_maxSearchRadiusDiffusion(in const vec3 scatterDistance, in const vec3 position, in const ivec2 pixel, in const vec2 size) {
    const float maxRadius = 2.0 * max(scatterDistance.r, max(scatterDistance.g, scatterDistance.b));

    const mat4 g_view_to_world_space = inverse(g_world_to_view_space);
    // transform screen space axes into world space
    const vec3 worldSpaceE1 = (g_view_to_world_space * vec4(1.0, 0.0, 0.0, 0.0)).xyz;
    const vec3 worldSpaceE2 = (g_view_to_world_space * vec4(0.0, 1.0, 0.0, 0.0)).xyz;

    // maximum offset positions in world space
    const vec3 worldSpaceOffsetPositionRight = position + maxRadius * worldSpaceE1;
    const vec3 worldSpaceOffsetPositionUp = position + maxRadius * worldSpaceE2;
    // transform back to screen space
    const vec3 screenSpaceOffsetPositionRight = (g_world_to_view_space * vec4(worldSpaceOffsetPositionRight, 1.0)).xyz;
    const vec3 screenSpaceOffsetPositionUp = (g_world_to_view_space * vec4(worldSpaceOffsetPositionUp, 1.0)).xyz;
    // project to clip space
    const vec3 clipSpaceOffsetPositionRight = (g_view_to_clip_space * vec4(screenSpaceOffsetPositionRight, 1.0)).xyz; // [-1,1]
    const vec3 clipSpaceOffsetPositionUp = (g_view_to_clip_space * vec4(screenSpaceOffsetPositionUp, 1.0)).xyz; // [-1,1]
    const vec2 clipSpaceProjectedOffsetPositionRight = clipSpaceOffsetPositionRight.xy / clipSpaceOffsetPositionRight.z;
    const vec2 clipSpaceProjectedOffsetPositionUp = clipSpaceOffsetPositionUp.xy / clipSpaceOffsetPositionUp.z;
    // convert to NDC
    const vec2 ndcOffsetPositionRight = clipSpaceProjectedOffsetPositionRight * 0.5 + 0.5; // [0,1]
    const vec2 ndcOffsetPositionUp = clipSpaceProjectedOffsetPositionUp * 0.5 + 0.5; // [0,1]

    const vec2 pixelRight = round(ndcOffsetPositionRight * vec2(size));
    const vec2 pixelUp = round(ndcOffsetPositionUp * vec2(size));

    const float searchRadiusRight = distance(vec2(pixel), pixelRight);
    const float searchRadiusUp = distance(vec2(pixel), pixelUp);

    return clamp(max(searchRadiusRight, searchRadiusUp), 5, 30);
}

#endif