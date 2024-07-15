#ifndef ENVIRONMENT_GLSL
#define ENVIRONMENT_GLSL

vec3 environment_evaluate(in const vec3 direction) {
    if (g_environment_map == 0) {
        return g_sky_color;
    }
    const vec2 longitudeLatitude = vec2((atan(direction.x, direction.z) / PI + 1.0) * 0.5 + g_environment_map_rotation.x, asin(-direction.y) / PI + 0.5 + g_environment_map_rotation.y);
    return texture(textureEnvironment, longitudeLatitude).rgb;
}

#endif