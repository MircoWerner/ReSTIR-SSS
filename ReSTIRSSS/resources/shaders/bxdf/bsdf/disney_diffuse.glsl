#ifndef BSDF_DISNEY_DIFFUSE_GLSL
#define BSDF_DISNEY_DIFFUSE_GLSL

#include "bsdf.glsl"

float fresnelDiffuse90(in const float roughness, in const vec3 dirOut, in const vec3 halfVector) {
    return 0.5 + 2.0 * roughness * dot(halfVector, dirOut) * dot(halfVector, dirOut);
}

float fresnelDiffuse(in const float fd90, in const BSDFFrame frame, in const vec3 omega) {
    return 1.0 + (fd90 - 1.0) * pow(1.0 - max(dot(frame.n, omega), 0), 5);
}

vec3 sampleCosHemisphere(in const vec2 rndParam) {
//    float tmp = sqrt(clamp(1 - rndParam[0], 0, 1));
//    float phi = TWO_PI * rndParam[1];
//    return vec3(cos(phi) * tmp, sin(phi) * tmp, sqrt(clamp(rndParam[0], 0, 1)));
    float theta = acos(sqrt(1 - rndParam[0]));
    float phi = TWO_PI * rndParam[1];
    vec3 omegaLocal = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    return omegaLocal;
}

vec2 reverseSampleCosHemisphere(in const vec3 omegaLocal) {
    float cosTheta = omegaLocal.z;
    const float reverseXi1 = 1 - cosTheta * cosTheta;
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    float sinPhi = omegaLocal.y / sinTheta;
    float cosPhi = omegaLocal.x / sinTheta;
    // get angle in [0,2Pi]
    float reversePhi = atan(sinPhi, cosPhi);
    reversePhi = reversePhi >= 0 ? reversePhi : reversePhi + 2 * PI;
    const float reverseXi2 = reversePhi / (2 * PI);
    return vec2(reverseXi1, reverseXi2);
}

//#undef ALPHA
//#define ALPHA 2
//vec3 sampleCosHemisphere(in vec2 rndParam) {
//    //    float tmp = sqrt(clamp(1 - rndParam[0], 0, 1));
//    //    float phi = TWO_PI * rndParam[1];
//    //    return vec3(cos(phi) * tmp, sin(phi) * tmp, sqrt(clamp(rndParam[0], 0, 1)));
//    float theta = acos(pow(1 - rndParam[0], 1.0 / (ALPHA + 1.0)));
//    float phi = TWO_PI * rndParam[1];
//    vec3 omegaLocal = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
//    return omegaLocal;
//}
//
//vec2 reverseSampleCosHemisphere(in vec3 omegaLocal) {
//    float cosTheta = omegaLocal.z;
//    const float reverseXi1 = 1 - pow(cosTheta, ALPHA + 1);
//    float sinTheta = sqrt(1 - pow(cosTheta, ALPHA + 1));
//    float sinPhi = omegaLocal.y / sinTheta;
//    float cosPhi = omegaLocal.x / sinTheta;
//    // get angle in [0,2Pi]
//    float reversePhi = atan(sinPhi, cosPhi);
//    reversePhi = reversePhi >= 0 ? reversePhi : reversePhi + 2 * PI;
//    const float reverseXi2 = reversePhi / (2 * PI);
//    return vec2(reverseXi1, reverseXi2);
//}

vec3 bsdf_disney_diffuse_evaluate(in const BSDFVertex vertex, in BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, in const vec3 dirOut) {
    if (dot(vertex.geometricNormal, dirIn) < 0 ||
    dot(vertex.geometricNormal, dirOut) < 0) {
        // No light below the surface
        return vec3(0);
    }

    // Flip the shading frame if it is inconsistent with the geometry normal
    if (dot(frame.n, dirIn) < 0) {
        frame.t = -frame.t;
        frame.b = -frame.b;
        frame.n = -frame.n;
    }

    const vec3 halfVector = normalize(dirIn + dirOut);

    // base diffuse
    const float fd90 = fresnelDiffuse90(material.roughness, dirOut, halfVector);
    const vec3 baseDiffuse = material.baseColor / PI
    * fresnelDiffuse(fd90, frame, dirIn) * fresnelDiffuse(fd90, frame, dirOut)
    * max(dot(frame.n, dirOut), 0);

    return baseDiffuse;
}

bool bsdf_disney_diffuse_sample(in const BSDFVertex vertex, in BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, out vec3 dirOut, in const vec2 rndParam) {
    if (dot(vertex.geometricNormal, dirIn) < 0) {
        // No light below the surface
        return false;
    }

    // Flip the shading frame if it is inconsistent with the geometry normal
    if (dot(frame.n, dirIn) < 0) {
        frame.t = -frame.t;
        frame.b = -frame.b;
        frame.n = -frame.n;
    }

    dirOut = toWorld(frame, sampleCosHemisphere(rndParam));
    return true;
}

float bsdf_disney_diffuse_pdf(in const BSDFVertex vertex, in BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, in const vec3 dirOut) {
    if (dot(vertex.geometricNormal, dirIn) < 0 ||
    dot(vertex.geometricNormal, dirOut) < 0) {
        // No light below the surface
        return 0;
    }

    // Flip the shading frame if it is inconsistent with the geometry normal
    if (dot(frame.n, dirIn) < 0) {
        frame.t = -frame.t;
        frame.b = -frame.b;
        frame.n = -frame.n;
    }

    return max(dot(frame.n, dirOut), 0) / PI;
//    return (ALPHA + 1.0) * pow(max(0, dot(dirOut, frame.n)), ALPHA) / (2.0 * PI);
}

#endif