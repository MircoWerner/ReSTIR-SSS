#ifndef SSS_SAMPLING_DISK_GLSL
#define SSS_SAMPLING_DISK_GLSL

#include "sss_sampling.glsl"

#include "sss_diffusion_profile.glsl"

#include "../../defines.glsl"
#include "../../utility/random.glsl"
#include "../../utility/normal_mapping.glsl"
#include "../../trace/trace.glsl"
#include "../bsdf/bsdf.glsl"

#define SSS_SAMPLING_DISK_AXIS_0_WEIGHT 0.5
#define SSS_SAMPLING_DISK_AXIS_1_WEIGHT 0.25
#define SSS_SAMPLING_DISK_AXIS_2_WEIGHT 0.25
#define SSS_SAMPLING_DISK_CHANNEL_0_WEIGHT 1.0 / 3.0
#define SSS_SAMPLING_DISK_CHANNEL_1_WEIGHT 1.0 / 3.0
#define SSS_SAMPLING_DISK_CHANNEL_2_WEIGHT 1.0 / 3.0

bool sss_sampling_disk_sample(inout uint rngState, in const vec3 sssPosition, in const vec3 origin, in const vec3 direction, in const float tMin, in const float tMax, in const int objectDescriptorId, out SSSSample sssSample, out float pdf) {
    uint chosenIntersection = 0;
    uint numIntersections = 0;

    // weighted reservoir sampling - we want to choose an intersection with the probability 1/numIntersections
    float weightTotal = 0.0;
    float weightNew = 1.0;

    int wrsObjectDescriptorId = 0;
    int wrsTriangleID = 0;
    vec2 wrsBarycentrics = vec2(0);
    float wrsWeight = 0;
    //

    // IMPORTANT: We want to find each intersection only once in the while loop. By default, the specification does not guarantee that rayQueryProceedEXT runs only once per intersection.
    // This can be prevented by adding VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR to the VkAccelerationStructureGeometryKHR object while building the BLAS.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGeometryFlagBitsNV.html
    // https://nvpro-samples.github.io/vk_mini_path_tracer/index.html#fourusesofintersectiondata/countingintersectionswithrayqueryproceedext
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, // Ray query
    topLevelAS, // Top-level acceleration structure
    gl_RayFlagsNoOpaqueEXT, // Ray flags, here saying "treat all geometry as opaque"
    0xFF, // 8-bit instance mask, here saying "trace against all instances"
    origin, // Ray origin
    tMin, // Minimum t-value
    direction, // Ray direction
    tMax);// Maximum t-value

    while (rayQueryProceedEXT(rayQuery)) {
        const int nextObjectDescriptorId = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false);// false = for the candidate intersection, not the commited one
        if (nextObjectDescriptorId != objectDescriptorId) {
            // we hit another object, ignore this intersection
            continue;
        }

        const int nextTriangleID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
        const vec2 nextBarycentrics = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);

        // weighted reservoir sampling
        if (nextFloat(rngState) <= weightNew / (weightNew + weightTotal)) {
            wrsObjectDescriptorId = nextObjectDescriptorId;
            wrsTriangleID = nextTriangleID;
            wrsBarycentrics = nextBarycentrics;
            chosenIntersection = numIntersections;
            wrsWeight = weightNew;
        }
        weightTotal += weightNew;

        numIntersections++;
    }

    if (numIntersections > 0) {
        sssSample.objectDescriptorId = wrsObjectDescriptorId;
        sssSample.triangleId = wrsTriangleID;
        sssSample.barycentrics = wrsBarycentrics;
        HitPayload payload = HitPayload(wrsObjectDescriptorId, wrsTriangleID, 0, wrsBarycentrics, gl_HitKindFrontFacingTriangleEXT);
        intersectionInfo(payload, sssSample.position, sssSample.geometricNormal, sssSample.normal);
        sssSample.intersection = chosenIntersection;

        pdf = wrsWeight / weightTotal;
        return true;
    }
    return false;
}

bool sss_sampling_disk_sample_nthIntersection(inout uint rngState, in const vec3 sssPosition, in const vec3 origin, in const vec3 direction, in const float tMin, in const float tMax, in const int objectDescriptorId, in const uint nthIntersection, out SSSSample sssSample) {
    // IMPORTANT: We want to find each intersection only once in the while loop. By default, the specification does not guarantee that rayQueryProceedEXT runs only once per intersection.
    // This can be prevented by adding VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR to the VkAccelerationStructureGeometryKHR object while building the BLAS.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGeometryFlagBitsNV.html
    // https://nvpro-samples.github.io/vk_mini_path_tracer/index.html#fourusesofintersectiondata/countingintersectionswithrayqueryproceedext
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, // Ray query
    topLevelAS, // Top-level acceleration structure
    gl_RayFlagsNoOpaqueEXT, // Ray flags, here saying "treat all geometry as opaque"
    0xFF, // 8-bit instance mask, here saying "trace against all instances"
    origin, // Ray origin
    tMin, // Minimum t-value
    direction, // Ray direction
    tMax);// Maximum t-value

    uint numIntersections = 0;
    while (rayQueryProceedEXT(rayQuery)) {
        const int nextObjectDescriptorId = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false);// false = for the candidate intersection, not the commited one
        if (nextObjectDescriptorId != objectDescriptorId) {
            // we hit another object, ignore this intersection
            continue;
        }

        numIntersections++;

        if (nthIntersection == numIntersections - 1) {
            sssSample.objectDescriptorId = nextObjectDescriptorId;
            sssSample.triangleId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
            sssSample.barycentrics = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
            break;
        }
    }

    if (numIntersections <= nthIntersection) {
        return false;
    }

    HitPayload payload = HitPayload(sssSample.objectDescriptorId, sssSample.triangleId, 0, sssSample.barycentrics, gl_HitKindFrontFacingTriangleEXT);
    intersectionInfo(payload, sssSample.position, sssSample.geometricNormal, sssSample.normal);
    sssSample.intersection = nthIntersection;

    return true;
}

bool sss_sampling_disk_sample_intersectionId(in const vec3 origin, in const vec3 direction, in const float tMin, in const float tMax, in const int objectDescriptorId, in const int triangleId, in const vec2 barycentrics, out uint intersectionId) {
    // IMPORTANT: We want to find each intersection only once in the while loop. By default, the specification does not guarantee that rayQueryProceedEXT runs only once per intersection.
    // This can be prevented by adding VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR to the VkAccelerationStructureGeometryKHR object while building the BLAS.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGeometryFlagBitsNV.html
    // https://nvpro-samples.github.io/vk_mini_path_tracer/index.html#fourusesofintersectiondata/countingintersectionswithrayqueryproceedext
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, // Ray query
    topLevelAS, // Top-level acceleration structure
    gl_RayFlagsNoOpaqueEXT, // Ray flags, here saying "treat all geometry as opaque"
    0xFF, // 8-bit instance mask, here saying "trace against all instances"
    origin, // Ray origin
    tMin, // Minimum t-value
    direction, // Ray direction
    tMax);// Maximum t-value

    uint numIntersections = 0;
    while (rayQueryProceedEXT(rayQuery)) {
        const int nextObjectDescriptorId = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false);// false = for the candidate intersection, not the commited one
        if (nextObjectDescriptorId != objectDescriptorId) {
            // we hit another object, ignore this intersection
            continue;
        }

        const uint nextTriangleId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);

        if (triangleId == nextTriangleId/* && all(lessThan(abs(barycentrics - nextBarycentrics), vec2(0.01)))*/) {
            intersectionId = numIntersections;
            return true;
        }

        numIntersections++;
    }

    return false;
}

// https://dl.acm.org/doi/10.1145/2504459.2504520
// https://www.researchgate.net/publication/262244208_BSSRDF_importance_sampling
// https://www.pbr-book.org/3ed-2018/Light_Transport_II_Volume_Rendering/Sampling_Subsurface_Reflection_Functions#SeparableBSSRDF::Pdf_Sp
float sss_sampling_disk_pdf(in const vec3 position, in const BSDFFrame frame, in const vec3 samplePosition, in const vec3 sampleNormal, in const vec3 scatterDistance) {
    const vec3 d = samplePosition - position;
    const vec3 dLocal = vec3(dot(frame.n, d), dot(frame.t, d), dot(frame.b, d));

    const vec3 rProj = vec3(
    sqrt(dLocal.y * dLocal.y + dLocal.z * dLocal.z),
    sqrt(dLocal.z * dLocal.z + dLocal.x * dLocal.x),
    sqrt(dLocal.x * dLocal.x + dLocal.y * dLocal.y));

    const vec3 nLocal = abs(vec3(dot(frame.n, sampleNormal), dot(frame.t, sampleNormal), dot(frame.b, sampleNormal)));
    const vec3 axisProb = { SSS_SAMPLING_DISK_AXIS_0_WEIGHT, SSS_SAMPLING_DISK_AXIS_1_WEIGHT, SSS_SAMPLING_DISK_AXIS_2_WEIGHT };
    const vec3 channelProb = { SSS_SAMPLING_DISK_CHANNEL_0_WEIGHT, SSS_SAMPLING_DISK_CHANNEL_1_WEIGHT, SSS_SAMPLING_DISK_CHANNEL_2_WEIGHT };

    float pdf = 0.0;
    // axis = 0, channel = 0,1,2
    //    pdf += sss_diffusion_profile_pdf(rProj[0], scatterDistance[0]) * axisProb[0] * channelProb[0] * nLocal[0];
    //    pdf += sss_diffusion_profile_pdf(rProj[0], scatterDistance[1]) * axisProb[0] * channelProb[0] * nLocal[0];
    //    pdf += sss_diffusion_profile_pdf(rProj[0], scatterDistance[2]) * axisProb[0] * channelProb[0] * nLocal[0];
    vec3 pdfAxis = sss_diffusion_profile_pdf_vectorized(rProj[0], scatterDistance) * axisProb[0] * channelProb[0] * nLocal[0];
    pdf += pdfAxis[0] + pdfAxis[1] + pdfAxis[2];
    // axis = 1, channel = 0,1,2
    //    pdf += sss_diffusion_profile_pdf(rProj[1], scatterDistance[0]) * axisProb[1] * channelProb[1] * nLocal[1];
    //    pdf += sss_diffusion_profile_pdf(rProj[1], scatterDistance[1]) * axisProb[1] * channelProb[1] * nLocal[1];
    //    pdf += sss_diffusion_profile_pdf(rProj[1], scatterDistance[2]) * axisProb[1] * channelProb[1] * nLocal[1];
    pdfAxis = sss_diffusion_profile_pdf_vectorized(rProj[1], scatterDistance) * axisProb[1] * channelProb[1] * nLocal[1];
    pdf += pdfAxis[0] + pdfAxis[1] + pdfAxis[2];
    // axis = 2, channel = 0,1,2
    //    pdf += sss_diffusion_profile_pdf(rProj[2], scatterDistance[0]) * axisProb[2] * channelProb[2] * nLocal[2];
    //    pdf += sss_diffusion_profile_pdf(rProj[2], scatterDistance[1]) * axisProb[2] * channelProb[2] * nLocal[2];
    //    pdf += sss_diffusion_profile_pdf(rProj[2], scatterDistance[2]) * axisProb[2] * channelProb[2] * nLocal[2];
    pdfAxis = sss_diffusion_profile_pdf_vectorized(rProj[2], scatterDistance) * axisProb[2] * channelProb[2] * nLocal[2];
    pdf += pdfAxis[0] + pdfAxis[1] + pdfAxis[2];

    return pdf;
}

void sss_sampling_disk_pdf_axis_channel(in const vec3 position, in const BSDFFrame frame, in const vec3 samplePosition, in const vec3 sampleNormal, in const vec3 scatterDistance, out vec3 pdfAxis0, out vec3 pdfAxis1, out vec3 pdfAxis2) {
    const vec3 d = samplePosition - position;
    const vec3 dLocal = vec3(dot(frame.n, d), dot(frame.t, d), dot(frame.b, d));

    const vec3 rProj = vec3(
    sqrt(dLocal.y * dLocal.y + dLocal.z * dLocal.z),
    sqrt(dLocal.z * dLocal.z + dLocal.x * dLocal.x),
    sqrt(dLocal.x * dLocal.x + dLocal.y * dLocal.y));

    const vec3 nLocal = abs(vec3(dot(frame.n, sampleNormal), dot(frame.t, sampleNormal), dot(frame.b, sampleNormal)));

    //    pdfAxis0[0] = sss_diffusion_profile_pdf(rProj[0], scatterDistance[0]) * nLocal[0];
    //    pdfAxis0[1] = sss_diffusion_profile_pdf(rProj[0], scatterDistance[1]) * nLocal[0];
    //    pdfAxis0[2] = sss_diffusion_profile_pdf(rProj[0], scatterDistance[2]) * nLocal[0];
    pdfAxis0 = sss_diffusion_profile_pdf_vectorized(rProj[0], scatterDistance) * nLocal[0];

    //    pdfAxis1[0] = sss_diffusion_profile_pdf(rProj[1], scatterDistance[0]) * nLocal[1];
    //    pdfAxis1[1] = sss_diffusion_profile_pdf(rProj[1], scatterDistance[1]) * nLocal[1];
    //    pdfAxis1[2] = sss_diffusion_profile_pdf(rProj[1], scatterDistance[2]) * nLocal[1];
    pdfAxis1 = sss_diffusion_profile_pdf_vectorized(rProj[1], scatterDistance) * nLocal[1];

    //    pdfAxis2[0] = sss_diffusion_profile_pdf(rProj[2], scatterDistance[0]) * nLocal[2];
    //    pdfAxis2[1] = sss_diffusion_profile_pdf(rProj[2], scatterDistance[1]) * nLocal[2];
    //    pdfAxis2[2] = sss_diffusion_profile_pdf(rProj[2], scatterDistance[2]) * nLocal[2];
    pdfAxis2 = sss_diffusion_profile_pdf_vectorized(rProj[2], scatterDistance) * nLocal[2];
}

uint sss_sampling_axis_index(in const float xiAxis) {
    if (xiAxis < SSS_SAMPLING_DISK_AXIS_0_WEIGHT) {
        return 0;
    } else if (xiAxis < (SSS_SAMPLING_DISK_AXIS_0_WEIGHT + SSS_SAMPLING_DISK_AXIS_1_WEIGHT)) {
        return 1;
    } else {
        return 2;
    }
}

void sss_sampling_axis(in const uint axis, in const BSDFFrame frame, out BSDFFrame projectionFrame) {
    if (axis == 0) {
        projectionFrame.t = frame.t;
        projectionFrame.b = frame.b;
        projectionFrame.n = frame.n;
    } else if (axis == 1) {
        projectionFrame.t = frame.b;
        projectionFrame.b = frame.n;
        projectionFrame.n = frame.t;
    } else {
        projectionFrame.t = frame.n;
        projectionFrame.b = frame.t;
        projectionFrame.n = frame.b;
    }
}

uint sss_sampling_scatterDistance_index(in const float xiChannel) {
    return clamp(uint(floor(3 * xiChannel)), 0, 2);
}

float sss_sampling_scatterDistance(in const uint channel, in const vec3 scatterDistance) {
    return scatterDistance[channel];
}

bool sss_sampling_sample(in const BSDFFrame frame, in const BSDFFrame projectionFrame, in const SSSInfo sssInfo, in const uint channel, in const float xiRadius, in const float xiAngle, inout uint rngStateIntersection, out SSSSample sssSample, out float pdf, out float intersectionPDF) {
    const float sampledScatterDistance = sss_sampling_scatterDistance(channel, sssInfo.scatterDistance);

    const float radius = sss_diffusion_profile_sample(xiRadius, sampledScatterDistance);
    const float radiusMax = sss_diffusion_profile_sample(0.999, sampledScatterDistance);
    if (radius > radiusMax) {
        return false;
    }

    const float phi = xiAngle * TWO_PI;

    const vec3 origin = sssInfo.position + radiusMax * projectionFrame.n + cos(phi) * radius * projectionFrame.t + sin(phi) * radius * projectionFrame.b;
    const vec3 direction = -projectionFrame.n;
    const float sphereFraction = sqrt(radiusMax * radiusMax - radius * radius);
    const float tMin = radiusMax - sphereFraction;
    const float tMax = radiusMax + sphereFraction;

    if (sssInfo.intersection == INVALID_UINT_VALUE) {
        if (!sss_sampling_disk_sample(rngStateIntersection, sssInfo.position, origin, direction, tMin, tMax, sssInfo.objectDescriptorId, sssSample, intersectionPDF)) {
            return false;
        }
    } else {
        intersectionPDF = 1.0;
        if (!sss_sampling_disk_sample_nthIntersection(rngStateIntersection, sssInfo.position, origin, direction, tMin, tMax, sssInfo.objectDescriptorId, sssInfo.intersection, sssSample)) {
            return false;
        }
    }

    pdf = sss_sampling_disk_pdf(sssInfo.position, frame, sssSample.position, sssSample.geometricNormal, sssInfo.scatterDistance);

    return true;
}

bool sss_sampling_sampleInvert(in const BSDFFrame frame, in const BSDFFrame projectionFrame, in const SSSInfo sssInfo, in const uint channel, out float xiRadius, out float xiAngle, in SSSSample sssSample, out float pdf) {
    pdf = sss_sampling_disk_pdf(sssInfo.position, frame, sssSample.position, sssSample.geometricNormal, sssInfo.scatterDistance);

    // use same channel as base path
    const float sampledScatterDistance = sss_sampling_scatterDistance(channel, sssInfo.scatterDistance);

    const vec3 difference = sssSample.position - sssInfo.position;
    const float distanceInTangetPlaneX = dot(difference, projectionFrame.t);
    const float distanceInTangetPlaneY = dot(difference, projectionFrame.b);

    // determine new radius
    const float radius = sqrt(distanceInTangetPlaneX * distanceInTangetPlaneX + distanceInTangetPlaneY * distanceInTangetPlaneY);
    const float radiusMax = sss_diffusion_profile_sample(0.999, sampledScatterDistance);
    if (radius > radiusMax) {
        return false;
    }
    xiRadius = 1.0 - 0.25 * exp(-radius / sampledScatterDistance) - 0.75 * exp(-radius / (3.0 * sampledScatterDistance));

    // determine new angle
    const float cosPhi = distanceInTangetPlaneX / radius;
    const float sinPhi = distanceInTangetPlaneY / radius;
    float phi = atan(sinPhi, cosPhi);
    phi = phi >= 0 ? phi : phi + 2.0 * PI;
    xiAngle = phi / (2.0 * PI);

    return true;
}

bool sss_sampling_sampleIntersectionId(in const BSDFFrame frame, in const BSDFFrame projectionFrame, in const SSSInfo sssInfo, in const uint channel, in const SSSSample sssSample, out uint intersectionId) {
    const vec3 difference = sssSample.position - sssInfo.position;
    const float distanceInTangetPlaneX = dot(difference, projectionFrame.t);
    const float distanceInTangetPlaneY = dot(difference, projectionFrame.b);
    const float radius = sqrt(distanceInTangetPlaneX * distanceInTangetPlaneX + distanceInTangetPlaneY * distanceInTangetPlaneY);

    const float sampledScatterDistance = sss_sampling_scatterDistance(channel, sssInfo.scatterDistance);
    const float radiusMax = sss_diffusion_profile_sample(0.999, sampledScatterDistance);

    intersectionId = INVALID_UINT_VALUE;

    const vec3 origin = sssInfo.position + radiusMax * projectionFrame.n + distanceInTangetPlaneX * projectionFrame.t + distanceInTangetPlaneY * projectionFrame.b;
    const vec3 direction = -projectionFrame.n;
    const float sphereFraction = sqrt(radiusMax * radiusMax - radius * radius);
    const float tMin = radiusMax - sphereFraction;
    const float tMax = radiusMax + sphereFraction;

    if (!sss_sampling_disk_sample_intersectionId(origin, direction, tMin, tMax, sssInfo.objectDescriptorId, sssSample.triangleId, sssSample.barycentrics, intersectionId)) {
        return false;
    }

    return true;
}

#endif