#ifndef TRACE_RAYQUERY_GLSL
#define TRACE_RAYQUERY_GLSL

#include "../raycommon.glsl"
#include "trace.glsl"

bool trace(in const vec3 origin, in const vec3 direction, in const float tMin, in const float tMax, out HitPayload hitPayload) {
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery,
    topLevelAS,
    gl_RayFlagsOpaqueEXT,
    0xFF,
    origin,
    tMin + 0.001,
    direction,
    tMax);

    while(rayQueryProceedEXT(rayQuery)){
    }

    if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionNoneEXT) {
        return false;
    }

    hitPayload.objectDescriptorId = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, true);
    hitPayload.triangleId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true);
    hitPayload.t = rayQueryGetIntersectionTEXT(rayQuery, true);
    hitPayload.barycentrics = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
    hitPayload.face = rayQueryGetIntersectionFrontFaceEXT(rayQuery, true) ? gl_HitKindFrontFacingTriangleEXT : gl_HitKindBackFacingTriangleEXT;
    return true;
}

#endif