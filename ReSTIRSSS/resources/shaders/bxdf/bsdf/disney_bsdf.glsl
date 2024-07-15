#ifndef BSDF_DISNEYA_GLSL
#define BSDF_DISNEYA_GLSL

#include "bsdf.glsl"
#include "disney_diffuse.glsl"
#include "disney_metal.glsl"

float bsdf_disney_lobe_weight(in const BSDFMaterial material, in const float rndLobe) {
    const float metallic = material.metallic;
    const float diffuseWeight = 1.0 - metallic;
    const float metalWeight = metallic;
    const float sumWeights = diffuseWeight + metalWeight;

    if (rndLobe < diffuseWeight / sumWeights) {
        return diffuseWeight / sumWeights;
    } else {
        return metalWeight / sumWeights;
    }
}

uint bsdf_disney_lobe_index(in const BSDFMaterial material, in const float rndLobe) {
    const float metallic = material.metallic;
    const float diffuseWeight = 1.0 - metallic;
    const float metalWeight = metallic;
    const float sumWeights = diffuseWeight + metalWeight;

    if (rndLobe < diffuseWeight / sumWeights) {
        return 0;
    } else {
        return 1;
    }
}

vec3 bsdf_disney_evaluate(in const BSDFVertex vertex, in const BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, in const vec3 dirOut) {
    const vec3 diffuse = bsdf_disney_diffuse_evaluate(vertex, frame, material, dirIn, dirOut);
    const vec3 metal = bsdf_disney_metal_evaluate(vertex, frame, material, dirIn, dirOut);

    const float metallic = material.metallic;

    return (1.0 - metallic) * diffuse + metallic * metal;
}

bool bsdf_disney_sample(in const BSDFVertex vertex, in const BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, out vec3 dirOut, in const uint lobe, in const vec2 rndParam) {
    if (lobe == 0) {
        return bsdf_disney_diffuse_sample(vertex, frame, material, dirIn, dirOut, rndParam);
    } else {
        return bsdf_disney_metal_sample(vertex, frame, material, dirIn, dirOut, rndParam);
    }
}

float bsdf_disney_pdf(in const BSDFVertex vertex, in const BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, in const vec3 dirOut) {
    const float diffusePDF = bsdf_disney_diffuse_pdf(vertex, frame, material, dirIn, dirOut);
    const float metalPDF = bsdf_disney_metal_pdf(vertex, frame, material, dirIn, dirOut);

    const float metallic = material.metallic;
    const float diffuseWeight = 1.0 - metallic;
    const float metalWeight = metallic;
    const float sumWeights = diffuseWeight + metalWeight;

    return (diffuseWeight * diffusePDF + metalWeight * metalPDF) / sumWeights;
}

#endif