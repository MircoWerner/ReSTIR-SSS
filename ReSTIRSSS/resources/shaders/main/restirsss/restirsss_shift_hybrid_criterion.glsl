#ifndef RESTIRSSS_SHIFT_HYBRID_CRITERION_GLSL
#define RESTIRSSS_SHIFT_HYBRID_CRITERION_GLSL

// true if vertex x_2 is connectable from vertex x_1 (or y_1)
bool restirsss_shift_connectable(in const vec3 x1Position, in const vec3 x1Normal, in const vec3 x2Position, in const vec3 x2Normal, in const vec3 scatterDistance) {
    // connectable if the angle between the normals is less than 60 degrees or the distance is greater than 2 times the maximum scatter distance
    return dot(x1Normal, x2Normal) <= 0.5 || distance(x1Position, x2Position) >= 2.0 * max(scatterDistance.r, max(scatterDistance.g, scatterDistance.b));
}

#endif