#ifndef GBUFFER_GLSL
#define GBUFFER_GLSL

#include "../../defines.glsl"
#include "../../utility/normal_mapping.glsl"
#include "../../material.glsl"
#include "../../trace/trace.glsl"

bool pixelInfoFromGBuffer(in const vec3 origin, in const GBuffer gBuffer, out ReSTIRPixelInfo pixelInfo) {
    // handle error or sky hit
    if (gBuffer.objectDescriptorId == GBUFFER_NULL || gBuffer.triangleId == GBUFFER_NULL) {
        return false;
    }

    // intersection info
    const HitPayload payload = HitPayload(int(gBuffer.objectDescriptorId), int(gBuffer.triangleId), 0, vec2(gBuffer.barycentricsY, gBuffer.barycentricsZ), gl_HitKindFrontFacingTriangleEXT);
    intersectionInfo(payload, origin, pixelInfo.position, pixelInfo.geometricNormal, pixelInfo.normal, pixelInfo.material);
    pixelInfo.v = normalize(origin - pixelInfo.position);

    return true;
}

#endif