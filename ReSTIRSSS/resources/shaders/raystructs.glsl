#ifndef RAYSTRUCTS_GLSL
#define RAYSTRUCTS_GLSL

struct ObjectDescriptor {
    uint64_t vertexAddress;
    uint64_t indexAddress;
    uint materialId;

    mat4 objectToWorld;
    mat4 objectToWorldNormal; // == transpose(inverse(objectToWorld))
};

struct Vertex {
    float position_x;
    float position_y;
    float position_z;
    float normal_x;
    float normal_y;
    float normal_z;
    float tangent_x;
    float tangent_y;
    float tangent_z;
    float tangent_w;
    float texCoord_u;
    float texCoord_v;
};

struct Material {
    vec3 baseColorFactor;
    int baseColorTexture;
    float metallicFactor;
    float roughnessFactor;
    int metallicRoughnessTexture;

    int normalTexture;

    vec3 emission;

    float subsurface;
    vec3 meanFreePath;
};

struct Light {
    uint type;

    uint objectDescriptorId;
    uint triangleId;

    vec3 position;
    vec3 emission;
};

#define GBUFFER_NULL 0xFFFFFFFFu

struct GBuffer {
    uint objectDescriptorId; // gl_InstanceCustomIndexEXT
    uint triangleId; // gl_PrimitiveID
    float barycentricsY; // encodes vec3(1.0 - barycentricsY - barycentricsZ, barycentricsY, barycentricsZ)
    float barycentricsZ; // encodes vec3(1.0 - barycentricsY - barycentricsZ, barycentricsY, barycentricsZ)
};

#endif