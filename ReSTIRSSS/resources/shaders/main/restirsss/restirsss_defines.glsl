#ifndef RESTIRSSS_DEFINES_GLSL
#define RESTIRSSS_DEFINES_GLSL

// shifts
#define RESTIRSSS_SHIFT_RECONNECTION 0
#define RESTIRSSS_SHIFT_DELAYED_RECONNECTION 1

#define RESTIRSSS_SHIFT_HYBRID 2

#define RESTIRSSS_SHIFT_SEQUENTIAL_RECONNECTION 3
#define RESTIRSSS_SHIFT_SEQUENTIAL_DELAYED_RECONNECTION 4

// sequential shift performance optimization (introduces small bias): calculate intersection id for the selected sample only after the resampling pass and possibly set intersection id to an unknown state (see paper Sec. 4.3 for details)
#define RESTIRSSS_RECONNECTION_INTERSECTIONID_DELAYED

// sample a light source per candidate and share between all intersections (performance) or sample light source for each found intersection (less variance possible)
#define RESTIRSSS_LIGHT_PER_CANDIDATE

//#define RESTIRSSS_PROBINVERSETEST_L3PATH

//#define RESTIRSSS_WRITE_DEBUG_IMAGES

#endif