#ifndef SSS_DIFFUSION_PROFILE_BURLEY_GLSL
#define SSS_DIFFUSION_PROFILE_BURLEY_GLSL

#include "sss_diffusion_profile.glsl"

// d = l / s <- mfp / scaling
float sss_diffusion_profile_burley_cdfDiffusion(in const float r, in const float d) {
    return 1.0 - 0.25 * exp(-r / d) - 0.75 * exp(-r / (3.0 * d));
}

float sss_diffusion_profile_burley_cdfFirstDerivativeDiffusion(in const float r, in const float d) {
    return 0.25 / d * (exp(-r / d) + exp(-r / (3.0 * d)));
}

float sss_diffusion_profile_burley_cdfSecondDerivativeDiffusion(in const float r, in const float d) {
    return -0.25 / (d * d) * (exp(-r / d) + exp(-r / (3.0 * d)) / 3.0);
}

vec3 sss_diffusion_profile_scatterDistance(in const vec3 surfaceAlbedo) {
    const vec3 a = surfaceAlbedo - vec3(0.8);
    return 1.9 - surfaceAlbedo + 3.5 * a * a;
}

vec3 sss_diffusion_profile_evaluate(in const float radius, in const vec3 scatterDistance) {
    if (radius <= 0) {
        return vec3(0.25 / PI) / max(vec3(0.000001), scatterDistance);
    }
    const vec3 rd = radius / scatterDistance;
    return (exp(-rd) + exp(-rd / 3.0)) / max(vec3(0.000001), (8.0 * PI * scatterDistance * radius));
}

float sss_diffusion_profile_pdf(in const float radius, in const float scatterDistance) {
    if (radius <= 0) {
        return (0.25 / PI) / max(0.000001, scatterDistance);
    }
    const float rd = radius / scatterDistance;
    return (exp(-rd) + exp(-rd / 3.0)) / max(0.000001, (8.0 * PI * scatterDistance * radius));// divide by r to convert from polar to cartesian
}

vec3 sss_diffusion_profile_pdf_vectorized(in const float radius, in const vec3 scatterDistance) {
    if (radius <= 0) {
        return (0.25 / PI) / max(vec3(0.000001), scatterDistance);
    }
    const vec3 rd = radius / scatterDistance;
    return (exp(-rd) + exp(-rd / 3.0)) / max(vec3(0.000001), (8.0 * PI * scatterDistance * radius));// divide by r to convert from polar to cartesian
}

#define LOG2_E 1.44269504089
//// https://zero-radiance.github.io/post/sampling-diffusion/
//// Performs sampling of a Normalized Burley diffusion profile in polar coordinates.
//// 'u' is the random number (the value of the CDF): [0, 1).
//// rcp(s) = 1 / ShapeParam = ScatteringDistance.
//// 'r' is the sampled radial distance, s.t. (u = 0 -> r = 0) and (u = 1 -> r = Inf).
//// rcp(Pdf) is the reciprocal of the corresponding PDF value.
float sampleBurleyDiffusionProfileAnalytical(in float u, in const float rcpS) {
    u = 1 - u;// Convert CDF to CCDF; the resulting value of (u != 0)

    const float g = 1 + (4 * u) * (2 * u + sqrt(1 + (4 * u) * u));
    const float n = exp2(log2(g) * (-1.0/3.0));// g^(-1/3)
    const float p = (g * n) * n;// g^(+1/3)
    const float c = 1 + p + n;// 1 + g^(+1/3) + g^(-1/3)
    const float x = (3 / LOG2_E) * log2(c / (4 * u));// 3 * Log[c / (4 * u)]

    // x      = s * r
    // exp_13 = Exp[-x/3] = Exp[-1/3 * 3 * Log[c / (4 * u)]]
    // exp_13 = Exp[-Log[c / (4 * u)]] = (4 * u) / c
    // exp_1  = Exp[-x] = exp_13 * exp_13 * exp_13
    // expSum = exp_1 + exp_13 = exp_13 * (1 + exp_13 * exp_13)
    // rcpExp = rcp(expSum) = c^3 / ((4 * u) * (c^2 + 16 * u^2))
    const float rcpExp = ((c * c) * c) / ((4 * u) * ((c * c) + (4 * u) * (4 * u)));

    return x * rcpS; // r
}


float sss_diffusion_profile_sample(in const float xi, in const float scatterDistance) {
    return sampleBurleyDiffusionProfileAnalytical(xi, scatterDistance);

    // SOLVE for r: xi = cdf(r,d)
    // use Halley's method to perform numerical inversion (converges in 2-4 iterations) -> https://en.wikipedia.org/wiki/Halley%27s_method
    // this is suggested by https://advances.realtimerendering.com/s2018/Efficient%20screen%20space%20subsurface%20scattering%20Siggraph%202018.pdf

    //    // xi = cdf(r,d) <=> 0 = cdf(r,d) - xi =: f
    //    float r = 0.0;
    //    for (int i = 0; i < 4; i++) {
    //        float f = sss_diffusion_profile_burley_cdfDiffusion(r, scatterDistance) - xi;
    //        float df = sss_diffusion_profile_burley_cdfFirstDerivativeDiffusion(r, scatterDistance);
    //        float ddf = sss_diffusion_profile_burley_cdfSecondDerivativeDiffusion(r, scatterDistance);
    //
    //        r = r - 2.0 * f * df / (2.0 * df * df - f * ddf);
    //    }
    //
    //    return r;
}

#endif