#ifndef SSS_DIFFUSION_PROFILE_GLSL
#define SSS_DIFFUSION_PROFILE_GLSL

vec3 sss_diffusion_profile_scatterDistance(in const vec3 surfaceAlbedo);

vec3 sss_diffusion_profile_evaluate(in const float radius, in const vec3 scatterDistance);

float sss_diffusion_profile_pdf(in const float radius, in const float scatterDistance);
vec3 sss_diffusion_profile_pdf_vectorized(in const float radius, in const vec3 scatterDistance);

float sss_diffusion_profile_sample(in const float xi, in const float scatterDistance);

#endif