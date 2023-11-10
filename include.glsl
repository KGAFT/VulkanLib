#define PI 3.14159265359

float distributeGGX(vec3 normals, vec3 halfWayVector, float roughness)
{
    float roughness4 = roughness*roughness*roughness*roughness;
    float halwayDot2 = max(dot(normals, halfWayVector), 0.0)*max(dot(normals, halfWayVector), 0.0);
    float numerator   = roughness4;
    float denominator = (halwayDot2 * (roughness4 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;
    return numerator / denominator;
}