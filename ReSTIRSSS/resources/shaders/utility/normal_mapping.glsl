#ifndef UTILITY_NORMAL_MAPPING_GLSL
#define UTILITY_NORMAL_MAPPING_GLSL

mat3 tangentSpaceMatrix(in const vec3 tangent, in const vec3 bitangent, in const vec3 normal) {
    return transpose(inverse(mat3(tangent, bitangent, normal)));
//    return mat3(tangent, bitangent, normal);
}

#endif