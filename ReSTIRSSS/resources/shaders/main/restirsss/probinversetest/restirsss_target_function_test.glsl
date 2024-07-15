#ifndef RESTIRSSS_TARGET_FUNCTION_TEST_GLSL
#define RESTIRSSS_TARGET_FUNCTION_TEST_GLSL

float restirsss_target_function_evaluate(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample);

/**
* Used to combine multiple reservoirs.
* Either the same as the default restir_target_function_evaluate or include additional checks, e.g. visibility check for ReSTIR DI, to stay unbiased.
*/
float restirsss_target_function_resampleEvaluate(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample, in const bool visibilityCheck);

// PRIVATE
#include "../../bxdf/disney.glsl"
#include "../../visibility/visibility.glsl"
#include "../../material.glsl"

vec3 restirsss_target_function_unshadowedLightContribution(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample) {
    if (restirsss_reservoir_isNullSample(evalSample)) {
        return vec3(0);
    }

    const vec3 x1x2 = evalSample.samplePosition - pixelInfo.position;// visibile point -> x2
    const vec3 x1x2Normalized = normalize(x1x2);
    const vec3 x2x3 = evalSample.lightPosition - evalSample.samplePosition;// x2 -> light source
    const vec3 x2x3Normalized = normalize(x2x3);

    vec3 unshadowedLightContribution = vec3(1);

    const BSDFVertex vertex = BSDFVertex(pixelInfo.geometricNormal);
    const BSDFFrame frame = coordinateSystem(pixelInfo.normal);
    // bsdf at x1
    unshadowedLightContribution *= bsdf_disney_evaluate(vertex, frame, pixelInfo.material, pixelInfo.v, x1x2Normalized);

    // bsdf at x2
    BSDFVertex vertexAtX2;
    BSDFFrame frameAtX2;
    BSDFMaterial materialAtX2;
    {
        const ObjectDescriptor objDesc = g_objectDescriptors[evalSample.sampleObjectDescriptor];
        Indices indices = Indices(objDesc.indexAddress);
        Vertices vertices = Vertices(objDesc.vertexAddress);
        const Material material = g_materials[objDesc.materialId];

        const ivec3 ind = indices.i[evalSample.sampleTriangleId];
        const Vertex v0 = vertices.v[ind.x];
        const Vertex v1 = vertices.v[ind.y];
        const Vertex v2 = vertices.v[ind.z];

        const vec2 uv0 = vec2(v0.texCoord_u, v0.texCoord_v);
        const vec2 uv1 = vec2(v1.texCoord_u, v1.texCoord_v);
        const vec2 uv2 = vec2(v2.texCoord_u, v2.texCoord_v);

        const vec3 barycentrics = vec3(1.0 - evalSample.sampleBarycentrics.x - evalSample.sampleBarycentrics.y, evalSample.sampleBarycentrics.x, evalSample.sampleBarycentrics.y);

        // UV
        const vec2 uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;

        vertexAtX2 = BSDFVertex(evalSample.sampleGeometricNormal);
        frameAtX2 = coordinateSystem(evalSample.sampleNormal);
        materialAtX2 = bsdfMaterialInit(material, uv);
    }
    unshadowedLightContribution *= bsdf_disney_evaluate(vertexAtX2, frameAtX2, materialAtX2, -x1x2Normalized, x2x3Normalized);

    // light
    const float cosAtSurface = max(0, dot(evalSample.sampleNormal, x2x3Normalized));
    //    float cosAtLight = max(0, dot(evalSample.lightNormal, -x2x3Normalized));
    //    float geometryTerm = cosAtSurface * cosAtLight / dot(x2x3, x2x3);
    unshadowedLightContribution *= evalSample.lightEmission;// * cosAtSurface /** geometryTerm*/;

    return unshadowedLightContribution;
}

vec3 restirsss_measurementContributionFunction(in const ReSTIRPixelInfo pixelInfo, in const ReSTIRSSSSample evalSample) {
    const vec3 color = restirsss_target_function_unshadowedLightContribution(pixelInfo, evalSample);

    //    if (g_restir_visibility == 0) {
    //        // no visibility pass, check if chosen light is visible from the chosen sample position
    //        vec3 surfaceToLightNormalized = normalize(evalSample.lightPosition - evalSample.samplePosition);
    //        color *= visibility_shadowRay(evalSample.samplePosition, evalSample.sampleGeometricNormal, surfaceToLightNormalized, evalSample.lightObjectDescriptor, evalSample.lightTriangleId);
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
    if (restirsss_reservoir_isNullSample(evalSample)) {
        return 0;
    }

    /*if (g_restir_shift == RESTIRSSS_SHIFT_RECONNECTION || g_restir_shift == RESTIRSSS_SHIFT_SEQUENTIAL_RECONNECTION)*/ /* unoptimized but ok for this debug case */ {
        // the proposal function ensures that [x1,x2] is visible, so we have to ensure this for resampling as well to stay unbiased
        const vec3 surfaceToX2 = evalSample.samplePosition - pixelInfo.position;
        const vec3 surfaceToX2Normalized = normalize(surfaceToX2);
        if (!visibility_shadowRay(pixelInfo.position, pixelInfo.geometricNormal, surfaceToX2Normalized, length(surfaceToX2), evalSample.sampleObjectDescriptor, evalSample.sampleTriangleId)) {
            return 0;
        }
    }
    if (visibilityCheck) {
        const vec3 surfaceToLight = evalSample.lightPosition - evalSample.samplePosition;
        const vec3 surfaceToLightNormalized = normalize(surfaceToLight);
        if (!visibility_shadowRay(evalSample.samplePosition, evalSample.sampleGeometricNormal, surfaceToLightNormalized, length(surfaceToLight), evalSample.lightObjectDescriptor, evalSample.lightTriangleId)) {
            return 0;
        }
    }
    return restirsss_target_function_evaluate(pixelInfo, evalSample);
}

#endif