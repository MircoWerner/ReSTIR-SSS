#ifndef TRACE_GLSL
#define TRACE_GLSL

#include "../defines.glsl"
#include "../raycommon.glsl"
#include "../utility/normal_mapping.glsl"
#include "../material.glsl"
#include "../bxdf/bsdf/bsdf.glsl"

bool trace(in const vec3 origin, in const vec3 direction, in const float tMin, in const float tMax, out HitPayload hitPayload);

void intersectionInfo(in const HitPayload payload, out vec3 positionWorld, out vec3 geometricNormalWorld, out vec3 normalWorld, out vec2 uv, out Material material) {
    const ObjectDescriptor objDesc = g_objectDescriptors[payload.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);
    material = g_materials[objDesc.materialId];

    const ivec3 ind = indices.i[payload.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);
    const vec3 n0 = vec3(v0.normal_x, v0.normal_y, v0.normal_z);
    const vec3 n1 = vec3(v1.normal_x, v1.normal_y, v1.normal_z);
    const vec3 n2 = vec3(v2.normal_x, v2.normal_y, v2.normal_z);
    const vec2 uv0 = vec2(v0.texCoord_u, v0.texCoord_v);
    const vec2 uv1 = vec2(v1.texCoord_u, v1.texCoord_v);
    const vec2 uv2 = vec2(v2.texCoord_u, v2.texCoord_v);

    const vec3 barycentrics = vec3(1.0 - payload.barycentrics.x - payload.barycentrics.y, payload.barycentrics.x, payload.barycentrics.y);

    // POSITION
    const vec3 positionObject = p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    positionWorld = vec3(objDesc.objectToWorld * vec4(positionObject, 1.0));

    // UV
    uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;

    // GEOMETRIC NORMAL
    geometricNormalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(cross(p1 - p0, p2 - p0), 0.0)));
    geometricNormalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? geometricNormalWorld : -geometricNormalWorld;

    #ifdef GEOMETRIC_NORMAL
    normalWorld = geometricNormalWorld;
    #else
    // NORMAL
    const vec3 normalObject = normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z);
    if (g_normal_mapping != 0 && material.normalTexture >= 0) {
        // normal mapping
        // construct tangent space
        const vec4 t0 = vec4(v0.tangent_x, v0.tangent_y, v0.tangent_z, v0.tangent_w);
        const vec4 t1 = vec4(v1.tangent_x, v1.tangent_y, v1.tangent_z, v1.tangent_w);
        const vec4 t2 = vec4(v2.tangent_x, v2.tangent_y, v2.tangent_z, v2.tangent_w);
        vec4 tangentObject = t0 * barycentrics.x + t1 * barycentrics.y + t2 * barycentrics.z;
        tangentObject = vec4(normalize(vec3(tangentObject.xyz)), tangentObject.w);
        const vec3 biTangentObject = normalize(cross(normalObject, tangentObject.xyz) * sign(tangentObject.w));
        const mat3 TBN = tangentSpaceMatrix(tangentObject.xyz, biTangentObject, normalObject);

        // read normal texture
        const vec3 shadingNormalTangent = normalize(vec3(2.0) * texture(textures[material.normalTexture], uv).rgb - vec3(1.0));

        // transform tangent space normal to world space
        normalWorld = normalize((objDesc.objectToWorldNormal * vec4(TBN * shadingNormalTangent, 0.0)).xyz);

        if (any(isnan(normalWorld))) {
            normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
        }
    } else {
        // w/o normal mapping
        normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
    }
    normalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? normalWorld : -normalWorld;
    #endif
}

void intersectionInfo(in const HitPayload payload, out vec3 positionWorld, out vec3 geometricNormalWorld, out vec3 normalWorld, out BSDFMaterial bsdfMaterial) {
    const ObjectDescriptor objDesc = g_objectDescriptors[payload.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);
    const Material material = g_materials[objDesc.materialId];

    const ivec3 ind = indices.i[payload.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);
    const vec3 n0 = vec3(v0.normal_x, v0.normal_y, v0.normal_z);
    const vec3 n1 = vec3(v1.normal_x, v1.normal_y, v1.normal_z);
    const vec3 n2 = vec3(v2.normal_x, v2.normal_y, v2.normal_z);
    const vec2 uv0 = vec2(v0.texCoord_u, v0.texCoord_v);
    const vec2 uv1 = vec2(v1.texCoord_u, v1.texCoord_v);
    const vec2 uv2 = vec2(v2.texCoord_u, v2.texCoord_v);

    const vec3 barycentrics = vec3(1.0 - payload.barycentrics.x - payload.barycentrics.y, payload.barycentrics.x, payload.barycentrics.y);

    // POSITION
    const vec3 positionObject = p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    positionWorld = vec3(objDesc.objectToWorld * vec4(positionObject, 1.0));

    // UV
    const vec2 uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;

    // MATERIAL
    bsdfMaterial = bsdfMaterialInit(material, uv);

    // GEOMETRIC NORMAL
    geometricNormalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(cross(p1 - p0, p2 - p0), 0.0)));
    geometricNormalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? geometricNormalWorld : -geometricNormalWorld;

    #ifdef GEOMETRIC_NORMAL
    normalWorld = geometricNormalWorld;
    #else
    // NORMAL
    const vec3 normalObject = normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z);
    if (g_normal_mapping != 0 && material.normalTexture >= 0) {
        // normal mapping
        // construct tangent space
        const vec4 t0 = vec4(v0.tangent_x, v0.tangent_y, v0.tangent_z, v0.tangent_w);
        const vec4 t1 = vec4(v1.tangent_x, v1.tangent_y, v1.tangent_z, v1.tangent_w);
        const vec4 t2 = vec4(v2.tangent_x, v2.tangent_y, v2.tangent_z, v2.tangent_w);
        vec4 tangentObject = t0 * barycentrics.x + t1 * barycentrics.y + t2 * barycentrics.z;
        tangentObject = vec4(normalize(vec3(tangentObject.xyz)), tangentObject.w);
        const vec3 biTangentObject = normalize(cross(normalObject, tangentObject.xyz) * sign(tangentObject.w));
        const mat3 TBN = tangentSpaceMatrix(tangentObject.xyz, biTangentObject, normalObject);

        // read normal texture
        const vec3 shadingNormalTangent = normalize(vec3(2.0) * texture(textures[material.normalTexture], uv).rgb - vec3(1.0));

        // transform tangent space normal to world space
        normalWorld = normalize((objDesc.objectToWorldNormal * vec4(TBN * shadingNormalTangent, 0.0)).xyz);

        if (any(isnan(normalWorld))) {
            normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
        }
    } else {
        // w/o normal mapping
        normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
    }
    normalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? normalWorld : -normalWorld;
    #endif
}

void intersectionInfo(in const HitPayload payload, in const vec3 alignVertex, out vec3 positionWorld, out vec3 geometricNormalWorld, out vec3 normalWorld, out BSDFMaterial bsdfMaterial) {
    const ObjectDescriptor objDesc = g_objectDescriptors[payload.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);
    const Material material = g_materials[objDesc.materialId];

    const ivec3 ind = indices.i[payload.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);
    const vec3 n0 = vec3(v0.normal_x, v0.normal_y, v0.normal_z);
    const vec3 n1 = vec3(v1.normal_x, v1.normal_y, v1.normal_z);
    const vec3 n2 = vec3(v2.normal_x, v2.normal_y, v2.normal_z);
    const vec2 uv0 = vec2(v0.texCoord_u, v0.texCoord_v);
    const vec2 uv1 = vec2(v1.texCoord_u, v1.texCoord_v);
    const vec2 uv2 = vec2(v2.texCoord_u, v2.texCoord_v);

    const vec3 barycentrics = vec3(1.0 - payload.barycentrics.x - payload.barycentrics.y, payload.barycentrics.x, payload.barycentrics.y);

    // POSITION
    const vec3 positionObject = p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    positionWorld = vec3(objDesc.objectToWorld * vec4(positionObject, 1.0));

    const vec3 v = normalize(alignVertex - positionWorld);

    // UV
    const vec2 uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;

    // MATERIAL
    bsdfMaterial = bsdfMaterialInit(material, uv);

    // GEOMETRIC NORMAL
    geometricNormalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(cross(p1 - p0, p2 - p0), 0.0)));
//    geometricNormalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? geometricNormalWorld : -geometricNormalWorld;
    geometricNormalWorld = dot(v, geometricNormalWorld) >= 0 ? geometricNormalWorld : - geometricNormalWorld;

    #ifdef GEOMETRIC_NORMAL
    normalWorld = geometricNormalWorld;
    #else
    // NORMAL
    const vec3 normalObject = normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z);
    if (g_normal_mapping != 0 && material.normalTexture >= 0) {
        // normal mapping
        // construct tangent space
        const vec4 t0 = vec4(v0.tangent_x, v0.tangent_y, v0.tangent_z, v0.tangent_w);
        const vec4 t1 = vec4(v1.tangent_x, v1.tangent_y, v1.tangent_z, v1.tangent_w);
        const vec4 t2 = vec4(v2.tangent_x, v2.tangent_y, v2.tangent_z, v2.tangent_w);
        vec4 tangentObject = t0 * barycentrics.x + t1 * barycentrics.y + t2 * barycentrics.z;
        tangentObject = vec4(normalize(vec3(tangentObject.xyz)), tangentObject.w);
        const vec3 biTangentObject = normalize(cross(normalObject, tangentObject.xyz) * sign(tangentObject.w));
        const mat3 TBN = tangentSpaceMatrix(tangentObject.xyz, biTangentObject, normalObject);

        // read normal texture
        const vec3 shadingNormalTangent = normalize(vec3(2.0) * texture(textures[material.normalTexture], uv).rgb - vec3(1.0));

        // transform tangent space normal to world space
        normalWorld = normalize((objDesc.objectToWorldNormal * vec4(TBN * shadingNormalTangent, 0.0)).xyz);

        if (any(isnan(normalWorld))) {
            normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
        }
    } else {
        // w/o normal mapping
        normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
    }
//    normalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? normalWorld : -normalWorld;
    normalWorld = dot(v, normalWorld) >= 0 ? normalWorld : -normalWorld;
    #endif
}

void intersectionInfo(in const HitPayload payload, out vec3 positionWorld, out vec3 geometricNormalWorld, out vec3 normalWorld) {
    const ObjectDescriptor objDesc = g_objectDescriptors[payload.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);
    const Material material = g_materials[objDesc.materialId];

    const ivec3 ind = indices.i[payload.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);
    const vec3 n0 = vec3(v0.normal_x, v0.normal_y, v0.normal_z);
    const vec3 n1 = vec3(v1.normal_x, v1.normal_y, v1.normal_z);
    const vec3 n2 = vec3(v2.normal_x, v2.normal_y, v2.normal_z);
    const vec2 uv0 = vec2(v0.texCoord_u, v0.texCoord_v);
    const vec2 uv1 = vec2(v1.texCoord_u, v1.texCoord_v);
    const vec2 uv2 = vec2(v2.texCoord_u, v2.texCoord_v);

    const vec3 barycentrics = vec3(1.0 - payload.barycentrics.x - payload.barycentrics.y, payload.barycentrics.x, payload.barycentrics.y);

    // POSITION
    const vec3 positionObject = p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    positionWorld = vec3(objDesc.objectToWorld * vec4(positionObject, 1.0));

    // UV
    const vec2 uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;

    // GEOMETRIC NORMAL
    geometricNormalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(cross(p1 - p0, p2 - p0), 0.0)));
    geometricNormalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? geometricNormalWorld : -geometricNormalWorld;

    #ifdef GEOMETRIC_NORMAL
    normalWorld = geometricNormalWorld;
    #else
    // NORMAL
    const vec3 normalObject = normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z);
    if (g_normal_mapping != 0 && material.normalTexture >= 0) {
        // normal mapping
        // construct tangent space
        const vec4 t0 = vec4(v0.tangent_x, v0.tangent_y, v0.tangent_z, v0.tangent_w);
        const vec4 t1 = vec4(v1.tangent_x, v1.tangent_y, v1.tangent_z, v1.tangent_w);
        const vec4 t2 = vec4(v2.tangent_x, v2.tangent_y, v2.tangent_z, v2.tangent_w);
        vec4 tangentObject = t0 * barycentrics.x + t1 * barycentrics.y + t2 * barycentrics.z;
        tangentObject = vec4(normalize(vec3(tangentObject.xyz)), tangentObject.w);
        const vec3 biTangentObject = normalize(cross(normalObject, tangentObject.xyz) * sign(tangentObject.w));
        const mat3 TBN = tangentSpaceMatrix(tangentObject.xyz, biTangentObject, normalObject);

        // read normal texture
        const vec3 shadingNormalTangent = normalize(vec3(2.0) * texture(textures[material.normalTexture], uv).rgb - vec3(1.0));// but this seems to be correct

        // transform tangent space normal to world space
        normalWorld = normalize((objDesc.objectToWorldNormal * vec4(TBN * shadingNormalTangent, 0.0)).xyz);

        if (any(isnan(normalWorld))) {
            normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
        }
    } else {
        // w/o normal mapping
        normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
    }
    normalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? normalWorld : -normalWorld;
    #endif
}

void intersectionInfoDEBUG(in const HitPayload payload, out ObjectDescriptor objDesc, out vec3 positionObject, out vec3 positionWorld, out vec3 geometricNormalWorld, out vec3 normalWorld, out vec3 tangentWorld, out vec3 biTangentWorld, out vec2 uv, out Material material) {
    objDesc = g_objectDescriptors[payload.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);
    material = g_materials[objDesc.materialId];

    const ivec3 ind = indices.i[payload.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);
    const vec3 n0 = vec3(v0.normal_x, v0.normal_y, v0.normal_z);
    const vec3 n1 = vec3(v1.normal_x, v1.normal_y, v1.normal_z);
    const vec3 n2 = vec3(v2.normal_x, v2.normal_y, v2.normal_z);
    const vec4 t0 = vec4(v0.tangent_x, v0.tangent_y, v0.tangent_z, v0.tangent_w);
    const vec4 t1 = vec4(v1.tangent_x, v1.tangent_y, v1.tangent_z, v1.tangent_w);
    const vec4 t2 = vec4(v2.tangent_x, v2.tangent_y, v2.tangent_z, v2.tangent_w);
    const vec2 uv0 = vec2(v0.texCoord_u, v0.texCoord_v);
    const vec2 uv1 = vec2(v1.texCoord_u, v1.texCoord_v);
    const vec2 uv2 = vec2(v2.texCoord_u, v2.texCoord_v);

    const vec3 barycentrics = vec3(1.0 - payload.barycentrics.x - payload.barycentrics.y, payload.barycentrics.x, payload.barycentrics.y);

    positionObject = p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    positionWorld = vec3(objDesc.objectToWorld * vec4(positionObject, 1.0));

    // UV
    uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;

    // GEOMETRIC NORMAL
    geometricNormalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(cross(p1 - p0, p2 - p0), 0.0)));
    geometricNormalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? geometricNormalWorld : -geometricNormalWorld;

    #ifdef GEOMETRIC_NORMAL
    normalWorld = geometricNormalWorld;
    #else
    // NORMAL, TANGENT, BITANGENT
    const vec3 normalObject = normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z);

    vec4 tangentObject = t0 * barycentrics.x + t1 * barycentrics.y + t2 * barycentrics.z;
    tangentObject = vec4(normalize(vec3(tangentObject.xyz)), tangentObject.w);
    tangentWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(tangentObject.xyz, 0.0)));
    const vec3 biTangentObject = normalize(cross(normalObject, tangentObject.xyz) * sign(tangentObject.w));
    biTangentWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(biTangentObject.xyz, 0.0)));

    if (g_normal_mapping != 0 && material.normalTexture >= 0) {
        // normal mapping
        // construct tangent space
        const mat3 TBN = tangentSpaceMatrix(tangentObject.xyz, biTangentObject, normalObject);

        // read normal texture
        // vec3 shadingNormalTangent = normalize(vec3(2.0, 2.0, 1.0) * texture(textures[material.normalTexture], uv).rgb - vec3(1.0, 1.0, 0.0)); // this looks nearly the same
        const vec3 shadingNormalTangent = normalize(vec3(2.0, 2.0, 2.0) * texture(textures[material.normalTexture], uv).rgb - vec3(1.0, 1.0, 1.0));// but this seems to be correct

        // transform tangent space normal to world space
        normalWorld = normalize((objDesc.objectToWorldNormal * vec4(TBN * shadingNormalTangent, 0.0)).xyz);
    } else {
        // w/o normal mapping
        normalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(normalObject, 0.0)));
    }
    normalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? normalWorld : -normalWorld;
    #endif
}

bool intersectionInfoLightSource(in const HitPayload payload, out vec3 p0World, out vec3 p1World, out vec3 p2World, out vec3 positionWorld, out vec3 geometricNormalWorld, out vec3 emission) {
    ObjectDescriptor objDesc = g_objectDescriptors[payload.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);
    const Material material = g_materials[objDesc.materialId];

    if (!material_isLightSource(material)) {
        return false;
    }

    const ivec3 ind = indices.i[payload.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);

    const vec3 barycentrics = vec3(1.0 - payload.barycentrics.x - payload.barycentrics.y, payload.barycentrics.x, payload.barycentrics.y);

    // POSITION
    p0World = (objDesc.objectToWorld * vec4(p0, 1.0)).xyz;
    p1World = (objDesc.objectToWorld * vec4(p1, 1.0)).xyz;
    p2World = (objDesc.objectToWorld * vec4(p2, 1.0)).xyz;
    positionWorld = p0World * barycentrics.x + p1World * barycentrics.y + p2World * barycentrics.z;

    // GEOMETRIC NORMAL
    geometricNormalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(cross(p1 - p0, p2 - p0), 0.0)));
    geometricNormalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? geometricNormalWorld : -geometricNormalWorld;

    emission = material.emission;

    return true;
}

void intersectionInfoLightSource(in const HitPayload payload, out vec3 positionWorld, out vec3 geometricNormalWorld, out vec3 emission) {
    ObjectDescriptor objDesc = g_objectDescriptors[payload.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);
    Material material = g_materials[objDesc.materialId];

    const ivec3 ind = indices.i[payload.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);

    const vec3 barycentrics = vec3(1.0 - payload.barycentrics.x - payload.barycentrics.y, payload.barycentrics.x, payload.barycentrics.y);

    // POSITION
    const vec3 positionObject = p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    positionWorld = vec3(objDesc.objectToWorld * vec4(positionObject, 1.0));

    // GEOMETRIC NORMAL
    geometricNormalWorld = normalize(vec3(objDesc.objectToWorldNormal * vec4(cross(p1 - p0, p2 - p0), 0.0)));
    geometricNormalWorld = payload.face == gl_HitKindFrontFacingTriangleEXT ? geometricNormalWorld : -geometricNormalWorld;

    emission = material.emission;
}

#endif