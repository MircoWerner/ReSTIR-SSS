#ifndef VISIBILITY_GLSL
#define VISIBILITY_GLSL

bool visibility_shadowRay(in const vec3 positionWorld, in const vec3 geometricNormalWorld, in const vec3 lightDirection, in const float tMax, in const uint objectDescriptor, in const uint triangleId) {
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery,
    topLevelAS,
    //    gl_RayFlagsOpaqueEXT,
    gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
    0xFF,
    positionWorld,
    0.001,
    lightDirection,
    tMax - 0.001);

    //    while(rayQueryProceedEXT(rayQuery)){
    //    }
    //
    //    bool hit = (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT);
    //    if (!hit || rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, true) == objectDescriptor && rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true) == triangleId) {
    //        return 1.0;
    //    }
    //    return 0.0;
    rayQueryProceedEXT(rayQuery);
    return rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionNoneEXT;
}

#endif