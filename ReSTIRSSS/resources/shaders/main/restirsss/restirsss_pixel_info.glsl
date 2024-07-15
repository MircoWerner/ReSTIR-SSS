#ifndef RESTIR_RESERVOIR_GLSL
#define RESTIR_RESERVOIR_GLSL

#include "../../utility/random.glsl"
#include "../../bxdf/bsdf/bsdf.glsl"

struct ReSTIRPixelInfo {
    BSDFMaterial material;
    vec3 v;// inverse ray direction, i.e. from surface to camera or previous bounce
    vec3 position;
    vec3 geometricNormal;
    vec3 normal;
};

bool restirsss_pixel_info_isSSS(in const ReSTIRPixelInfo pixelInfo) {
    return g_sss > 0 && pixelInfo.material.subsurface > 0;
}

#endif