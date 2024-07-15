#ifndef BSDF_DISNEY_METAL_GLSL
#define BSDF_DISNEY_METAL_GLSL

#include "bsdf.glsl"

//#undef ALPHA
//#define ALPHA 0

float smithMaskingGTR2(in const vec3 omegaLocal, in const float alphaX, in const float alphaY) {
    return 1.0 / (1.0 + (sqrt(1.0 + (omegaLocal.x * omegaLocal.x * alphaX * alphaX + omegaLocal.y * omegaLocal.y * alphaY * alphaY) / (omegaLocal.z * omegaLocal.z)) - 1.0) / 2.0);
}

float GTR2(in const vec3 halfVectorLocal, in const float alphaX, in const float alphaY) {
    return 1.0 / (PI * alphaX * alphaY * pow(halfVectorLocal.x * halfVectorLocal.x / (alphaX * alphaX) + halfVectorLocal.y * halfVectorLocal.y / (alphaY * alphaY) + halfVectorLocal.z * halfVectorLocal.z, 2));
}

/// See "Sampling the GGX Distribution of Visible Normals", Heitz, 2018.
/// https://jcgt.org/published/0007/04/01/
vec3 sampleVisibleNormals(in vec3 localDirIn, in const float alphaX, in const float alphaY, in const vec2 rndParam) {
    // The incoming direction is in the "ellipsodial configuration" in Heitz's paper
    bool changeSign = false;
    if (localDirIn.z < 0) {
        // Ensure the input is on top of the surface.
        changeSign = true;
        localDirIn = -localDirIn;
    }

    // Transform the incoming direction to the "hemisphere configuration".
    const vec3 hemiDirIn = normalize(vec3(alphaX * localDirIn.x, alphaY * localDirIn.y, localDirIn.z));

    // Parameterization of the projected area of a hemisphere.
    // First, sample a disk.
    const float r = sqrt(rndParam.x);
    const float phi = 2.0 * PI * rndParam.y;
    const float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    // Vertically scale the position of a sample to account for the projection.
    const float s = (1.0 + hemiDirIn.z) / 2.0;
    t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;
    // Point in the disk space
    const float ti = sqrt(max(0, 1.0 - t1*t1 - t2*t2));
    //    vec3 diskN = vec3(t1, t2, sqrt(max(0, 1.0 - t1*t1 - t2*t2)));

    // Reprojection onto hemisphere -- we get our sampled normal in hemisphere space.
    const BSDFFrame hemiFrame = coordinateSystem(hemiDirIn);

    const vec3 wm = t1 * hemiFrame.t + t2 * hemiFrame.b + ti * hemiFrame.n;

    // Transforming the normal back to the ellipsoid configuration
    const vec3 halfVectorLocal = normalize(vec3(alphaX * wm.x, alphaY * wm.y, max(0, wm.z)));

    return changeSign ? -halfVectorLocal : halfVectorLocal;
}


/// See "Sampling the GGX Distribution of Visible Normals", Heitz, 2018.
/// https://jcgt.org/published/0007/04/01/
vec2 reverseSampleVisibleNormals(in vec3 localDirIn, in const float alphaX, in const float alphaY, in vec3 halfVectorLocal) {
    // The incoming direction is in the "ellipsodial configuration" in Heitz's paper
    if (localDirIn.z < 0) {
        // Ensure the input is on top of the surface.
        localDirIn = -localDirIn;
        halfVectorLocal = -halfVectorLocal;
    }

    // Transform the incoming direction to the "hemisphere configuration".
    const vec3 hemiDirIn = normalize(vec3(alphaX * localDirIn.x, alphaY * localDirIn.y, localDirIn.z));

    // Vertically scale the position of a sample to account for the projection.
    const float s = (1.0 + hemiDirIn.z) / 2.0;

    // Reprojection onto hemisphere -- we get our sampled normal in hemisphere space.
    const BSDFFrame hemiFrame = coordinateSystem(hemiDirIn);
    //    vec3 w1 = hemiFrame.t;
    //    vec3 w2 = hemiFrame.b;
    //    vec3 wi = hemiFrame.n;

    float reverseT1;
    float reverseT2;
    {
        // https://www.wolframalpha.com/
        // solve a*j*x+a*m*y+a*p*z=s, b*k*x+b*n*y+b*q*z=t, l*x+o*y+r*z=u for x,y,z
        const float a = alphaX;
        const float b = alphaY;

        const float s = halfVectorLocal.x;
        const float t = halfVectorLocal.y;
        const float u = halfVectorLocal.z;

        const float j = hemiFrame.t.x;
        const float k = hemiFrame.t.y;
        const float l = hemiFrame.t.z;
        const float m = hemiFrame.b.x;
        const float n = hemiFrame.b.y;
        const float o = hemiFrame.b.z;
        const float p = hemiFrame.n.x;
        const float q = hemiFrame.n.y;
        const float r = hemiFrame.n.z;

        const float denominator = (a * b * (-j * n * r + j * o * q + k * m * r - k * o * p - l * m * q + l * n * p));
        const float x = - (a * b * m * q * u - a * b * n * p * u - a * m * r * t + a * o * p * t + b * n * r * s - b * o * q * s) / denominator;// t1 / lambda
        const float y = - (-a * b * j * q * u + a * b * k * p * u + a * j * r * t - a * l * p * t - b * k * r * s + b * l * q * s) / denominator;// t2 / lambda
        const float z = - (a * b * j * n * u - a * b * k * m * u - a * j * o * t + a * l * m * t + b * k * o * s - b * l * n * s) / denominator;// ti / lambda

        const float lambdaSquared = 1.0 / (x * x + y * y + z * z);
        const float reverseLambda = sqrt(lambdaSquared);
        reverseT1 = x * reverseLambda;
        reverseT2 = y * reverseLambda;
    }
    reverseT2 = (reverseT2 - (1 - s) * sqrt(1 - reverseT1 * reverseT1)) / s;

    const float reverseR = sqrt(reverseT1 * reverseT1 + reverseT2 * reverseT2);

    const float reverseCosPhi = reverseT1 / reverseR;
    const float reverseSinPhi = reverseT2 / reverseR;
    float reversePhi = atan(reverseSinPhi, reverseCosPhi);
    reversePhi = reversePhi >= 0 ? reversePhi : reversePhi + 2 * PI;

    return vec2(reverseR * reverseR, reversePhi / (2 * PI));
}

//vec3 sampleUniformHemisphere(in vec2 rndParam) {
//    //    float tmp = sqrt(clamp(1 - rndParam[0], 0, 1));
//    //    float phi = TWO_PI * rndParam[1];
//    //    return vec3(cos(phi) * tmp, sin(phi) * tmp, sqrt(clamp(rndParam[0], 0, 1)));
//    float theta = acos(pow(1 - rndParam[0], 1.0 / (ALPHA + 1.0)));
//    float phi = TWO_PI * rndParam[1];
//    vec3 omegaLocal = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
//    return omegaLocal;
//}
//
//vec2 reverseSampleUniformHemisphere(in vec3 omegaLocal) {
//    float cosTheta = omegaLocal.z;
//    const float reverseXi1 = 1 - pow(cosTheta, ALPHA + 1);
//    float sinTheta = sqrt(1 - pow(cosTheta, ALPHA + 1));
//    float sinPhi = omegaLocal.y / sinTheta;
//    float cosPhi = omegaLocal.x / sinTheta;
//    // get angle in [0,2Pi]
//    float reversePhi = atan(sinPhi, cosPhi);
//    reversePhi = reversePhi >= 0 ? reversePhi : reversePhi + 2 * PI;
//    const float reverseXi2 = reversePhi / (2 * PI);
//    return vec2(reverseXi1, reverseXi2);
//}

vec3 bsdf_disney_metal_evaluate(in const BSDFVertex vertex, in BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, in const vec3 dirOut) {
    if (dot(vertex.geometricNormal, dirIn) < 0 ||
    dot(vertex.geometricNormal, dirOut) < 0) {
        // No light below the surface
        return vec3(0);
    }

    // Flip the shading frame if it is inconsistent with the geometry normal
    if (dot(frame.n, dirIn) < 0) {
        frame.t = -frame.t;
        frame.b = -frame.b;
        frame.n = -frame.n;
    }

    const vec3 halfVector = normalize(dirIn + dirOut);

    if (dot(frame.n, dirOut) <= 0 || dot(frame.n, halfVector) <= 0) {
        return vec3(0);
    }

    const vec3 dirInLocal = toLocal(frame, dirIn);
    const vec3 dirOutLocal = toLocal(frame, dirOut);
    const vec3 halfVectorLocal = toLocal(frame, halfVector);
    const float aspect = sqrt(1 - 0.9 * material.anisotropic);
    const float alphaMin = 0.0001;
    const float alphaX = max(alphaMin, material.roughness * material.roughness / aspect);
    const float alphaY = max(alphaMin, material.roughness * material.roughness * aspect);

    const vec3 fresnel = material.baseColor + (1.0 - material.baseColor) * pow(1.0 - max(dot(halfVector, dirOut), 0), 5);
    const float normalDistrubutionFunction = GTR2(halfVectorLocal, alphaX, alphaY);
    const float shadowingMasking = smithMaskingGTR2(dirInLocal, alphaX, alphaY) * smithMaskingGTR2(dirOutLocal, alphaX, alphaY);

    return fresnel * normalDistrubutionFunction * shadowingMasking / (4.0 * max(dot(frame.n, dirIn), 0));
}

bool bsdf_disney_metal_sample(in const BSDFVertex vertex, in BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, out vec3 dirOut, in const vec2 rndParam) {
    if (dot(vertex.geometricNormal, dirIn) < 0) {
        // No light below the surface
        return false;
    }

    // Flip the shading frame if it is inconsistent with the geometry normal
    if (dot(frame.n, dirIn) < 0) {
        frame.t = -frame.t;
        frame.b = -frame.b;
        frame.n = -frame.n;
    }

    const float aspect = sqrt(1.0 - 0.9 * material.anisotropic);
    const float alphaMin = 0.0001;
    const float alphaX = max(alphaMin, material.roughness * material.roughness / aspect);
    const float alphaY = max(alphaMin, material.roughness * material.roughness * aspect);

    const vec3 dirInLocal = toLocal(frame, dirIn);
    const vec3 halfVectorLocal = sampleVisibleNormals(dirInLocal, alphaX, alphaY, rndParam);
    //    vec2 reverseRndParam = reverseSampleVisibleNormals(dirInLocal, alphaX, alphaY, halfVectorLocal);
    //    if (abs(reverseRndParam.x - rndParam.x) < 0.01 && abs(reverseRndParam.y - rndParam.y) < 0.01) {
    //        return false;
    //    }

    // Transform the micro normal to world space
    const vec3 halfVector = toWorld(frame, halfVectorLocal);
    // Reflect over the world space normal
    dirOut = normalize(-dirIn + 2.0 * dot(dirIn, halfVector) * halfVector);

    return true;
}

float bsdf_disney_metal_pdf(in const BSDFVertex vertex, in BSDFFrame frame, in const BSDFMaterial material, in const vec3 dirIn, in const vec3 dirOut) {
    if (dot(vertex.geometricNormal, dirIn) < 0 ||
    dot(vertex.geometricNormal, dirOut) < 0) {
        // No light below the surface
        return 0;
    }

    // Flip the shading frame if it is inconsistent with the geometry normal
    if (dot(frame.n, dirIn) < 0) {
        frame.t = -frame.t;
        frame.b = -frame.b;
        frame.n = -frame.n;
    }

    const vec3 halfVector = normalize(dirIn + dirOut);

    if (dot(frame.n, dirOut) <= 0 || dot(frame.n, halfVector) <= 0) {
        return 0;
    }

    const vec3 dirInLocal = toLocal(frame, dirIn);
    const vec3 halfVectorLocal = toLocal(frame, halfVector);
    const float aspect = sqrt(1.0 - 0.9 * material.anisotropic);
    const float alphaMin = 0.0001;
    const float alphaX = max(alphaMin, material.roughness * material.roughness / aspect);
    const float alphaY = max(alphaMin, material.roughness * material.roughness * aspect);

    const float G = smithMaskingGTR2(dirInLocal, alphaX, alphaY);
    const float D = GTR2(halfVectorLocal, alphaX, alphaY);
    // (4 * cos_theta_v) is the Jacobian of the reflectiokn
    return G * D / (4.0 * dot(frame.n, dirIn));
}

//bool bsdf_disney_metal_sample(in BSDFVertex vertex, in BSDFFrame frame, in BSDFMaterial material, in vec3 dirIn, out vec3 dirOut, in vec2 rndParam) {
//    if (dot(vertex.geometricNormal, dirIn) < 0) {
//        // No light below the surface
//        return false;
//    }
//
//    // Flip the shading frame if it is inconsistent with the geometry normal
//    if (dot(frame.n, dirIn) < 0) {
//        frame.t = -frame.t;
//        frame.b = -frame.b;
//        frame.n = -frame.n;
//    }
//
//    dirOut = toWorld(frame, sampleUniformHemisphere(rndParam));
//    return true;
//}
//
//float bsdf_disney_metal_pdf(in BSDFVertex vertex, in BSDFFrame frame, in BSDFMaterial material, in vec3 dirIn, in vec3 dirOut) {
//    if (dot(vertex.geometricNormal, dirIn) < 0 ||
//    dot(vertex.geometricNormal, dirOut) < 0) {
//        // No light below the surface
//        return 0;
//    }
//
//    // Flip the shading frame if it is inconsistent with the geometry normal
//    if (dot(frame.n, dirIn) < 0) {
//        frame.t = -frame.t;
//        frame.b = -frame.b;
//        frame.n = -frame.n;
//    }
//
////    return 1.0 / TWO_PI;
//    return (ALPHA + 1.0) * pow(max(0, dot(dirOut, frame.n)), ALPHA) / (2.0 * PI);
//}

#endif