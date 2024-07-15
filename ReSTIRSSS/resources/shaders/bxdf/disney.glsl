#ifndef BSDF_DISNEY_GLSL
#define BSDF_DISNEY_GLSL

#include "diffusion/sss_diffusion_profile_burley.glsl"
#include "diffusion/sss_sampling_disk.glsl"

#include "bsdf/bsdf.glsl"
#include "bsdf/disney_bsdf.glsl"

#define SCATTERING_TYPE_BSDF 0x1
#define SCATTERING_TYPE_BSSRDF 0x2

// === SCATTERING TYPE ===
uint disney_sampleScatteringType(inout uint rngState, in const bool sss, in const uint bounce, in const float sssFactor, out float pdf) {
    if (!sss || bounce > 0) {
        pdf = 1.0;
        return SCATTERING_TYPE_BSDF;
    }
    if (sssFactor > 0) {
        pdf = 1.0;
        return SCATTERING_TYPE_BSSRDF;
    }
    pdf = 1.0;
    return SCATTERING_TYPE_BSDF;
}

uint disney_sampleScatteringTypeWOSSS(inout uint rngState, in const bool sss, in const uint bounce, in const float sssFactor, out float pdf) {
    pdf = 1.0;
    return SCATTERING_TYPE_BSDF;
}

// === UTILITY ===
float disney_schlickWeight(in const float a) {
    const float b = clamp(1.0 - a, 0.0, 1.0);
    const float bb = b * b;
    return bb * bb * b;
}

float disney_diffuseLambertWeight(in const float fv, in const float fl) {
    return (1.0 - 0.5 * fl) * (1.0 - 0.5 * fv);
}

float disney_diffuseLambertWeightSingle(in const float f) {
    return 1.0 - 0.5 * f;
}

// === BSSRDF ===
vec3 disney_bssrdf_fresnel_evaluate(in const vec3 normal, in const vec3 direction) {
//    return vec3(1); // setting this to 1 ensures smooth transition to Lambert diffuse

    const float dotND = dot(normal, direction);
    const float schlick = disney_schlickWeight(dotND);
    const float lambertWeight = disney_diffuseLambertWeightSingle(schlick);
    return vec3(lambertWeight);
}

void disney_bssrdf_evaluate(in const vec3 normal, in const vec3 v, in const float distance, in const vec3 scatterDistance, in const vec3 surfaceAlbedo, out vec3 bssrdf) {
    const vec3 diffusionProfile = surfaceAlbedo * sss_diffusion_profile_evaluate(distance, scatterDistance);

    bssrdf = diffusionProfile / PI * disney_bssrdf_fresnel_evaluate(normal, v);
}

void disney_bssrdf_evaluate(in const vec3 normal, in const vec3 v, in const vec3 normalSample, in const vec3 l, in const float distance, in const vec3 scatterDistance, in const vec3 surfaceAlbedo, out vec3 bssrdf, out vec3 bsdf) {
    const vec3 diffusionProfile = surfaceAlbedo * sss_diffusion_profile_evaluate(distance, scatterDistance);

    bssrdf = diffusionProfile / PI * disney_bssrdf_fresnel_evaluate(normal, v);
    bsdf = disney_bssrdf_fresnel_evaluate(normalSample, l);
}
#endif