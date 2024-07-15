#ifndef UTILITY_RANDOM_GLSL
#define UTILITY_RANDOM_GLSL

#include "constants.glsl"

/**
 * Random number in [0,1].
 * https://www.shadertoy.com/view/XlGcRh
 */
float pcg(inout uint state) {
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    word = (word >> 22u) ^ word;
    return word / 4294967295.0;
}

// https://github.com/nvpro-samples/vk_mini_path_tracer/blob/main/vk_mini_path_tracer/shaders/raytrace.comp.glsl#L25
// Steps the RNG and returns a floating-point value between 0 and 1 inclusive.
float nextFloat(inout uint rngState) {
    // Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
    rngState  = rngState * 747796405 + 1;
    uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
    word      = (word >> 22) ^ word;
    return float(word) / 4294967295.0f;
}

// [0, maxUInt]
// maxUInt inclusive!
uint nextUInt(inout uint rngState, uint maxUInt) {
    rngState  = rngState * 747796405 + 1;
    uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
    word      = (word >> 22) ^ word;
    return word % (maxUInt + 1);
}

#endif