#ifndef RESTIRSSS_SHIFT_GLSL
#define RESTIRSSS_SHIFT_GLSL

#include "restirsss_shift_hybrid_criterion.glsl"

void restirsss_reconnection_intersectionId(in const ReSTIRPixelInfo pixelInfoPath, inout ReSTIRSSSSample samplePath) {
    if (restirsss_reservoir_isNullSample(samplePath)) {
        return;
    }

    const vec3 sssTangentFrame = pixelInfoPath.geometricNormal;

    const BSDFFrame frame = coordinateSystem(sssTangentFrame);
    BSDFFrame projectionFrame;
    sss_sampling_axis(samplePath.axis, frame, projectionFrame);

    const SSSInfo sssInfo = SSSInfo(pixelInfoPath.position, int(samplePath.sampleObjectDescriptor), pixelInfoPath.material.scatterDistance, INVALID_UINT_VALUE);
    const SSSSample sssSample = SSSSample(int(samplePath.sampleObjectDescriptor), int(samplePath.sampleTriangleId), samplePath.sampleBarycentrics, samplePath.samplePosition, samplePath.sampleGeometricNormal, samplePath.sampleNormal, INVALID_UINT_VALUE);

    uint newIntersectionId = INVALID_UINT_VALUE;
    if (!sss_sampling_sampleIntersectionId(frame, projectionFrame, sssInfo, samplePath.channel, sssSample, newIntersectionId)) {
        #ifndef RESTIRSSS_RECONNECTION_INTERSECTIONID_DELAYED
        restirsss_reservoir_initNullSample(samplePath);
        #else
        samplePath.intersectionId = INVALID_UINT_VALUE;
        #endif
        return;
    }

    samplePath.intersectionId = newIntersectionId;
}

void restirsss_reconnection_inverse(inout uint rngState, in const ReSTIRPixelInfo pixelInfoPath, inout ReSTIRSSSSample samplePath) {
    if (restirsss_reservoir_isNullSample(samplePath)) {
        return;
    }

    const vec3 sssTangentFrame = pixelInfoPath.geometricNormal;
    const BSDFFrame frame = coordinateSystem(sssTangentFrame);

    const SSSInfo sssInfo = SSSInfo(pixelInfoPath.position, int(samplePath.sampleObjectDescriptor), pixelInfoPath.material.scatterDistance, INVALID_UINT_VALUE);
    const SSSSample sssSample = SSSSample(int(samplePath.sampleObjectDescriptor), int(samplePath.sampleTriangleId), samplePath.sampleBarycentrics, samplePath.samplePosition, samplePath.sampleGeometricNormal, samplePath.sampleNormal, INVALID_UINT_VALUE);

    uint newAxis;
    uint newChannel;
    {
        const float xi1 = nextFloat(rngState);
        const float xi2 = nextFloat(rngState);

        // select axis and channel
        {
            vec3 axis0PDF;// contains channel pdfs for axis 0
            vec3 axis1PDF;// contains channel pdfs for axis 1
            vec3 axis2PDF;// contains channel pdfs for axis 2
            sss_sampling_disk_pdf_axis_channel(sssInfo.position, frame, sssSample.position, sssSample.geometricNormal, pixelInfoPath.material.scatterDistance, axis0PDF, axis1PDF, axis2PDF);

            const float weightedAxis0Channel0PDF = SSS_SAMPLING_DISK_AXIS_0_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_0_WEIGHT * axis0PDF[0];
            const float weightedAxis0Channel1PDF = SSS_SAMPLING_DISK_AXIS_0_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_1_WEIGHT * axis0PDF[1];
            const float weightedAxis0Channel2PDF = SSS_SAMPLING_DISK_AXIS_0_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_2_WEIGHT * axis0PDF[2];
            const float weightedAxis1Channel0PDF = SSS_SAMPLING_DISK_AXIS_1_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_0_WEIGHT * axis1PDF[0];
            const float weightedAxis1Channel1PDF = SSS_SAMPLING_DISK_AXIS_1_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_1_WEIGHT * axis1PDF[1];
            const float weightedAxis1Channel2PDF = SSS_SAMPLING_DISK_AXIS_1_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_2_WEIGHT * axis1PDF[2];
            const float weightedAxis2Channel0PDF = SSS_SAMPLING_DISK_AXIS_2_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_0_WEIGHT * axis2PDF[0];
            const float weightedAxis2Channel1PDF = SSS_SAMPLING_DISK_AXIS_2_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_1_WEIGHT * axis2PDF[1];
            const float weightedAxis2Channel2PDF = SSS_SAMPLING_DISK_AXIS_2_WEIGHT * SSS_SAMPLING_DISK_CHANNEL_2_WEIGHT * axis2PDF[2];

            const float sumWeights = weightedAxis0Channel0PDF + weightedAxis0Channel1PDF + weightedAxis0Channel2PDF
            + weightedAxis1Channel0PDF + weightedAxis1Channel1PDF + weightedAxis1Channel2PDF
            + weightedAxis2Channel0PDF + weightedAxis2Channel1PDF + weightedAxis2Channel2PDF;

            if (sumWeights <= 0) {
                // no axis and channel is valid
                restirsss_reservoir_initNullSample(samplePath);
                return;
            }

            if (xi1 < weightedAxis0Channel0PDF / sumWeights) {
                // choose axis 0 and channel 0
                newAxis = 0;
                newChannel = 0;
            } else if (xi1 < (weightedAxis0Channel0PDF + weightedAxis0Channel1PDF) / sumWeights) {
                // choose axis 0 and channel 1
                newAxis = 0;
                newChannel = 1;
            } else if (xi1 < (weightedAxis0Channel0PDF + weightedAxis0Channel1PDF + weightedAxis0Channel2PDF) / sumWeights) {
                // choose axis 0 and channel 2
                newAxis = 0;
                newChannel = 2;
            } else if (xi1 < (weightedAxis0Channel0PDF + weightedAxis0Channel1PDF + weightedAxis0Channel2PDF + weightedAxis1Channel0PDF) / sumWeights) {
                // choose axis 1 and channel 0
                newAxis = 1;
                newChannel = 0;
            } else if (xi1 < (weightedAxis0Channel0PDF + weightedAxis0Channel1PDF + weightedAxis0Channel2PDF + weightedAxis1Channel0PDF + weightedAxis1Channel1PDF) / sumWeights) {
                // choose axis 1 and channel 1
                newAxis = 1;
                newChannel = 1;
            } else if (xi1 < (weightedAxis0Channel0PDF + weightedAxis0Channel1PDF + weightedAxis0Channel2PDF + weightedAxis1Channel0PDF + weightedAxis1Channel1PDF + weightedAxis1Channel2PDF) / sumWeights) {
                // choose axis 1 and channel 2
                newAxis = 1;
                newChannel = 2;
            } else if (xi1 < (weightedAxis0Channel0PDF + weightedAxis0Channel1PDF + weightedAxis0Channel2PDF + weightedAxis1Channel0PDF + weightedAxis1Channel1PDF + weightedAxis1Channel2PDF + weightedAxis2Channel0PDF) / sumWeights) {
                // choose axis 2 and channel 0
                newAxis = 2;
                newChannel = 0;
            } else if (xi1 < (weightedAxis0Channel0PDF + weightedAxis0Channel1PDF + weightedAxis0Channel2PDF + weightedAxis1Channel0PDF + weightedAxis1Channel1PDF + weightedAxis1Channel2PDF + weightedAxis2Channel0PDF + weightedAxis2Channel1PDF) / sumWeights) {
                // choose axis 2 and channel 1
                newAxis = 2;
                newChannel = 1;
            } else {
                // choose axis 2 and channel 2
                newAxis = 2;
                newChannel = 2;
            }
        }
    }

    float newXiRadius;
    float newXiAngle;
    float newBssrdfPDF;

    // inverse sample
    BSDFFrame projectionFrame;
    sss_sampling_axis(newAxis, frame, projectionFrame);
    if (!sss_sampling_sampleInvert(frame, projectionFrame, sssInfo, newChannel, newXiRadius, newXiAngle, sssSample, newBssrdfPDF)) {
        restirsss_reservoir_initNullSample(samplePath);
        return;
    }
    if (newBssrdfPDF <= 0 || isnan(newBssrdfPDF) || isinf(newBssrdfPDF)) {
        restirsss_reservoir_initNullSample(samplePath);
        return;
    }

    samplePath.jacobian = newBssrdfPDF;
    samplePath.axis = newAxis;
    samplePath.channel = newChannel;
    samplePath.xiRadius = newXiRadius;
    samplePath.xiAngle = newXiAngle;

    samplePath.intersectionId = INVALID_UINT_VALUE;

    #ifndef RESTIRSSS_RECONNECTION_INTERSECTIONID_DELAYED
    restirsss_reconnection_intersectionId(pixelInfoPath, samplePath);
    #endif
}

void restirsss_shift_sss(inout uint rngState, in const uint shift, in const ReSTIRPixelInfo pixelInfoOffsetPath, in const ReSTIRPixelInfo pixelInfoBasePath, in const ReSTIRSSSSample sampleBasePath, out ReSTIRSSSSample sampleOffsetPath, out float jacobian, out bool visibilityCheck) {
    visibilityCheck = false;

    if (shift == RESTIRSSS_SHIFT_RECONNECTION) {
        sampleOffsetPath = sampleBasePath;
        jacobian = 1.0;
        visibilityCheck = false;// visibility check not required, reconnection through translucent object [y1,x2] is always possible

        #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
        g_debug_valueR[LOCAL_LINEAR_INDEX]++; // shift successfull
        #endif
    } else if (shift == RESTIRSSS_SHIFT_DELAYED_RECONNECTION || shift == RESTIRSSS_SHIFT_SEQUENTIAL_DELAYED_RECONNECTION) {
        SSSSample sssSample;
        float bssrdfPDF;
        float bssrdfIntersectionPDF;

        const vec3 sssTangentFrame = pixelInfoOffsetPath.geometricNormal;
        const BSDFFrame frame = coordinateSystem(sssTangentFrame);
        BSDFFrame projectionFrame;
        sss_sampling_axis(sampleBasePath.axis, frame, projectionFrame);
        const SSSInfo sssInfo = SSSInfo(pixelInfoOffsetPath.position, int(sampleBasePath.sampleObjectDescriptor), pixelInfoOffsetPath.material.scatterDistance, sampleBasePath.intersectionId);

        if (!sss_sampling_sample(frame, projectionFrame, sssInfo, sampleBasePath.channel, sampleBasePath.xiRadius, sampleBasePath.xiAngle, rngState, sssSample, bssrdfPDF, bssrdfIntersectionPDF)) {
            // we generated an invalid sample
            restirsss_reservoir_initNullSample(sampleOffsetPath);
            jacobian = 0;
            visibilityCheck = false;
            #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
            g_debug_valueG[LOCAL_LINEAR_INDEX]++; // shift failed
            #endif
            return;
        }

        sampleOffsetPath = sampleBasePath;
        sampleOffsetPath.sampleObjectDescriptor = sssSample.objectDescriptorId;
        sampleOffsetPath.sampleTriangleId = sssSample.triangleId;
        sampleOffsetPath.sampleBarycentrics = sssSample.barycentrics;
        sampleOffsetPath.samplePosition = sssSample.position;
        sampleOffsetPath.sampleGeometricNormal = sssSample.geometricNormal;
        sampleOffsetPath.sampleNormal = sssSample.normal;

        sampleOffsetPath.intersectionId = sssSample.intersection;

        sampleOffsetPath.jacobian = bssrdfPDF;

        const float randomReplayJacobian = sampleBasePath.jacobian / bssrdfPDF;
        float reconnectionJacobian = 1.0;
        {
            // our light sampling (and pdf) is converted from area measure to solid angle measure
            // thus our jacobian is not 1
            const vec3 x2x3 = sampleBasePath.lightPosition - sampleBasePath.samplePosition;
            const vec3 x2x3Normalized = normalize(x2x3);
            const vec3 y2x3 = sampleBasePath.lightPosition - sampleOffsetPath.samplePosition;
            const vec3 y2x3Normalized = normalize(y2x3);

            const float cosX = abs(dot(sampleBasePath.lightNormal, -x2x3Normalized));
            const float cosY = abs(dot(sampleBasePath.lightNormal, -y2x3Normalized));
            const float lX = dot(x2x3, x2x3);
            const float lY = dot(y2x3, y2x3);

            // actually we need ratio of pdfs, but pdfs are always equal currently for area light sampling, so ratio is just 1
            reconnectionJacobian = cosY * lX / (cosX * lY);
        }

        jacobian = randomReplayJacobian * reconnectionJacobian;

        visibilityCheck = true;// visibility check required, [y2,x3] might be occluded

        #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
        g_debug_valueR[LOCAL_LINEAR_INDEX]++; // shift successfull
        #endif
    } else if (shift == RESTIRSSS_SHIFT_HYBRID) {
        // determine rcVertex of base path
        if (restirsss_shift_connectable(pixelInfoBasePath.position, pixelInfoBasePath.geometricNormal, sampleBasePath.samplePosition, sampleBasePath.sampleGeometricNormal, pixelInfoBasePath.material.scatterDistance)) {
            // if rcVertex == 1

            // test reconnection criterion for offset path
            if (!restirsss_shift_connectable(pixelInfoOffsetPath.position, pixelInfoOffsetPath.geometricNormal, sampleBasePath.samplePosition, sampleBasePath.sampleGeometricNormal, pixelInfoOffsetPath.material.scatterDistance)) {
                // fail
                restirsss_reservoir_initNullSample(sampleOffsetPath);
                jacobian = 0;
                visibilityCheck = false;
                #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
                g_debug_valueG[LOCAL_LINEAR_INDEX]++; // shift failed
                #endif
                return;
            }

            // reconnect
            sampleOffsetPath = sampleBasePath;
            jacobian = 1.0;
            visibilityCheck = false;// visibility check not required, reconnection through translucent object [y1,x2] is always possible
        } else {
            // else (if rcVertex == 2)

            // random replay
            float randomReplayJacobian = 1.0;
            {
                SSSSample sssSample;
                float bssrdfPDF;
                float bssrdfIntersectionPDF;

                const vec3 sssTangentFrame = pixelInfoOffsetPath.geometricNormal;
                const BSDFFrame frame = coordinateSystem(sssTangentFrame);
                BSDFFrame projectionFrame;
                sss_sampling_axis(sampleBasePath.axis, frame, projectionFrame);
                const SSSInfo sssInfo = SSSInfo(pixelInfoOffsetPath.position, int(sampleBasePath.sampleObjectDescriptor), pixelInfoOffsetPath.material.scatterDistance, sampleBasePath.intersectionId);

                if (!sss_sampling_sample(frame, projectionFrame, sssInfo, sampleBasePath.channel, sampleBasePath.xiRadius, sampleBasePath.xiAngle, rngState, sssSample, bssrdfPDF, bssrdfIntersectionPDF)) {
                    // we generated an invalid sample
                    restirsss_reservoir_initNullSample(sampleOffsetPath);
                    jacobian = 0;
                    visibilityCheck = false;
                    #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
                    g_debug_valueG[LOCAL_LINEAR_INDEX]++; // shift failed
                    #endif
                    return;
                }

                sampleOffsetPath = sampleBasePath;

                sampleOffsetPath.sampleObjectDescriptor = sssSample.objectDescriptorId;
                sampleOffsetPath.sampleTriangleId = sssSample.triangleId;
                sampleOffsetPath.sampleBarycentrics = sssSample.barycentrics;
                sampleOffsetPath.samplePosition = sssSample.position;
                sampleOffsetPath.sampleGeometricNormal = sssSample.geometricNormal;
                sampleOffsetPath.sampleNormal = sssSample.normal;

                sampleOffsetPath.intersectionId = sssSample.intersection;

                sampleOffsetPath.jacobian = bssrdfPDF;

                randomReplayJacobian = sampleBasePath.jacobian / bssrdfPDF;
            }

            // assert that random replayed offset path does not have rcVertex=1 (maybe fail)
            if (restirsss_shift_connectable(pixelInfoOffsetPath.position, pixelInfoOffsetPath.geometricNormal, sampleOffsetPath.samplePosition, sampleOffsetPath.sampleGeometricNormal, pixelInfoOffsetPath.material.scatterDistance)) {
                // fail
                restirsss_reservoir_initNullSample(sampleOffsetPath);
                jacobian = 0;
                visibilityCheck = false;
                #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
                g_debug_valueG[LOCAL_LINEAR_INDEX]++; // shift failed
                #endif
                return;
            }

            // test reconnection criterion for rcVertex=2 (maybe fail)
            // this is always true by our definition of connectability (last vertex always connectable)

            // reconnect
            float reconnectionJacobian = 1.0;
            {
                // our light sampling (and pdf) is converted from area measure to solid angle measure
                // thus our jacobian is not 1
                const vec3 x2x3 = sampleBasePath.lightPosition - sampleBasePath.samplePosition;
                const vec3 x2x3Normalized = normalize(x2x3);
                const vec3 y2x3 = sampleBasePath.lightPosition - sampleOffsetPath.samplePosition;
                const vec3 y2x3Normalized = normalize(y2x3);

                const float cosX = abs(dot(sampleBasePath.lightNormal, -x2x3Normalized));
                const  float cosY = abs(dot(sampleBasePath.lightNormal, -y2x3Normalized));
                const float lX = dot(x2x3, x2x3);
                const float lY = dot(y2x3, y2x3);

                // actually we need ratio of pdfs, but pdfs are always equal currently for area light sampling, so ratio is just 1
                reconnectionJacobian = cosY * lX / (cosX * lY);
            }

            jacobian = randomReplayJacobian * reconnectionJacobian;

            visibilityCheck = true;// visibility check required, [y2,x3] might be occluded
        }

        #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
        g_debug_valueR[LOCAL_LINEAR_INDEX]++; // shift successfull
        #endif
    } else if (shift == RESTIRSSS_SHIFT_SEQUENTIAL_RECONNECTION) {
        sampleOffsetPath = sampleBasePath;
        jacobian = 1.0;
        visibilityCheck = false;// visibility check not required, reconnection through translucent object [y1,x2] is always possible

        restirsss_reconnection_inverse(rngState, pixelInfoOffsetPath, sampleOffsetPath);
        if (restirsss_reservoir_isNullSample(sampleOffsetPath)) {
            jacobian = 0;
            #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
            g_debug_valueG[LOCAL_LINEAR_INDEX]++; // shift failed
            #endif
            return;
        }

        #ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
        g_debug_valueR[LOCAL_LINEAR_INDEX]++; // shift successfull
        #endif
    } else {
        restirsss_reservoir_initNullSample(sampleOffsetPath);
        jacobian = 0;
        return;
    }
}

void restirsss_shift_nosss(inout uint rngState, in const uint shift, in const ReSTIRPixelInfo pixelInfoOffsetPath, in const ReSTIRPixelInfo pixelInfoBasePath, in const ReSTIRSSSSample sampleBasePath, out ReSTIRSSSSample sampleOffsetPath, out float jacobian, out bool visibilityCheck) {
    // always do reconnection shift
    sampleOffsetPath = sampleBasePath;
    // this is a surface reflection, we reconnect from primary hit to light source
    float reconnectionJacobian = 1.0;
    {
        // our light sampling (and pdf) is converted from area measure to solid angle measure
        // thus our jacobian is not 1
        vec3 x1x2 = sampleBasePath.lightPosition - pixelInfoBasePath.position;
        vec3 x1x2Normalized = normalize(x1x2);
        vec3 y1x2 = sampleBasePath.lightPosition - pixelInfoOffsetPath.position;
        vec3 y1x2Normalized = normalize(y1x2);

        float cosX = abs(dot(sampleBasePath.lightNormal, -x1x2Normalized));
        float cosY = abs(dot(sampleBasePath.lightNormal, -y1x2Normalized));
        float lX = dot(x1x2, x1x2);
        float lY = dot(y1x2, y1x2);

        // actually we need ratio of pdfs, but pdfs are always equal currently for area light sampling, so ratio is just 1
        reconnectionJacobian = cosY * lX / (cosX * lY);
    }
    jacobian = reconnectionJacobian;

    visibilityCheck = true;// visibility check required, [y1,x2] might be occluded
}

void restirsss_shift(inout uint rngState, in const uint shift, in const ReSTIRPixelInfo pixelInfoOffsetPath, in const ReSTIRPixelInfo pixelInfoBasePath, in const ReSTIRSSSSample sampleBasePath, out ReSTIRSSSSample sampleOffsetPath, out float jacobian, out bool visibilityCheck) {
    if (restirsss_reservoir_isNullSample(sampleBasePath)) {
        restirsss_reservoir_initNullSample(sampleOffsetPath);
        jacobian = 0;
        visibilityCheck = false;
        return;
    }

    const bool basePathSSS = restirsss_pixel_info_isSSS(pixelInfoBasePath);
    const bool offsetPathSSS = restirsss_pixel_info_isSSS(pixelInfoOffsetPath);

    if (basePathSSS != offsetPathSSS) {
        // one path is subsurface, the other is not
        restirsss_reservoir_initNullSample(sampleOffsetPath);
        jacobian = 0;
        visibilityCheck = false;
        return;
    }

    if (basePathSSS) {
        restirsss_shift_sss(rngState, shift, pixelInfoOffsetPath, pixelInfoBasePath, sampleBasePath, sampleOffsetPath, jacobian, visibilityCheck);
    } else {
        restirsss_shift_nosss(rngState, shift, pixelInfoOffsetPath, pixelInfoBasePath, sampleBasePath, sampleOffsetPath, jacobian, visibilityCheck);
    }
}

#endif