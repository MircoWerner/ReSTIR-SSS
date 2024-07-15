#ifndef RESTIRSSS_DISNEY_BSDF_GLSL
#define RESTIRSSS_DISNEY_BSDF_GLS

bool continueBSDFPathLightSourceSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, inout ReSTIRSSSSample nextSample, out float misWeight, out float lightPDF) {
    // sample light candidate
    int lightObjectDescriptor;
    int lightTriangleId;
    vec2 lightBarycentrics;
    vec3 lightPosition;
    vec3 lightNormalUnaligned;
    vec3 lightEmission;
    float lightPDFVAM;
    arealight_sample_vam(nextUInt(rngState, g_num_lights - 1), nextFloat(rngState), nextFloat(rngState), lightObjectDescriptor, lightTriangleId, lightBarycentrics, lightPosition, lightNormalUnaligned, lightEmission, lightPDFVAM);

    vec3 lightToSurface = pixelInfo.position - lightPosition;
    vec3 lightToSurfaceNormalized = normalize(lightToSurface);
    vec3 lightNormal = dot(lightNormalUnaligned, lightToSurfaceNormalized) >= 0.0 ? lightNormalUnaligned : -lightNormalUnaligned;// align normal in correct direction
    float cosAtLight = max(0, dot(lightNormal, lightToSurfaceNormalized));
    float geometryTermSolidAngle = cosAtLight / dot(lightToSurface, lightToSurface);
    lightPDF = lightPDFVAM / geometryTermSolidAngle;
    if (lightPDF <= 0.0) {
        return false;
    }

    // record
    nextSample.lightObjectDescriptor = lightObjectDescriptor;
    nextSample.lightTriangleId = lightTriangleId;
    nextSample.lightBarycentrics = lightBarycentrics;
    nextSample.lightPosition = lightPosition;
    nextSample.lightNormal = lightNormal;
    nextSample.lightEmission = lightEmission;

    if (g_restir_candidate_samples_bsdf > 0) {
        // light source sampling is combined with bsdf sampling, calculate MIS weight
        const BSDFFrame frame = coordinateSystem(pixelInfo.normal);
        const float otherPDF = max(dot(frame.n, -lightToSurfaceNormalized), 0) / PI;
        misWeight = lightPDF / (g_restir_candidate_samples_light * lightPDF + g_restir_candidate_samples_bsdf * otherPDF);
    } else {
        // light source sampling is the only technique used
        misWeight = 1.0 / float(g_restir_candidate_samples_light);// m_i(X_i) = 1 / M
    }

    return true;
}

bool continueBSDFPathBSDFSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, inout ReSTIRSSSSample nextSample, out float misWeight, out float lightPDF) {
    const BSDFFrame frame = coordinateSystem(pixelInfo.normal);

    // sample x3
    const vec3 rayOutgoing = toWorld(frame, sampleCosHemisphere(vec2(nextFloat(rngState), nextFloat(rngState))));
    lightPDF = max(dot(frame.n, rayOutgoing), 0) / PI;

    // trace ray
    HitPayload payload;
    if (!trace(pixelInfo.position, rayOutgoing, 0, 10000.0, payload)) {
        return false;
    }

    // query intersection info
    vec3 p0World;
    vec3 p1World;
    vec3 p2World;
    vec3 positionWorld;
    vec3 geometricNormalWorld;
    vec3 emission;
    if (!intersectionInfoLightSource(payload, p0World, p1World, p2World, positionWorld, geometricNormalWorld, emission)) {
        // not a light source
        return false;
    }

    // record
    nextSample.lightObjectDescriptor = payload.objectDescriptorId;
    nextSample.lightTriangleId = payload.triangleId;
    nextSample.lightBarycentrics = payload.barycentrics;
    nextSample.lightPosition = positionWorld;
    nextSample.lightNormal = geometricNormalWorld;
    nextSample.lightEmission = emission;

    if (g_restir_candidate_samples_light > 0) {
        // bsdf sampling is combined with light source sampling, calculate MIS weight
        const float otherPDFVAM = arealight_pdf_vam(g_num_lights, p0World, p1World, p2World);
        const vec3 lightToSurface = pixelInfo.position - nextSample.lightPosition;
        const float cosAtLight = max(0, dot(nextSample.lightNormal, -rayOutgoing));
        const float geometryTermSolidAngle = cosAtLight / dot(lightToSurface, lightToSurface);
        const float otherPDF = otherPDFVAM / geometryTermSolidAngle;
        misWeight = lightPDF / (g_restir_candidate_samples_bsdf * lightPDF + g_restir_candidate_samples_light * otherPDF);
    } else {
        // bsdf sampling is the only technique used
        misWeight = 1.0 / float(g_restir_candidate_samples_bsdf);// m_i(X_i) = 1 / M
    }

    return true;
}

void generateSingleCandidateBSDFWithLightSourceSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, in const int objectDescriptorId, inout ReSTIRSSSReservoir reservoir) {
    ReSTIRSSSSample nextSample;
    restirsss_reservoir_initNullSample(nextSample);

    nextSample.jacobian = 1.0;

    float misWeight;
    float lightPDF;
    {
        // light source sampling
        if (continueBSDFPathLightSourceSampling(rngState, pixelInfo, nextSample, misWeight, lightPDF)) {
            const float targetFunction = restirsss_target_function_evaluate(pixelInfo, nextSample);
            const float proposalDistribution = lightPDF;// p(X_i)
            const float unbiasedContributionWeight = 1.0 / proposalDistribution;// W_X_i = 1 / p(X_i)

            const float weight = misWeight * targetFunction * unbiasedContributionWeight;// m_i(X_i) * ^p(X_i) * W_X_i

            restirsss_reservoir_update(rngState, reservoir, nextSample, weight, 1.0);// confidence = 1.0
        } else {
            // we generated an invalid sample, record this in the reservoir
            restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        }
    }
}

void generateSingleCandidateBSDFWithBSDFSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, in const int objectDescriptorId, inout ReSTIRSSSReservoir reservoir) {
    ReSTIRSSSSample nextSample;
    restirsss_reservoir_initNullSample(nextSample);

    nextSample.jacobian = 1.0;

    float misWeight;
    float lightPDF;
    {
        // light source sampling
        if (continueBSDFPathBSDFSampling(rngState, pixelInfo, nextSample, misWeight, lightPDF)) {
            const float targetFunction = restirsss_target_function_evaluate(pixelInfo, nextSample);
            const float proposalDistribution = lightPDF;// p(X_i)
            const float unbiasedContributionWeight = 1.0 / proposalDistribution;// W_X_i = 1 / p(X_i)

            const float weight = misWeight * targetFunction * unbiasedContributionWeight;// m_i(X_i) * ^p(X_i) * W_X_i

            restirsss_reservoir_update(rngState, reservoir, nextSample, weight, 1.0);// confidence = 1.0
        } else {
            // we generated an invalid sample, record this in the reservoir
            restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        }
    }
}

#ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
void generateCandidateL3Path(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, in const int objectDescriptorId, out ReSTIRSSSSample nextSample, out float weight) {
    const BSDFVertex vertex = BSDFVertex(pixelInfo.geometricNormal);
    const BSDFFrame frame = coordinateSystem(pixelInfo.normal);

    // sample x2
    const float xiLobe = nextFloat(rngState);
    const float xi1X2 = nextFloat(rngState);
    const float xi2X2 = nextFloat(rngState);
    vec3 rayOutgoing;
    if (!bsdf_disney_sample(vertex, frame, pixelInfo.material, pixelInfo.v, rayOutgoing, bsdf_disney_lobe_index(pixelInfo.material, xiLobe), vec2(xi1X2, xi2X2))) {
        // we generated an invalid sample, record this in the reservoir
        // although a null sample has a target function of 0 and thus a weight of 0, we still have to increase the confidence by 1
        weight = 0;
        return;
    }

    SSSSample sssSample;
    {
        HitPayload payload;
        vec3 positionWorld;// interpolated position
        vec3 geometricNormalWorld;
        vec3 normalWorld;

        // trace ray
        if (!trace(pixelInfo.position, rayOutgoing, 0, 10000.0, payload)) {
            // we generated an invalid sample, record this in the reservoir
            // although a null sample has a target function of 0 and thus a weight of 0, we still have to increase the confidence by 1
            weight = 0;
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
    const float x1PDF = bsdf_disney_pdf(vertex, frame, pixelInfo.material, pixelInfo.v, rayOutgoing);
    if (x1PDF <= 0.0) {
        restirsss_reservoir_initNullSample(nextSample);
        weight = 0;
        return;
    }

    // sample light candidate
    int lightObjectDescriptor;
    int lightTriangleId;
    vec2 lightBarycentrics;
    vec3 lightPosition;
    vec3 lightNormal;
    vec3 lightEmission;
    float lightPDF;
    arealight_sample_vam(nextUInt(rngState, g_num_lights - 1), nextFloat(rngState), nextFloat(rngState), sssSample.position, lightObjectDescriptor, lightTriangleId, lightBarycentrics, lightPosition, lightNormal, lightEmission, lightPDF);
    const vec3 lightToSurface = sssSample.position - lightPosition;
    const vec3 lightToSurfaceNormalized = normalize(lightToSurface);
    const float cosAtLight = max(0, dot(lightNormal, lightToSurfaceNormalized));
    const float geometryTermSolidAngle = cosAtLight / dot(lightToSurface, lightToSurface);
    lightPDF /= geometryTermSolidAngle;
    if (lightPDF <= 0.0) {
        // this should never occur, however...
        // we generated an invalid sample, record this in the reservoir
        // although a null sample has a target function of 0 and thus a weight of 0, we still have to increase the confidence by 1
        restirsss_reservoir_initNullSample(nextSample);
        weight = 0;
        return;
    }

    nextSample.sampleObjectDescriptor = sssSample.objectDescriptorId;
    nextSample.sampleTriangleId = sssSample.triangleId;
    nextSample.sampleBarycentrics = sssSample.barycentrics;
    nextSample.samplePosition = sssSample.position;
    nextSample.sampleGeometricNormal = sssSample.geometricNormal;
    nextSample.sampleNormal = sssSample.normal;

    nextSample.lightObjectDescriptor = lightObjectDescriptor;
    nextSample.lightTriangleId = lightTriangleId;
    nextSample.lightBarycentrics = lightBarycentrics;
    nextSample.lightPosition = lightPosition;
    nextSample.lightNormal = lightNormal;
    nextSample.lightEmission = lightEmission;

    nextSample.xiLobe = xiLobe;
    nextSample.xi1 = xi1X2;
    nextSample.xi2 = xi2X2;

    nextSample.jacobian = x1PDF;

    const float targetFunction = restirsss_target_function_evaluate(pixelInfo, nextSample);
    const float proposalDistribution = x1PDF * lightPDF;// p(X_i)
    const float misWeight = 1.0 / float(g_restir_candidate_samples_light);// m_i(X_i) = 1 / M
    const float unbiasedContributionWeight = 1.0 / proposalDistribution;// W_X_i = 1 / p(X_i)

    weight = misWeight * targetFunction * unbiasedContributionWeight;// m_i(X_i) * ^p(X_i) * W_X_i = 1 / M * ^p(X_i) / p(X_i)
}
#endif

#endif