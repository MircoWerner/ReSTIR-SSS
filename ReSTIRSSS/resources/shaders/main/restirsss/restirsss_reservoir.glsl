#ifndef RESTIRSSS_RESERVOIR_GLSL
#define RESTIRSSS_RESERVOIR_GLSL

#include "../../utility/constants.glsl"
#include "../../trace/trace.glsl"
#include "restirsss_defines.glsl"

#define RESTIR_SAMPLE_INVALID 0xFFFFFFFF

struct ReSTIRSSSSample {
// vertex x2
    int sampleObjectDescriptor;
    int sampleTriangleId;
    vec2 sampleBarycentrics;
    vec3 samplePosition;
    vec3 sampleGeometricNormal;
    vec3 sampleNormal;

// vertex x3
    int lightObjectDescriptor;
    int lightTriangleId;
    vec2 lightBarycentrics;
    vec3 lightPosition;
    vec3 lightNormal;
    vec3 lightEmission;

// axis, channel and random numbers
#ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
    float xiLobe;
    float _xiUnused;
    float xi1;
    float xi2;
#else
    uint axis;
    uint channel;
    float xiRadius;
    float xiAngle;
#endif

// intersection index
    uint intersectionId;

// cached part of Jacobian
    float jacobian;
};

struct ReSTIRSSSReservoir {
    ReSTIRSSSSample pickedSample;// Y
    float unbiasedContributionWeight;// W_Y

    float sumWeights;// w_sum
    float confidenceWeight;// c or M == numCandidates
};

void restirsss_reservoir_initNullSample(out ReSTIRSSSSample rSample) {
    rSample.sampleObjectDescriptor = RESTIR_SAMPLE_INVALID;
    rSample.sampleTriangleId = 0;
    rSample.sampleBarycentrics = vec2(0);
    rSample.samplePosition = vec3(0);
    rSample.sampleGeometricNormal = vec3(0);
    rSample.sampleNormal = vec3(0);

    rSample.lightObjectDescriptor = RESTIR_SAMPLE_INVALID;
    rSample.lightTriangleId = 0;
    rSample.lightPosition = vec3(0);
    rSample.lightNormal = vec3(0);
    rSample.lightEmission = vec3(0);

    #ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
    rSample.xiLobe = 0;
    rSample._xiUnused = 0;
    rSample.xi1 = 0;
    rSample.xi2 = 0;
    #else
    rSample.axis = 0;
    rSample.channel = 0;
    rSample.xiRadius = 0;
    rSample.xiAngle = 0;
    #endif

    rSample.intersectionId = INVALID_UINT_VALUE;

    rSample.jacobian = 1.0;
}

void restirsss_reservoir_initEmpty(out ReSTIRSSSReservoir reservoir) {
    restirsss_reservoir_initNullSample(reservoir.pickedSample);
    reservoir.unbiasedContributionWeight = 0;
    reservoir.sumWeights = 0;
    reservoir.confidenceWeight = 0;
}

bool restirsss_reservoir_isNullSample(in const ReSTIRSSSSample rSample) {
    return rSample.sampleObjectDescriptor == RESTIR_SAMPLE_INVALID && rSample.lightObjectDescriptor == RESTIR_SAMPLE_INVALID;
}

bool restirsss_reservoir_isNullSample(in const ReSTIRSSSReservoir reservoir) {
    return reservoir.pickedSample.sampleObjectDescriptor == RESTIR_SAMPLE_INVALID && reservoir.pickedSample.lightObjectDescriptor == RESTIR_SAMPLE_INVALID;
}

void restirsss_reservoir_calculateUnbiasedContributionWeight(inout ReSTIRSSSReservoir reservoir, in const float targetFunction) {
    if (restirsss_reservoir_isNullSample(reservoir)) {
        reservoir.unbiasedContributionWeight = 0;
        return;
    }
    reservoir.unbiasedContributionWeight = targetFunction == 0.0 ? 0 : reservoir.sumWeights / targetFunction;
}

void restirsss_reservoir_update(inout uint rngState, inout ReSTIRSSSReservoir reservoir, in const ReSTIRSSSSample nextSample, in const float weight, in const float confidence) {
    reservoir.sumWeights += weight;
    reservoir.confidenceWeight += confidence;

    if (weight > 0 && (reservoir.sumWeights == 0 || nextFloat(rngState) <= weight / reservoir.sumWeights)) {
        reservoir.pickedSample = nextSample;
    }
}

void restirsss_reservoir_capConfidence(inout ReSTIRSSSReservoir reservoir, in const float multiplier) {
    reservoir.confidenceWeight = min(reservoir.confidenceWeight, 20 * multiplier);
}

void restirsss_reservoir_merge(inout uint rngState, inout ReSTIRSSSReservoir reservoirCombined, in const ReSTIRSSSSample rSample, in const float targetFunction, in const float misWeight, in const float unbiasedContributionWeight, in const float confidenceWeight) {
    restirsss_reservoir_update(rngState, reservoirCombined, rSample, misWeight * targetFunction * unbiasedContributionWeight, confidenceWeight);
}

void restirsss_reservoir_merge(inout uint rngState, inout ReSTIRSSSReservoir reservoirCombined, in const ReSTIRSSSSample rSample, in const float weight, in const float confidenceWeight) {
    restirsss_reservoir_update(rngState, reservoirCombined, rSample, weight, confidenceWeight);
}

struct ReSTIRSSSReservoirCompressed {
// vertex x2
    int sampleObjectDescriptor;
    int sampleTriangleId;
    float sampleBarycentricsY;
    float sampleBarycentricsZ;

// vertex x3
    int lightObjectDescriptor;
    int lightTriangleId;
    float lightBarycentricsY;
    float lightBarycentricsZ;

// axis, channel and random numbers
#ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
    float xiLobe;
    float _xiUnused;
    float xi1;
    float xi2;
#else
    uint axis;
    uint channel;
    float xiRadius;
    float xiAngle;
#endif

// intersection index
    uint intersectionId;

// cached part of Jacobian
    float jacobian;


    float unbiasedContributionWeight;// W_Y

    float confidenceWeight;// c or M == numCandidates
};

ReSTIRSSSReservoirCompressed restirsss_reservoir_compress(in const ReSTIRSSSReservoir reservoir) {
    return ReSTIRSSSReservoirCompressed(
    reservoir.pickedSample.sampleObjectDescriptor,
    reservoir.pickedSample.sampleTriangleId,
    reservoir.pickedSample.sampleBarycentrics.x,
    reservoir.pickedSample.sampleBarycentrics.y,
    reservoir.pickedSample.lightObjectDescriptor,
    reservoir.pickedSample.lightTriangleId,
    reservoir.pickedSample.lightBarycentrics.x,
    reservoir.pickedSample.lightBarycentrics.y,
    #ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
    reservoir.pickedSample.xiLobe,
    reservoir.pickedSample._xiUnused,
    reservoir.pickedSample.xi1,
    reservoir.pickedSample.xi2,
    #else
    reservoir.pickedSample.axis,
    reservoir.pickedSample.channel,
    reservoir.pickedSample.xiRadius,
    reservoir.pickedSample.xiAngle,
    #endif
    reservoir.pickedSample.intersectionId,
    reservoir.pickedSample.jacobian,
    reservoir.unbiasedContributionWeight,
    reservoir.confidenceWeight
    );
}

void restirsss_reservoir_compress(in const ReSTIRSSSReservoir reservoir, out vec4 data0, out vec4 data1, out vec4 data2, out vec4 data3) {
    data0[0] = reservoir.pickedSample.sampleObjectDescriptor;
    data0[1] = reservoir.pickedSample.sampleTriangleId;
    data0[2] = reservoir.pickedSample.sampleBarycentrics.x;
    data0[3] = reservoir.pickedSample.sampleBarycentrics.y;
    data1[0] = reservoir.pickedSample.lightObjectDescriptor;
    data1[1] = reservoir.pickedSample.lightTriangleId;
    data1[2] = reservoir.pickedSample.lightBarycentrics.x;
    data1[3] = reservoir.pickedSample.lightBarycentrics.y;
    #ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
    data2[0] = reservoir.pickedSample.xiLobe;
    data2[1] = reservoir.pickedSample._xiUnused;
    data2[2] = reservoir.pickedSample.xi1;
    data2[3] = reservoir.pickedSample.xi2;
    #else
    data2[0] = reservoir.pickedSample.axis;
    data2[1] = reservoir.pickedSample.channel;
    data2[2] = reservoir.pickedSample.xiRadius;
    data2[3] = reservoir.pickedSample.xiAngle;
    #endif
    data3[0] = reservoir.pickedSample.intersectionId;
    data3[1] = reservoir.pickedSample.jacobian;
    data3[2] = reservoir.unbiasedContributionWeight;
    data3[3] = reservoir.confidenceWeight;
}

ReSTIRSSSReservoir restirsss_reservoir_decompress(in const ReSTIRSSSReservoirCompressed reservoirCompressed, in const vec3 x1Position) {
    ReSTIRSSSReservoir reservoir = ReSTIRSSSReservoir(
    ReSTIRSSSSample(
    reservoirCompressed.sampleObjectDescriptor,
    reservoirCompressed.sampleTriangleId,
    vec2(reservoirCompressed.sampleBarycentricsY, reservoirCompressed.sampleBarycentricsZ),
    vec3(0),
    vec3(0),
    vec3(0),
    reservoirCompressed.lightObjectDescriptor,
    reservoirCompressed.lightTriangleId,
    vec2(reservoirCompressed.lightBarycentricsY, reservoirCompressed.lightBarycentricsZ),
    vec3(0),
    vec3(0),
    vec3(0),
    #ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
    reservoirCompressed.xiLobe,
    reservoirCompressed._xiUnused,
    reservoirCompressed.xi1,
    reservoirCompressed.xi2,
    #else
    reservoirCompressed.axis,
    reservoirCompressed.channel,
    reservoirCompressed.xiRadius,
    reservoirCompressed.xiAngle,
    #endif
    reservoirCompressed.intersectionId,
    reservoirCompressed.jacobian
    ),
    reservoirCompressed.unbiasedContributionWeight,
    0,
    reservoirCompressed.confidenceWeight
    );

    if (restirsss_reservoir_isNullSample(reservoir)) {
        return reservoir;
    }

    vec3 surfacePosition = x1Position;
    if (reservoir.pickedSample.sampleObjectDescriptor != RESTIR_SAMPLE_INVALID) {
        intersectionInfo(
        HitPayload(reservoir.pickedSample.sampleObjectDescriptor, reservoir.pickedSample.sampleTriangleId, 0, reservoir.pickedSample.sampleBarycentrics, gl_HitKindFrontFacingTriangleEXT),
        reservoir.pickedSample.samplePosition,
        reservoir.pickedSample.sampleGeometricNormal,
        reservoir.pickedSample.sampleNormal
        );
        surfacePosition = reservoir.pickedSample.samplePosition;
    }

    intersectionInfoLightSource(
    HitPayload(reservoir.pickedSample.lightObjectDescriptor, reservoir.pickedSample.lightTriangleId, 0, reservoir.pickedSample.lightBarycentrics, gl_HitKindFrontFacingTriangleEXT),
    reservoir.pickedSample.lightPosition,
    reservoir.pickedSample.lightNormal,
    reservoir.pickedSample.lightEmission
    );
    reservoir.pickedSample.lightNormal = dot(reservoir.pickedSample.lightNormal, normalize(surfacePosition - reservoir.pickedSample.lightPosition)) >= 0.0 ? reservoir.pickedSample.lightNormal : -reservoir.pickedSample.lightNormal;// align normal in correct direction

    return reservoir;
}

ReSTIRSSSReservoir restirsss_reservoir_decompress(in const vec4 data0, in const vec4 data1, in const vec4 data2, in const vec4 data3, in const vec3 x1Position) {
    ReSTIRSSSReservoir reservoir = ReSTIRSSSReservoir(
    ReSTIRSSSSample(
    int(data0[0]),
    int(data0[1]),
    vec2(data0[2], data0[3]),
    vec3(0),
    vec3(0),
    vec3(0),
    int(data1[0]),
    int(data1[1]),
    vec2(data1[2], data1[3]),
    vec3(0),
    vec3(0),
    vec3(0),
    #ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
    data2[0],
    data2[1],
    #else
    uint(data2[0]),
    uint(data2[1]),
    #endif
    data2[2],
    data2[3],
    uint(data3[0]),
    data3[1]
    ),
    data3[2],
    0,
    data3[3]
    );

    if (restirsss_reservoir_isNullSample(reservoir)) {
        return reservoir;
    }

    vec3 surfacePosition = x1Position;
    if (reservoir.pickedSample.sampleObjectDescriptor != RESTIR_SAMPLE_INVALID) {
        intersectionInfo(
        HitPayload(reservoir.pickedSample.sampleObjectDescriptor, reservoir.pickedSample.sampleTriangleId, 0, reservoir.pickedSample.sampleBarycentrics, gl_HitKindFrontFacingTriangleEXT),
        reservoir.pickedSample.samplePosition,
        reservoir.pickedSample.sampleGeometricNormal,
        reservoir.pickedSample.sampleNormal
        );
        surfacePosition = reservoir.pickedSample.samplePosition;
    }

    intersectionInfoLightSource(
    HitPayload(reservoir.pickedSample.lightObjectDescriptor, reservoir.pickedSample.lightTriangleId, 0, reservoir.pickedSample.lightBarycentrics, gl_HitKindFrontFacingTriangleEXT),
    reservoir.pickedSample.lightPosition,
    reservoir.pickedSample.lightNormal,
    reservoir.pickedSample.lightEmission
    );
    reservoir.pickedSample.lightNormal = dot(reservoir.pickedSample.lightNormal, normalize(surfacePosition - reservoir.pickedSample.lightPosition)) >= 0.0 ? reservoir.pickedSample.lightNormal : -reservoir.pickedSample.lightNormal;// align normal in correct direction

    return reservoir;
}

#endif