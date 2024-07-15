#ifndef AREALIGHT_GLSL
#define AREALIGHT_GLSL

float arealight_areaTriangle(in const vec3 p0, in const vec3 p1, in const vec3 p2) {
    return 0.5 * length(cross(p1 - p0, p2 - p0));
}

float arealight_pdf_vam(in const float numLights, in const vec3 p0World, in const vec3 p1World, in const vec3 p2World) {
    // pdf in solid angle by dividing by the geometry term
    return 1.0 / (numLights * arealight_areaTriangle(p0World, p1World, p2World));
}

void arealight_sample_vam(in const uint xi1, in const float xi2, in const float xi3, in const vec3 vertexPositionWorld, out int objectDescriptorId, out int triangleId, out vec2 barycentrics, out vec3 position, out vec3 normal, out vec3 emission, out float pdf) {
    const Light light = g_lights[xi1];// introduces 1 / numLights pdf

    objectDescriptorId = int(light.objectDescriptorId);
    triangleId = int(light.triangleId);

    ObjectDescriptor objDesc = g_objectDescriptors[light.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);

    const ivec3 ind = indices.i[light.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);

    const vec3 p0World = (objDesc.objectToWorld * vec4(p0, 1.0)).xyz;
    const vec3 p1World = (objDesc.objectToWorld * vec4(p1, 1.0)).xyz;
    const vec3 p2World = (objDesc.objectToWorld * vec4(p2, 1.0)).xyz;

    // generate point on the triangle
    const float s = 1 - sqrt(1 - xi2);
    const float t = (1 - s) * xi3;
    position = p0World + s * (p1World - p0World) + t * (p2World - p0World);

    const vec3 normalWorld = normalize(cross(p1World - p0World, p2World - p0World));
    normal = dot(normalWorld, normalize(vertexPositionWorld - position)) >= 0.0 ? normalWorld : -normalWorld;// align normal in correct direction

    emission = light.emission;

    // pdf in area measure
    pdf = arealight_pdf_vam(g_num_lights, p0World, p1World, p2World);

    // barycentrics from positions
    const float area = arealight_areaTriangle(p0World, p1World, p2World);
    barycentrics.x = arealight_areaTriangle(p0World, position, p2World) / area;// barycentricsY
    barycentrics.y = arealight_areaTriangle(p0World, p1World, position) / area;// barycentricsZ
}

void arealight_sample_vam(in const uint xi1, in const float xi2, in const float xi3, out int objectDescriptorId, out int triangleId, out vec2 barycentrics, out vec3 position, out vec3 unalignedNormal, out vec3 emission, out float pdf) {
    const Light light = g_lights[xi1];// introduces 1 / numLights pdf

    objectDescriptorId = int(light.objectDescriptorId);
    triangleId = int(light.triangleId);

    ObjectDescriptor objDesc = g_objectDescriptors[light.objectDescriptorId];
    Indices indices = Indices(objDesc.indexAddress);
    Vertices vertices = Vertices(objDesc.vertexAddress);

    const ivec3 ind = indices.i[light.triangleId];
    const Vertex v0 = vertices.v[ind.x];
    const Vertex v1 = vertices.v[ind.y];
    const Vertex v2 = vertices.v[ind.z];

    const vec3 p0 = vec3(v0.position_x, v0.position_y, v0.position_z);
    const vec3 p1 = vec3(v1.position_x, v1.position_y, v1.position_z);
    const vec3 p2 = vec3(v2.position_x, v2.position_y, v2.position_z);

    const vec3 p0World = (objDesc.objectToWorld * vec4(p0, 1.0)).xyz;
    const vec3 p1World = (objDesc.objectToWorld * vec4(p1, 1.0)).xyz;
    const vec3 p2World = (objDesc.objectToWorld * vec4(p2, 1.0)).xyz;

    // generate point on the triangle
    const float s = 1 - sqrt(1 - xi2);
    const float t = (1 - s) * xi3;
    position = p0World + s * (p1World - p0World) + t * (p2World - p0World);

    unalignedNormal = normalize(cross(p1World - p0World, p2World - p0World));

    emission = light.emission;

    // pdf in area measure
    pdf = arealight_pdf_vam(g_num_lights, p0World, p1World, p2World);

    // barycentrics from positions
    const float area = arealight_areaTriangle(p0World, p1World, p2World);
    barycentrics.x = arealight_areaTriangle(p0World, position, p2World) / area;// barycentricsY
    barycentrics.y = arealight_areaTriangle(p0World, p1World, position) / area;// barycentricsZ
}

#endif