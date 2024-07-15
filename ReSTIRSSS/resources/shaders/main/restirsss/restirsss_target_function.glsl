#ifndef RESTIRSSS_TARGET_FUNCTION_GLSL
#define RESTIRSSS_TARGET_FUNCTION_GLSL

float restirsss_target_function_evaluate(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample);

/**
* Used to combine multiple reservoirs.
* Either the same as the default restir_target_function_evaluate or include additional checks, e.g. visibility check for ReSTIR DI, to stay unbiased.
*/
float restirsss_target_function_resampleEvaluate(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample, in const bool visibilityCheck);

// PRIVATE
#include "../../bxdf/disney.glsl"
#include "../../visibility/visibility.glsl"

vec3 restirsss_target_function_unshadowedLightContribution(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample) {
    if (restirsss_reservoir_isNullSample(evalSample)) {
        return vec3(0);
    }

    vec3 unshadowedLightContribution = vec3(1);

    if (restirsss_pixel_info_isSSS(pixelInfo)) {
        // bssrdf
        const float actualDistance = distance(pixelInfo.position, evalSample.samplePosition);

        const vec3 surfaceToLight = evalSample.lightPosition - evalSample.samplePosition;
        const vec3 surfaceToLightNormalized = normalize(surfaceToLight);

        vec3 bssrdf;
        vec3 bsdf;
        disney_bssrdf_evaluate(pixelInfo.normal, pixelInfo.v, evalSample.sampleNormal, surfaceToLightNormalized, actualDistance, pixelInfo.material.scatterDistance, pixelInfo.material.baseColor, bssrdf, bsdf);

        const float cosAtSurface = max(0, dot(evalSample.sampleNormal, surfaceToLightNormalized));

        unshadowedLightContribution *= bssrdf * bsdf * cosAtSurface * evalSample.lightEmission;
    } else {
        // bsdf
        const vec3 surfaceToLight = evalSample.lightPosition - pixelInfo.position;
        const vec3 surfaceToLightNormalized = normalize(surfaceToLight);

        const BSDFVertex vertex = BSDFVertex(pixelInfo.geometricNormal);
        const BSDFFrame frame = coordinateSystem(pixelInfo.normal);
        const vec3 bsdf = bsdf_disney_evaluate(vertex, frame, pixelInfo.material, pixelInfo.v, surfaceToLightNormalized);

        const float cosAtSurface = max(0, dot(pixelInfo.normal, surfaceToLightNormalized));

        unshadowedLightContribution *= bsdf * cosAtSurface * evalSample.lightEmission;
    }

    //    unshadowedLightContribution *= visibility_shadowRay(evalSample.samplePosition, evalSample.sampleGeometricNormal, surfaceToLightNormalized, evalSample.lightObjectDescriptor, evalSample.lightTriangleId);

    return unshadowedLightContribution;
}

vec3 restirsss_measurementContributionFunction(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample) {
    const vec3 color = restirsss_target_function_unshadowedLightContribution(pixelInfo, evalSample);

//    if (g_restir_visibility == 0) {
//        // no visibility pass, check if chosen light is visible from the chosen sample position
//        vec3 surfacePosition;
//        vec3 surfaceGeometricNormal;
//        vec3 surfaceToLightNormalized;
//        if (restirsss_pixel_info_isSSS(pixelInfo)) {
//            surfacePosition = evalSample.samplePosition;
//            surfaceGeometricNormal = evalSample.sampleGeometricNormal;
//            surfaceToLightNormalized = normalize(evalSample.lightPosition - evalSample.samplePosition);
//        } else {
//            surfacePosition = pixelInfo.position;
//            surfaceGeometricNormal = pixelInfo.geometricNormal;
//            surfaceToLightNormalized = normalize(evalSample.lightPosition - pixelInfo.position);
//        }
//        color *= visibility_shadowRay(surfacePosition, surfaceGeometricNormal, surfaceToLightNormalized, evalSample.lightObjectDescriptor, evalSample.lightTriangleId);
//    }
    return color;
}

float calcLuminance(in const vec3 color) {
    return dot(color.xyz, vec3(0.299f, 0.587f, 0.114f));
}

// implement "base class"
float restirsss_target_function_evaluate(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample) {
    const vec3 unshadowedLightContribution = restirsss_target_function_unshadowedLightContribution(pixelInfo, evalSample);

    //    float targetFunction = length(unshadowedLightContribution);// ^p(X_i)
    //    targetFunction = (isnan(targetFunction) || isinf(targetFunction)) ? 0 : targetFunction;
    const float targetFunction = calcLuminance(unshadowedLightContribution);

    return targetFunction;
}

// implement "base class"
float restirsss_target_function_resampleEvaluate(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample, in const bool visibilityCheck) {
    if (visibilityCheck) {
        vec3 surfacePosition;
        vec3 surfaceGeometricNormal;
        vec3 surfaceToLightNormalized;
        float tMax = 0;
        if (restirsss_pixel_info_isSSS(pixelInfo)) {
            surfacePosition = evalSample.samplePosition;
            surfaceGeometricNormal = evalSample.sampleGeometricNormal;
            const vec3 d = evalSample.lightPosition - evalSample.samplePosition;
            surfaceToLightNormalized = normalize(d);
            tMax = length(d);
        } else {
            surfacePosition = pixelInfo.position;
            surfaceGeometricNormal = pixelInfo.geometricNormal;
            const vec3 d = evalSample.lightPosition - pixelInfo.position;
            surfaceToLightNormalized = normalize(d);
            tMax = length(d);
        }
        if (!visibility_shadowRay(surfacePosition, surfaceGeometricNormal, surfaceToLightNormalized, tMax, evalSample.lightObjectDescriptor, evalSample.lightTriangleId)) {
            return 0;
        }
    }
    return restirsss_target_function_evaluate(pixelInfo, evalSample);
}

#endif