#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

vec3 material_getAlbedo(in const Material material, in const vec2 uv) {
    return material.baseColorTexture >= 0 ? texture(textures[material.baseColorTexture], uv).rgb : material.baseColorFactor.rgb;
}

float material_getMetallic(in const Material material, in const vec2 uv) {
    return material.metallicRoughnessTexture >= 0 ? texture(textures[material.metallicRoughnessTexture], uv).b : material.metallicFactor;
}

float material_getRoughness(in const Material material, in const vec2 uv) {
    return material.metallicRoughnessTexture >= 0 ? texture(textures[material.metallicRoughnessTexture], uv).g : material.roughnessFactor;
}

bool material_isLightSource(in const Material material) {
    return any(greaterThan(material.emission, vec3(0)));
}

bool material_isTranslucent(in const Material material) {
    return material.subsurface > 0.0;
}

#endif