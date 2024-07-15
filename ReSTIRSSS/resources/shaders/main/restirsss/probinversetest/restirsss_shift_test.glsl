#ifndef RESTIRSSS_SHIFT_GLSL
#define RESTIRSSS_SHIFT_GLSL

void restirsss_reconnection_inverse(inout uint rngState, in const ReSTIRPixelInfo pixelInfoPath, inout ReSTIRSSSSample samplePath) {
    if (restirsss_reservoir_isNullSample(samplePath)) {
        return;
    }

    const vec3 y1x2 = samplePath.samplePosition - pixelInfoPath.position;
    const vec3 y1x2Normalized = normalize(y1x2);

    const BSDFVertex vertex = BSDFVertex(pixelInfoPath.geometricNormal);
    BSDFFrame frame = coordinateSystem(pixelInfoPath.normal);

    const float metallic = pixelInfoPath.material.metallic;
    const float diffuseWeight = 1.0 - metallic;
    const float metalWeight = metallic;
    const float sumWeights = diffuseWeight + metalWeight;

    uint newLobe;
    float newXiLobe;
    {
        const float xi1 = nextFloat(rngState);
        const float xi2 = nextFloat(rngState);

        // select lobe
        const float diffusePDF = bsdf_disney_diffuse_pdf(vertex, frame, pixelInfoPath.material, pixelInfoPath.v, y1x2Normalized);
        const float metalPDF = bsdf_disney_metal_pdf(vertex, frame, pixelInfoPath.material, pixelInfoPath.v, y1x2Normalized);

        const float weightedSumPDF = (diffuseWeight * diffusePDF + metalWeight * metalPDF) / sumWeights;

        if (weightedSumPDF <= 0) {
            // no axis is valid
            restirsss_reservoir_initNullSample(samplePath);
            return;
        } else {
            const float weightedDiffusePDF = diffuseWeight * diffusePDF / sumWeights;
            const float weightedMetalPDF = metalWeight * metalPDF / sumWeights;

            if (xi1 < weightedDiffusePDF / weightedSumPDF) {
                // choose diffuse lobe
                newLobe = 0;
                newXiLobe = xi2 * diffuseWeight / sumWeights;
            } else {
                // choose metal lobe
                newLobe = 1;
                newXiLobe = diffuseWeight / sumWeights + xi2 * metalWeight / sumWeights;
            }
        }
    }

    float newXi1;
    float newXi2;
    float newX1PDF;

    // inverse sample
    if (newLobe == 0) {
        if (dot(frame.n, pixelInfoPath.v) < 0) {
            frame.t = -frame.t;
            frame.b = -frame.b;
            frame.n = -frame.n;
        }

        const vec3 omegaLocal = toLocal(frame, y1x2Normalized);

        const vec2 reverseRndParam = reverseSampleCosHemisphere(omegaLocal);

        newXi1 = reverseRndParam.x;
        newXi2 = reverseRndParam.y;
    } else {
        if (dot(frame.n, pixelInfoPath.v) < 0) {
            frame.t = -frame.t;
            frame.b = -frame.b;
            frame.n = -frame.n;
        }

        const float aspect = sqrt(1.0 - 0.9 * pixelInfoPath.material.anisotropic);
        const float alphaMin = 0.0001;
        const float alphaX = max(alphaMin, pixelInfoPath.material.roughness * pixelInfoPath.material.roughness / aspect);
        const float alphaY = max(alphaMin, pixelInfoPath.material.roughness * pixelInfoPath.material.roughness * aspect);

        const vec3 halfVector = normalize(pixelInfoPath.v + y1x2Normalized);
        const vec3 halfVectorLocal = toLocal(frame, halfVector);
        const vec3 dirInLocal = toLocal(frame, pixelInfoPath.v);

        const vec2 reverseRndParam = reverseSampleVisibleNormals(dirInLocal, alphaX, alphaY, halfVectorLocal);

        newXi1 = reverseRndParam.x;
        newXi2 = reverseRndParam.y;
    }

    newX1PDF = bsdf_disney_pdf(vertex, frame, pixelInfoPath.material, pixelInfoPath.v, y1x2Normalized);
    if (newX1PDF <= 0) {
        restirsss_reservoir_initNullSample(samplePath);
        return;
    }

    samplePath.xiLobe = newXiLobe;
    samplePath.xi1 = newXi1;
    samplePath.xi2 = newXi2;

    samplePath.jacobian = newX1PDF;
}

void restirsss_shift(inout uint rngState, in const uint shift, in const ReSTIRPixelInfo pixelInfoOffsetPath, in const ReSTIRPixelInfo pixelInfoBasePath, in const ReSTIRSSSSample sampleBasePath, out ReSTIRSSSSample sampleOffsetPath, out float jacobian, out bool visibilityCheck) {
    visibilityCheck = true;

    if (restirsss_reservoir_isNullSample(sampleBasePath) || shift == RESTIRSSS_SHIFT_HYBRID || sampleBasePath.sampleObjectDescriptor == RESTIR_SAMPLE_INVALID) {
        restirsss_reservoir_initNullSample(sampleOffsetPath);
        jacobian = 0;
        return;
    }

    if (shift == RESTIRSSS_SHIFT_RECONNECTION) {
        sampleOffsetPath = sampleBasePath;

        const vec3 x1x2 = sampleBasePath.samplePosition - pixelInfoBasePath.position;
        const vec3 x1x2Normalized = normalize(x1x2);
        const vec3 y1x2 = sampleBasePath.samplePosition - pixelInfoOffsetPath.position;
        const vec3 y1x2Normalized = normalize(y1x2);

        const float cosX = abs(dot(sampleBasePath.sampleGeometricNormal, -x1x2Normalized));
        const float cosY = abs(dot(sampleBasePath.sampleGeometricNormal, -y1x2Normalized));
        const float lX = dot(x1x2, x1x2);
        const float lY = dot(y1x2, y1x2);

        jacobian = cosY * lX / (cosX * lY);
    } else if (shift == RESTIRSSS_SHIFT_DELAYED_RECONNECTION || shift == RESTIRSSS_SHIFT_SEQUENTIAL_DELAYED_RECONNECTION) {
//        restirsss_reconnection_inverse(rngState, pixelInfoBasePath, sampleBasePath); // test: it is possible to re-calculate lobe and random numbers for each application of random replay without changing the result / the Jacobian
//        if (restirsss_reservoir_isNullSample(sampleBasePath)) {
//            restirsss_reservoir_initNullSample(sampleOffsetPath);
//            jacobian = 0;
//            return;
//        }

        const uint lobe = bsdf_disney_lobe_index(pixelInfoOffsetPath.material, sampleBasePath.xiLobe); // use the random number for the lobe selection at the offset path (with the offset path material)
        const float xi1 = sampleBasePath.xi1;
        const float xi2 = sampleBasePath.xi2;

        const BSDFVertex vertex = BSDFVertex(pixelInfoOffsetPath.geometricNormal);
        const BSDFFrame frame = coordinateSystem(pixelInfoOffsetPath.normal);

        vec3 rayOutgoing;
        if (!bsdf_disney_sample(vertex, frame, pixelInfoOffsetPath.material, pixelInfoOffsetPath.v, rayOutgoing, lobe, vec2(xi1, xi2))) {
            restirsss_reservoir_initNullSample(sampleOffsetPath);
            jacobian = 0;
            return;
        }

        SSSSample sssSample;
        {
            HitPayload payload;
            vec3 positionWorld;// interpolated position
            vec3 geometricNormalWorld;
            vec3 normalWorld;

            // trace ray
            if (!trace(pixelInfoOffsetPath.position, rayOutgoing, 0, 10000.0, payload)) {
                // we generated an invalid sample
                restirsss_reservoir_initNullSample(sampleOffsetPath);
                jacobian = 0;
                return;
            }

            // query intersection info
            intersectionInfo(payload, positionWorld, geometricNormalWorld, normalWorld);

            sssSample.objectDescriptorId = payload.objectDescriptorId;
            sssSample.triangleId = payload.triangleId;
            sssSample.barycentrics = payload.barycentrics;
            sssSample.position = positionWorld;
            sssSample.geometricNormal = geometricNormalWorld;
            sssSample.normal = normalWorld;
        }
        const float x1PDF = bsdf_disney_pdf(vertex, frame, pixelInfoOffsetPath.material, pixelInfoOffsetPath.v, rayOutgoing);
        if (x1PDF <= 0) {
            restirsss_reservoir_initNullSample(sampleOffsetPath);
            jacobian = 0;
            return;
        }

        sampleOffsetPath = sampleBasePath;

        sampleOffsetPath.sampleObjectDescriptor = sssSample.objectDescriptorId;
        sampleOffsetPath.sampleTriangleId = sssSample.triangleId;
        sampleOffsetPath.sampleBarycentrics = sssSample.barycentrics;
        sampleOffsetPath.samplePosition = sssSample.position;
        sampleOffsetPath.sampleGeometricNormal = sssSample.geometricNormal;
        sampleOffsetPath.sampleNormal = sssSample.normal;

        sampleOffsetPath.jacobian = x1PDF;

        const float randomReplayJacobian = sampleBasePath.jacobian / x1PDF;
        float reconnectionJacobian = 1.0;
        {
            // our light sampling (and pdf) is converted from area measure to solid angle measure
            // thus our jacobian is not 1
            vec3 x2x3 = sampleBasePath.lightPosition - sampleBasePath.samplePosition;
            vec3 x2x3Normalized = normalize(x2x3);
            vec3 y2x3 = sampleBasePath.lightPosition - sampleOffsetPath.samplePosition;
            vec3 y2x3Normalized = normalize(y2x3);

            float cosX = abs(dot(sampleBasePath.lightNormal, -x2x3Normalized));
            float cosY = abs(dot(sampleBasePath.lightNormal, -y2x3Normalized));
            float lX = dot(x2x3, x2x3);
            float lY = dot(y2x3, y2x3);

            // actually we need ratio of pdfs, but pdfs are always equal currently for area light sampling, so ratio is just 1
            reconnectionJacobian = cosY * lX / (cosX * lY);
        }

        jacobian = randomReplayJacobian * reconnectionJacobian;
    } else if (shift == RESTIRSSS_SHIFT_SEQUENTIAL_RECONNECTION) {
        sampleOffsetPath = sampleBasePath;

        const vec3 x1x2 = sampleBasePath.samplePosition - pixelInfoBasePath.position;
        const vec3 x1x2Normalized = normalize(x1x2);
        const vec3 y1x2 = sampleBasePath.samplePosition - pixelInfoOffsetPath.position;
        const vec3 y1x2Normalized = normalize(y1x2);

        const float cosX = abs(dot(sampleBasePath.sampleGeometricNormal, -x1x2Normalized));
        const float cosY = abs(dot(sampleBasePath.sampleGeometricNormal, -y1x2Normalized));
        const float lX = dot(x1x2, x1x2);
        const float lY = dot(y1x2, y1x2);

        jacobian = cosY * lX / (cosX * lY);

        restirsss_reconnection_inverse(rngState, pixelInfoOffsetPath, sampleOffsetPath);
        if (restirsss_reservoir_isNullSample(sampleOffsetPath)) {
            jacobian = 0;
            return;
        }
    }
}

#endif