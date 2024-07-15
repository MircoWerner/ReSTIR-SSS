#ifndef RESTIRSSS_DISNEY_BSSRDF_GLSL
#define RESTIRSSS_DISNEY_BSSRDF_GLSL

#include "restirsss_defines.glsl"

bool continueBSSRDFPathLightSourceSampling(inout uint rngState, inout ReSTIRSSSSample nextSample, in const vec3 lightNormalUnaligned, in const float lightPDFVAM, out float misWeight, out float lightPDF) {
    const vec3 lightToSurface = nextSample.samplePosition - nextSample.lightPosition;
    const vec3 lightToSurfaceNormalized = normalize(lightToSurface);
    const vec3 lightNormal = dot(lightNormalUnaligned, lightToSurfaceNormalized) >= 0.0 ? lightNormalUnaligned : -lightNormalUnaligned;// align normal in correct direction
    const float cosAtLight = max(0, dot(lightNormal, lightToSurfaceNormalized));
    const float geometryTermSolidAngle = cosAtLight / dot(lightToSurface, lightToSurface);
    lightPDF = lightPDFVAM / geometryTermSolidAngle;
    if (lightPDF <= 0.0) {
        return false;
    }

    // record
    nextSample.lightNormal = lightNormal;

    if (g_restir_candidate_samples_bsdf > 0) {
        // light source sampling is combined with bsdf sampling, calculate MIS weight
        const BSDFFrame frame = coordinateSystem(nextSample.sampleNormal);
        const float otherPDF = max(dot(frame.n, -lightToSurfaceNormalized), 0) / PI;
        misWeight = lightPDF / (g_restir_candidate_samples_light * lightPDF + g_restir_candidate_samples_bsdf * otherPDF);
    } else {
        // light source sampling is the only technique used
        misWeight = 1.0 / float(g_restir_candidate_samples_light);// m_i(X_i) = 1 / M
    }

    return true;
}

bool continueBSSRDFPathLightSourceSampling(inout uint rngState, inout ReSTIRSSSSample nextSample, out float misWeight, out float lightPDF) {
    // sample light candidate
    int lightObjectDescriptor;
    int lightTriangleId;
    vec2 lightBarycentrics;
    vec3 lightPosition;
    vec3 lightNormalUnaligned;
    vec3 lightEmission;
    float lightPDFVAM;
    arealight_sample_vam(nextUInt(rngState, g_num_lights - 1), nextFloat(rngState), nextFloat(rngState), lightObjectDescriptor, lightTriangleId, lightBarycentrics, lightPosition, lightNormalUnaligned, lightEmission, lightPDFVAM);

    const vec3 lightToSurface = nextSample.samplePosition - lightPosition;
    const vec3 lightToSurfaceNormalized = normalize(lightToSurface);
    const vec3 lightNormal = dot(lightNormalUnaligned, lightToSurfaceNormalized) >= 0.0 ? lightNormalUnaligned : -lightNormalUnaligned;// align normal in correct direction
    const float cosAtLight = max(0, dot(lightNormal, lightToSurfaceNormalized));
    const float geometryTermSolidAngle = cosAtLight / dot(lightToSurface, lightToSurface);
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
        const BSDFFrame frame = coordinateSystem(nextSample.sampleNormal);
        const float otherPDF = max(dot(frame.n, -lightToSurfaceNormalized), 0) / PI;
        misWeight = lightPDF / (g_restir_candidate_samples_light * lightPDF + g_restir_candidate_samples_bsdf * otherPDF);
    } else {
        // light source sampling is the only technique used
        misWeight = 1.0 / float(g_restir_candidate_samples_light);// m_i(X_i) = 1 / M
    }

    return true;
}

bool continueBSSRDFPathBSDFSampling(inout uint rngState, inout ReSTIRSSSSample nextSample, out float misWeight, out float lightPDF) {
    const BSDFFrame frame = coordinateSystem(nextSample.sampleNormal);

    // sample x3
    const vec3 rayOutgoing = toWorld(frame, sampleCosHemisphere(vec2(nextFloat(rngState), nextFloat(rngState))));
    lightPDF = max(dot(frame.n, rayOutgoing), 0) / PI;

    // trace ray
    HitPayload payload;
    if (!trace(nextSample.samplePosition, rayOutgoing, 0, 10000.0, payload)) {
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
        const vec3 lightToSurface = nextSample.samplePosition - nextSample.lightPosition;
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

void generateAllCandidatesBSSRDFWithLightSourceSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, in const int objectDescriptorId, inout ReSTIRSSSReservoir reservoir) {
    const uint sssRngState = rngState;

    const uint axis = sss_sampling_axis_index(nextFloat(rngState));
    const uint channel = sss_sampling_scatterDistance_index(nextFloat(rngState));
    const float xiRadius = nextFloat(rngState);
    const float xiAngle = nextFloat(rngState);
    const vec3 sssTangentFrame = pixelInfo.geometricNormal;

    const SSSInfo sssInfo = SSSInfo(pixelInfo.position, objectDescriptorId, pixelInfo.material.scatterDistance, INVALID_UINT_VALUE);

    const BSDFFrame frame = coordinateSystem(sssTangentFrame);
    BSDFFrame projectionFrame;
    sss_sampling_axis(axis, frame, projectionFrame);

    const float sampledScatterDistance = sss_sampling_scatterDistance(channel, sssInfo.scatterDistance);
    const float radiusMax = sss_diffusion_profile_sample(0.999, sampledScatterDistance);
    const float radius = sss_diffusion_profile_sample(xiRadius, sampledScatterDistance);
    const float phi = xiAngle * TWO_PI;

    ReSTIRSSSSample nextSample;

    if (radius > radiusMax) {
        // we generated an invalid sample, record this in the reservoir
        // although a null sample has a target function of 0 and thus a weight of 0, we still have to increase the confidence by 1
        restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        return;
    }

    const vec3 origin = sssInfo.position + radiusMax * projectionFrame.n + cos(phi) * radius * projectionFrame.t + sin(phi) * radius * projectionFrame.b;
    const vec3 direction = -projectionFrame.n;
    const float sphereFraction = sqrt(radiusMax * radiusMax - radius * radius);
    const float tMin = radiusMax - sphereFraction;
    const float tMax = radiusMax + sphereFraction;

    #ifdef RESTIRSSS_LIGHT_PER_CANDIDATE
    int lightObjectDescriptor;
    int lightTriangleId;
    vec2 lightBarycentrics;
    vec3 lightPosition;
    vec3 lightNormalUnaligned;
    vec3 lightEmission;
    float lightPDFVAM;
    arealight_sample_vam(nextUInt(rngState, g_num_lights - 1), nextFloat(rngState), nextFloat(rngState), lightObjectDescriptor, lightTriangleId, lightBarycentrics, lightPosition, lightNormalUnaligned, lightEmission, lightPDFVAM);

    nextSample.lightObjectDescriptor = lightObjectDescriptor;
    nextSample.lightTriangleId = lightTriangleId;
    nextSample.lightBarycentrics = lightBarycentrics;
    nextSample.lightPosition = lightPosition;
    //    nextSample.lightNormal = ...; // undefined, will be set later
    nextSample.lightEmission = lightEmission;
    #endif

    // trace
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, // Ray query
    topLevelAS, // Top-level acceleration structure
    gl_RayFlagsNoOpaqueEXT, // Ray flags, here saying "treat all geometry as opaque"
    0xFF, // 8-bit instance mask, here saying "trace against all instances"
    origin, // Ray origin
    tMin, // Minimum t-value
    direction, // Ray direction
    tMax);// Maximum t-value

    uint numCandidates = 0;
    SSSSample sssSample;
    while (rayQueryProceedEXT(rayQuery)) {
        const int nextObjectDescriptorId = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false);// false = for the candidate intersection, not the commited one
        if (nextObjectDescriptorId != objectDescriptorId) {
            // we hit another object, ignore this intersection
            continue;
        }

        const int nextTriangleID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
        const vec2 nextBarycentrics = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);

        sssSample.objectDescriptorId = nextObjectDescriptorId;
        sssSample.triangleId = nextTriangleID;
        sssSample.barycentrics = nextBarycentrics;
        const HitPayload payload = HitPayload(nextObjectDescriptorId, nextTriangleID, 0, nextBarycentrics, gl_HitKindFrontFacingTriangleEXT);
        intersectionInfo(payload, sssSample.position, sssSample.geometricNormal, sssSample.normal);

        const float bssrdfPDF = sss_sampling_disk_pdf(sssInfo.position, frame, sssSample.position, sssSample.geometricNormal, sssInfo.scatterDistance);

        // record
        nextSample.sampleObjectDescriptor = sssSample.objectDescriptorId;
        nextSample.sampleTriangleId = sssSample.triangleId;
        nextSample.sampleBarycentrics = sssSample.barycentrics;
        nextSample.samplePosition = sssSample.position;
        nextSample.sampleGeometricNormal = sssSample.geometricNormal;
        nextSample.sampleNormal = sssSample.normal;

        nextSample.axis = axis;
        nextSample.channel = channel;
        nextSample.xiRadius = xiRadius;
        nextSample.xiAngle = xiAngle;

        nextSample.intersectionId = numCandidates;

        nextSample.jacobian = bssrdfPDF;

        float misWeight;
        float lightPDF;
        {
            // light source sampling
            #ifdef RESTIRSSS_LIGHT_PER_CANDIDATE
            if (continueBSSRDFPathLightSourceSampling(rngState, nextSample, lightNormalUnaligned, lightPDFVAM, misWeight, lightPDF))
            #else
            if (continueBSSRDFPathLightSourceSampling(rngState, nextSample, misWeight, lightPDF))
            #endif
            {
                const float targetFunction = restirsss_target_function_evaluate(pixelInfo, nextSample);
                //        const float targetFunction = restirsss_target_function_resampleEvaluate(pixelInfo, nextSample, true);
                const float proposalDistribution = bssrdfPDF * lightPDF;// p(X_i)
                const float unbiasedContributionWeight = 1.0 / proposalDistribution;// W_X_i = 1 / p(X_i)

                const float weight = misWeight * targetFunction * unbiasedContributionWeight;// m_i(X_i) * ^p(X_i) * W_X_i

                restirsss_reservoir_update(rngState, reservoir, nextSample, weight, 0.0);// confidence = 1.0 for candidate (for all intersections together, add later)
            }
            numCandidates += 1;
        }
    }

    if (numCandidates > 0) {
        reservoir.confidenceWeight += 1.0;
    } else {
        restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
    }
}

void generateAllCandidatesBSSRDFWithBSDFSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, in const int objectDescriptorId, inout ReSTIRSSSReservoir reservoir) {
    const uint sssRngState = rngState;

    const uint axis = sss_sampling_axis_index(nextFloat(rngState));
    const uint channel = sss_sampling_scatterDistance_index(nextFloat(rngState));
    const float xiRadius = nextFloat(rngState);
    const float xiAngle = nextFloat(rngState);
    const vec3 sssTangentFrame = pixelInfo.geometricNormal;

    const SSSInfo sssInfo = SSSInfo(pixelInfo.position, objectDescriptorId, pixelInfo.material.scatterDistance, INVALID_UINT_VALUE);

    const BSDFFrame frame = coordinateSystem(sssTangentFrame);
    BSDFFrame projectionFrame;
    sss_sampling_axis(axis, frame, projectionFrame);

    const float sampledScatterDistance = sss_sampling_scatterDistance(channel, sssInfo.scatterDistance);
    const float radiusMax = sss_diffusion_profile_sample(0.999, sampledScatterDistance);
    const float radius = sss_diffusion_profile_sample(xiRadius, sampledScatterDistance);
    const float phi = xiAngle * TWO_PI;

    ReSTIRSSSSample nextSample;

    if (radius > radiusMax) {
        // we generated an invalid sample, record this in the reservoir
        // although a null sample has a target function of 0 and thus a weight of 0, we still have to increase the confidence by 1
        restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        return;
    }

    const vec3 origin = sssInfo.position + radiusMax * projectionFrame.n + cos(phi) * radius * projectionFrame.t + sin(phi) * radius * projectionFrame.b;
    const vec3 direction = -projectionFrame.n;
    const float sphereFraction = sqrt(radiusMax * radiusMax - radius * radius);
    const float tMin = radiusMax - sphereFraction;
    const float tMax = radiusMax + sphereFraction;

    // trace
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, // Ray query
    topLevelAS, // Top-level acceleration structure
    gl_RayFlagsNoOpaqueEXT, // Ray flags, here saying "treat all geometry as opaque"
    0xFF, // 8-bit instance mask, here saying "trace against all instances"
    origin, // Ray origin
    tMin, // Minimum t-value
    direction, // Ray direction
    tMax);// Maximum t-value

    uint numCandidates = 0;
    SSSSample sssSample;
    while (rayQueryProceedEXT(rayQuery)) {
        const int nextObjectDescriptorId = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false);// false = for the candidate intersection, not the commited one
        if (nextObjectDescriptorId != objectDescriptorId) {
            // we hit another object, ignore this intersection
            continue;
        }

        const int nextTriangleID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
        const vec2 nextBarycentrics = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);

        sssSample.objectDescriptorId = nextObjectDescriptorId;
        sssSample.triangleId = nextTriangleID;
        sssSample.barycentrics = nextBarycentrics;
        const HitPayload payload = HitPayload(nextObjectDescriptorId, nextTriangleID, 0, nextBarycentrics, gl_HitKindFrontFacingTriangleEXT);
        intersectionInfo(payload, sssSample.position, sssSample.geometricNormal, sssSample.normal);

        const float bssrdfPDF = sss_sampling_disk_pdf(sssInfo.position, frame, sssSample.position, sssSample.geometricNormal, sssInfo.scatterDistance);

        // record
        nextSample.sampleObjectDescriptor = sssSample.objectDescriptorId;
        nextSample.sampleTriangleId = sssSample.triangleId;
        nextSample.sampleBarycentrics = sssSample.barycentrics;
        nextSample.samplePosition = sssSample.position;
        nextSample.sampleGeometricNormal = sssSample.geometricNormal;
        nextSample.sampleNormal = sssSample.normal;

        nextSample.axis = axis;
        nextSample.channel = channel;
        nextSample.xiRadius = xiRadius;
        nextSample.xiAngle = xiAngle;

        nextSample.intersectionId = numCandidates;

        nextSample.jacobian = bssrdfPDF;

        float misWeight;
        float lightPDF;
        {
            // bsdf sampling
            if (continueBSSRDFPathBSDFSampling(rngState, nextSample, misWeight, lightPDF)) {
                const float targetFunction = restirsss_target_function_evaluate(pixelInfo, nextSample);
                const float proposalDistribution = bssrdfPDF * lightPDF;// p(X_i)
                const float unbiasedContributionWeight = 1.0 / proposalDistribution;// W_X_i = 1 / p(X_i)

                const float weight = misWeight * targetFunction * unbiasedContributionWeight;// m_i(X_i) * ^p(X_i) * W_X_i

                restirsss_reservoir_update(rngState, reservoir, nextSample, weight, 0.0);// confidence = 1.0 for candidate (for all intersections together, add later)
            }
            numCandidates += 1;
        }
    }

    if (numCandidates > 0) {
        reservoir.confidenceWeight += 1.0;
    } else {
        restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
    }
}

void generateSingleCandidateBSSRDFWithLightSourceSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, in const int objectDescriptorId, inout ReSTIRSSSReservoir reservoir) {
    const uint sssRngState = rngState;

    // sample surface candidate
    const uint axis = sss_sampling_axis_index(nextFloat(rngState));
    const uint channel = sss_sampling_scatterDistance_index(nextFloat(rngState));
    const float xiRadius = nextFloat(rngState);
    const float xiAngle = nextFloat(rngState);
    const vec3 sssTangentFrame = pixelInfo.geometricNormal;

    const BSDFFrame frame = coordinateSystem(sssTangentFrame);
    BSDFFrame projectionFrame;
    sss_sampling_axis(axis, frame, projectionFrame);

    ReSTIRSSSSample nextSample;

    SSSSample sssSample;
    float bssrdfPDF;
    float bssrdfIntersectionPDF;
    SSSInfo sssInfo = SSSInfo(pixelInfo.position, objectDescriptorId, pixelInfo.material.scatterDistance, INVALID_UINT_VALUE);
    if (!sss_sampling_sample(frame, projectionFrame, sssInfo, channel, xiRadius, xiAngle, rngState, sssSample, bssrdfPDF, bssrdfIntersectionPDF)) {
        // we generated an invalid sample, record this in the reservoir
        // although a null sample has a target function of 0 and thus a weight of 0, we still have to increase the confidence by 1
        restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        return;
    }

    nextSample.sampleObjectDescriptor = sssSample.objectDescriptorId;
    nextSample.sampleTriangleId = sssSample.triangleId;
    nextSample.sampleBarycentrics = sssSample.barycentrics;
    nextSample.samplePosition = sssSample.position;
    nextSample.sampleGeometricNormal = sssSample.geometricNormal;
    nextSample.sampleNormal = sssSample.normal;

    nextSample.axis = axis;
    nextSample.channel = channel;
    nextSample.xiRadius = xiRadius;
    nextSample.xiAngle = xiAngle;

    nextSample.intersectionId = sssSample.intersection;

    nextSample.jacobian = bssrdfPDF;

    float misWeight;
    float lightPDF;
    {
        // light source sampling
        if (continueBSSRDFPathLightSourceSampling(rngState, nextSample, misWeight, lightPDF)) {
            const float targetFunction = restirsss_target_function_evaluate(pixelInfo, nextSample);
            //        const float targetFunction = restirsss_target_function_resampleEvaluate(pixelInfo, nextSample, true);
            const float proposalDistribution = bssrdfPDF * bssrdfIntersectionPDF * lightPDF;// p(X_i)
            const float unbiasedContributionWeight = 1.0 / proposalDistribution;// W_X_i = 1 / p(X_i)

            const float weight = misWeight * targetFunction * unbiasedContributionWeight;// m_i(X_i) * ^p(X_i) * W_X_i

            restirsss_reservoir_update(rngState, reservoir, nextSample, weight, 1.0);// confidence = 1.0
        } else {
            // we generated an invalid sample, record this in the reservoir
            restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        }
    }
}

void generateSingleCandidateBSSRDFWithBSDFSampling(inout uint rngState, in const ReSTIRPixelInfo pixelInfo, in const int objectDescriptorId, inout ReSTIRSSSReservoir reservoir) {
    const uint sssRngState = rngState;

    // sample surface candidate
    const uint axis = sss_sampling_axis_index(nextFloat(rngState));
    const uint channel = sss_sampling_scatterDistance_index(nextFloat(rngState));
    const float xiRadius = nextFloat(rngState);
    const float xiAngle = nextFloat(rngState);
    const vec3 sssTangentFrame = pixelInfo.geometricNormal;

    const BSDFFrame frame = coordinateSystem(sssTangentFrame);
    BSDFFrame projectionFrame;
    sss_sampling_axis(axis, frame, projectionFrame);

    ReSTIRSSSSample nextSample;

    SSSSample sssSample;
    float bssrdfPDF;
    float bssrdfIntersectionPDF;
    SSSInfo sssInfo = SSSInfo(pixelInfo.position, objectDescriptorId, pixelInfo.material.scatterDistance, INVALID_UINT_VALUE);
    if (!sss_sampling_sample(frame, projectionFrame, sssInfo, channel, xiRadius, xiAngle, rngState, sssSample, bssrdfPDF, bssrdfIntersectionPDF)) {
        // we generated an invalid sample, record this in the reservoir
        // although a null sample has a target function of 0 and thus a weight of 0, we still have to increase the confidence by 1
        restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        return;
    }

    nextSample.sampleObjectDescriptor = sssSample.objectDescriptorId;
    nextSample.sampleTriangleId = sssSample.triangleId;
    nextSample.sampleBarycentrics = sssSample.barycentrics;
    nextSample.samplePosition = sssSample.position;
    nextSample.sampleGeometricNormal = sssSample.geometricNormal;
    nextSample.sampleNormal = sssSample.normal;

    nextSample.axis = axis;
    nextSample.channel = channel;
    nextSample.xiRadius = xiRadius;
    nextSample.xiAngle = xiAngle;

    nextSample.intersectionId = sssSample.intersection;

    nextSample.jacobian = bssrdfPDF;

    float misWeight;
    float lightPDF;
    {
        // light source sampling
        if (continueBSSRDFPathBSDFSampling(rngState, nextSample, misWeight, lightPDF)) {
            const float targetFunction = restirsss_target_function_evaluate(pixelInfo, nextSample);
            //        const float targetFunction = restirsss_target_function_resampleEvaluate(pixelInfo, nextSample, true);
            const float proposalDistribution = bssrdfPDF * bssrdfIntersectionPDF * lightPDF;// p(X_i)
            const float unbiasedContributionWeight = 1.0 / proposalDistribution;// W_X_i = 1 / p(X_i)

            const float weight = misWeight * targetFunction * unbiasedContributionWeight;// m_i(X_i) * ^p(X_i) * W_X_i

            restirsss_reservoir_update(rngState, reservoir, nextSample, weight, 1.0);// confidence = 1.0
        } else {
            // we generated an invalid sample, record this in the reservoir
            restirsss_reservoir_update(rngState, reservoir, nextSample, 0, 1.0);// confidence = 1.0
        }
    }
}

#endif