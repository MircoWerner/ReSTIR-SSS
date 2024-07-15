#ifndef RESTIRSSS_MIS_GLSL
#define RESTIRSSS_MIS_GLSL

#include "restirsss_defines.glsl"
#include "../../bxdf/disney.glsl"
#include "../../raycommon.glsl"
#include "../../trace/trace_rayquery.glsl"

#ifdef RESTIRSSS_PROBINVERSETEST_L3PATH
#include "probinversetest/restirsss_shift_test.glsl"
#else
#include "restirsss_shift.glsl"
#endif

#undef RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE
#undef RESTIRSSS_RESAMPLE_TARGET_FUNCTION_RESAMPLEEVALUATE
#undef RESTIRSSS_RESAMPLE_RESERVOIR_MERGE
#undef RESTIRSSS_RESAMPLE_RESERVOIR_CALCULATEUNBIASEDCONTRIBUTIONWEIGHT

#define RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(pixelInfo, evalSample) restirsss_target_function_evaluate(pixelInfo, evalSample)
#define RESTIRSSS_RESAMPLE_TARGET_FUNCTION_RESAMPLEEVALUATE(pixelInfo, evalSample, visibilityCheck) restirsss_target_function_resampleEvaluate(pixelInfo, evalSample, visibilityCheck)
#define RESTIRSSS_RESAMPLE_RESERVOIR_MERGE(rngState, reservoirCombined, rSample, weight, confidenceWeight) restirsss_reservoir_merge(rngState, reservoirCombined, rSample, weight, confidenceWeight)
#define RESTIRSSS_RESAMPLE_RESERVOIR_CALCULATEUNBIASEDCONTRIBUTIONWEIGHT(reservoir, targetFunction) restirsss_reservoir_calculateUnbiasedContributionWeight(reservoir, targetFunction)

// === GENERALIZED BALANCE HEURISTIC (2 reservoirs) ===
void restirsss_mis_generalized_balance_heuristic_two(inout uint rngState, in const uint shift,
in const ReSTIRSSSReservoir canonicalReservoir, in const ReSTIRPixelInfo canonicalPixelInfo,
in const ReSTIRSSSReservoir neighborReservoir, in const ReSTIRPixelInfo neighborPixelInfo,
inout ReSTIRSSSReservoir combinedReservoir) {
    // neighborReservoir contains sample in domain \omega_i
    ReSTIRSSSSample sampleNeighborInCanonicalDomain;// sample in domain \omega
    float jacobianNeighborToCanonicalDomain;// jacobian for shift from \omega_i -> \omega
    bool visibilityCheckNeighborToCanonicalDomain;// whether visibility check is required for shift from \omega_i -> \omega
    restirsss_shift(rngState, shift, canonicalPixelInfo, neighborPixelInfo, neighborReservoir.pickedSample, sampleNeighborInCanonicalDomain, jacobianNeighborToCanonicalDomain, visibilityCheckNeighborToCanonicalDomain);// shift from \omega_i -> \omega

    ReSTIRSSSSample sampleCanonicalInNeighborDomain;// sample in domain \omega_i
    float jacobianCanonicalToNeighborDomain;// jacobian for shift from \omega -> \omega_i
    bool visibilityCheckCanonicalToNeighborDomain;// whether visibility check is required for shift from \omega -> \omega_i
    restirsss_shift(rngState, shift, neighborPixelInfo, canonicalPixelInfo, canonicalReservoir.pickedSample, sampleCanonicalInNeighborDomain, jacobianCanonicalToNeighborDomain, visibilityCheckCanonicalToNeighborDomain);// shift from \omega -> \omega_i

    const float neighborTargetFunctionAtCanonical = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_RESAMPLEEVALUATE(canonicalPixelInfo, sampleNeighborInCanonicalDomain, visibilityCheckNeighborToCanonicalDomain);
    const float canonicalTargetFunctionAtNeighbor = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_RESAMPLEEVALUATE(neighborPixelInfo, sampleCanonicalInNeighborDomain, visibilityCheckCanonicalToNeighborDomain) * jacobianCanonicalToNeighborDomain;// target function from domain \omega to \omega_i by multiplying with jacobian
    const float neighborTargetFunctionAtNeighbor = jacobianNeighborToCanonicalDomain == 0 ? 0 : RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(neighborPixelInfo, neighborReservoir.pickedSample) / jacobianNeighborToCanonicalDomain;// target function from domain \omega_i to \omega by dividing by jacobian
    const float canonicalTargetFunctionAtCanonical = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(canonicalPixelInfo, canonicalReservoir.pickedSample);

    // neighbor resampling MIS
    const float neighborMISAtNeighborDenominator = neighborReservoir.confidenceWeight * neighborTargetFunctionAtNeighbor + canonicalReservoir.confidenceWeight * neighborTargetFunctionAtCanonical;
    float neighborMISAtNeighbor = neighborMISAtNeighborDenominator == 0 ? 0 : neighborReservoir.confidenceWeight * neighborTargetFunctionAtNeighbor / neighborMISAtNeighborDenominator;
    if (isnan(neighborMISAtNeighbor) || isinf(neighborMISAtNeighbor)) neighborMISAtNeighbor = 0;

    // canonical resampling MIS
    const float canonicalMISAtCanonicalDenominator = canonicalReservoir.confidenceWeight * canonicalTargetFunctionAtCanonical + neighborReservoir.confidenceWeight * canonicalTargetFunctionAtNeighbor;
    float canonicalMISAtCanonical = canonicalMISAtCanonicalDenominator == 0 ? 0 : canonicalReservoir.confidenceWeight * canonicalTargetFunctionAtCanonical / canonicalMISAtCanonicalDenominator;
    if (isnan(canonicalMISAtCanonical) || isinf(canonicalMISAtCanonical)) canonicalMISAtCanonical = 0;
    // float canonicalMISAtNeighbor = neighborReservoir.confidenceWeight * canonicalTargetFunctionAtNeighbor / (neighborReservoir.confidenceWeight * canonicalTargetFunctionAtNeighbor + canonicalReservoir.confidenceWeight * canonicalTargetFunctionAtCanonical);
    // canonicalMISAtNeighbor = 1 - canonicalMISAtCanonical

    // merge neighbor
    float neighborWeight = neighborMISAtNeighbor * neighborTargetFunctionAtCanonical * neighborReservoir.unbiasedContributionWeight * jacobianNeighborToCanonicalDomain;// unbiased contribution weight from domain \omega_i to \omega by multiplying with jacobian
    if (isnan(neighborWeight) || isinf(neighborWeight)) neighborWeight = 0;
    RESTIRSSS_RESAMPLE_RESERVOIR_MERGE(rngState, combinedReservoir, sampleNeighborInCanonicalDomain, neighborWeight, neighborReservoir.confidenceWeight);

    #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
    if (neighborTargetFunctionAtCanonical > canonicalTargetFunctionAtCanonical) {
        g_debug_valueB[LOCAL_LINEAR_INDEX]++;// shifted sample is better
    } else {
        g_debug_valueA[LOCAL_LINEAR_INDEX]++;// shifted sample is worse
    }
    #endif

    // merge canonical
    float canonicalWeight = canonicalMISAtCanonical * canonicalTargetFunctionAtCanonical * canonicalReservoir.unbiasedContributionWeight;
    if (isnan(canonicalWeight) || isinf(canonicalWeight)) canonicalWeight = 0;
    RESTIRSSS_RESAMPLE_RESERVOIR_MERGE(rngState, combinedReservoir, canonicalReservoir.pickedSample, canonicalWeight, canonicalReservoir.confidenceWeight);

    // finalize
    const float targetFunction = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(canonicalPixelInfo, combinedReservoir.pickedSample);// evaluate contribution of picked sample y at current pixel
    RESTIRSSS_RESAMPLE_RESERVOIR_CALCULATEUNBIASEDCONTRIBUTIONWEIGHT(combinedReservoir, targetFunction);
}


// === STREAMING PAIRWISE MIS ===
void restirsss_mis_pariwise_stream_neigbor(inout uint rngState, in const uint shift,
in const ReSTIRSSSReservoir canonicalReservoir, in const ReSTIRPixelInfo canonicalPixelInfo,
in const ReSTIRSSSReservoir neighborReservoir, in const ReSTIRPixelInfo neighborPixelInfo,
inout ReSTIRSSSReservoir combinedReservoir,
in const uint numNeighbors, inout float canonicalMIS) {
    // neighborReservoir contains sample in domain \omega_i
    ReSTIRSSSSample sampleNeighborInCanonicalDomain;// sample in domain \omega
    float jacobianNeighborToCanonicalDomain;// jacobian for shift from \omega_i -> \omega
    bool visibilityCheckNeighborToCanonicalDomain;// whether visibility check is required for shift from \omega_i -> \omega
    restirsss_shift(rngState, shift, canonicalPixelInfo, neighborPixelInfo, neighborReservoir.pickedSample, sampleNeighborInCanonicalDomain, jacobianNeighborToCanonicalDomain, visibilityCheckNeighborToCanonicalDomain);// shift from \omega_i -> \omega

    ReSTIRSSSSample sampleCanonicalInNeighborDomain;// sample in domain \omega_i
    float jacobianCanonicalToNeighborDomain;// jacobian for shift from \omega -> \omega_i
    bool visibilityCheckCanonicalToNeighborDomain;// whether visibility check is required for shift from \omega -> \omega_i
    restirsss_shift(rngState, shift, neighborPixelInfo, canonicalPixelInfo, canonicalReservoir.pickedSample, sampleCanonicalInNeighborDomain, jacobianCanonicalToNeighborDomain, visibilityCheckCanonicalToNeighborDomain);// shift from \omega -> \omega_i

    const float neighborTargetFunctionAtCanonical = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_RESAMPLEEVALUATE(canonicalPixelInfo, sampleNeighborInCanonicalDomain, visibilityCheckNeighborToCanonicalDomain);
    const float canonicalTargetFunctionAtNeighbor = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_RESAMPLEEVALUATE(neighborPixelInfo, sampleCanonicalInNeighborDomain, visibilityCheckCanonicalToNeighborDomain) * jacobianCanonicalToNeighborDomain;// target function from domain \omega to \omega_i by multiplying with jacobian
    const float neighborTargetFunctionAtNeighbor = jacobianNeighborToCanonicalDomain == 0 ? 0 : RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(neighborPixelInfo, neighborReservoir.pickedSample) / jacobianNeighborToCanonicalDomain;// target function from domain \omega_i to \omega by dividing by jacobian
    const float canonicalTargetFunctionAtCanonical = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(canonicalPixelInfo, canonicalReservoir.pickedSample);

    const float neighborMISAtNeighborDenominator = neighborReservoir.confidenceWeight * neighborTargetFunctionAtNeighbor + canonicalReservoir.confidenceWeight * neighborTargetFunctionAtCanonical / float(numNeighbors);
    float neighborMISAtNeighbor = neighborMISAtNeighborDenominator == 0 ? 0 : neighborReservoir.confidenceWeight * neighborTargetFunctionAtNeighbor / neighborMISAtNeighborDenominator;
    if (isnan(neighborMISAtNeighbor) || isinf(neighborMISAtNeighbor)) neighborMISAtNeighbor = 0;
    const float canonicalMISAtNeighborDenominator = neighborReservoir.confidenceWeight * canonicalTargetFunctionAtNeighbor + canonicalReservoir.confidenceWeight * canonicalTargetFunctionAtCanonical / float(numNeighbors);
    float canonicalMISAtNeighbor = canonicalMISAtNeighborDenominator == 0 ? 0 : neighborReservoir.confidenceWeight * canonicalTargetFunctionAtNeighbor / canonicalMISAtNeighborDenominator;
    if (isnan(canonicalMISAtNeighbor) || isinf(canonicalMISAtNeighbor)) canonicalMISAtNeighbor = 0;

    canonicalMIS += (1.0 - canonicalMISAtNeighbor);

    #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
    if (neighborTargetFunctionAtCanonical > canonicalTargetFunctionAtCanonical) {
        g_debug_valueB[LOCAL_LINEAR_INDEX]++;// shifted sample is better
    } else {
        g_debug_valueA[LOCAL_LINEAR_INDEX]++;// shifted sample is worse
    }
    #endif

    float neighborWeight = neighborMISAtNeighbor * neighborTargetFunctionAtCanonical * neighborReservoir.unbiasedContributionWeight * jacobianNeighborToCanonicalDomain;// unbiased contribution weight from domain \omega_i to \omega by multiplying with jacobian
    if (isnan(neighborWeight) || isinf(neighborWeight)) neighborWeight = 0;
    RESTIRSSS_RESAMPLE_RESERVOIR_MERGE(rngState, combinedReservoir, sampleNeighborInCanonicalDomain, neighborWeight, neighborReservoir.confidenceWeight);
}

void restirsss_mis_pariwise_stream_canonical(inout uint rngState,
in const ReSTIRSSSReservoir canonicalReservoir, in const ReSTIRPixelInfo canonicalPixelInfo,
inout ReSTIRSSSReservoir combinedReservoir,
in const float canonicalMIS) {
    const float canonicalTargetFunctionAtCanonical = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(canonicalPixelInfo, canonicalReservoir.pickedSample);

    float canonicalWeight = canonicalMIS * canonicalTargetFunctionAtCanonical * canonicalReservoir.unbiasedContributionWeight;
    if (isnan(canonicalWeight) || isinf(canonicalWeight)) canonicalWeight = 0;
    RESTIRSSS_RESAMPLE_RESERVOIR_MERGE(rngState, combinedReservoir, canonicalReservoir.pickedSample, canonicalWeight, canonicalReservoir.confidenceWeight);
}

void restirsss_mis_pariwise_stream_finalize(
in const ReSTIRPixelInfo canonicalPixelInfo,
inout ReSTIRSSSReservoir combinedReservoir,
in const uint validNeighbors) {
    const float targetFunction = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(canonicalPixelInfo, combinedReservoir.pickedSample);// evaluate contribution of picked sample y at current pixel

    //    RESTIRSSS_RESAMPLE_RESERVOIR_CALCULATEUNBIASEDCONTRIBUTIONWEIGHT(combinedReservoir, targetFunction * (max(1.0, validNeighbors))); // non-defensive pairwise MIS
    RESTIRSSS_RESAMPLE_RESERVOIR_CALCULATEUNBIASEDCONTRIBUTIONWEIGHT(combinedReservoir, targetFunction * (validNeighbors + 1.0));// defensive pairwise MIS
}

void restirsss_mis_pariwise_stream_finalize_non_defensive(
in const ReSTIRPixelInfo canonicalPixelInfo,
inout ReSTIRSSSReservoir combinedReservoir,
in const uint validNeighbors) {
    const float targetFunction = RESTIRSSS_RESAMPLE_TARGET_FUNCTION_EVALUATE(canonicalPixelInfo, combinedReservoir.pickedSample);// evaluate contribution of picked sample y at current pixel

    RESTIRSSS_RESAMPLE_RESERVOIR_CALCULATEUNBIASEDCONTRIBUTIONWEIGHT(combinedReservoir, targetFunction * (max(1.0, validNeighbors)));// non-defensive pairwise MIS
    //    RESTIRSSS_RESAMPLE_RESERVOIR_CALCULATEUNBIASEDCONTRIBUTIONWEIGHT(combinedReservoir, targetFunction * (validNeighbors + 1.0));// defensive pairwise MIS
}

#endif