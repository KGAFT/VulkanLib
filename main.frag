#version 450

#define LIGHT_BLOCKS_AMOUNT 100
#define PI 3.14159265359

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 cameraPosition;
struct PointLight{
    vec3 position;
    vec3 color;
    float intensity;
};

struct DirectLight{
    vec3 direction;
    vec3 color;
    float intensity;
};

layout(std140, binding = 0) uniform LightConfiguration{
    DirectLight directLights[LIGHT_BLOCKS_AMOUNT];
    PointLight pointLights[LIGHT_BLOCKS_AMOUNT];

    int enabledDirects;
    int enabledPoints;

    float emissiveIntensity;
    float emissiveShininess;
    float gammaCorrect;
    float ambientIntensity;
} lightUbo;

vec3 fragmentPosition;

layout(set = 0, binding = 1) uniform sampler2D verticesSampler;
layout(set = 0, binding = 2) uniform sampler2D albedoSampler;
layout(set = 0, binding = 3) uniform sampler2D normalSampler;
layout(set = 0, binding = 4) uniform sampler2D metallicRoughnessEmissiveSampler;
layout(set = 0, binding = 5) uniform sampler2D aoMap;
layout (set = 0, binding = 6) uniform samplerCube prefilteredMap;
layout(set = 0, binding = 7) uniform sampler2D background;
layout(location = 0) out vec4 fragColor;


float distributeGGX(vec3 normals, vec3 halfWayVector, float roughness)
{
    float roughness4 = roughness*roughness*roughness*roughness;
    float halwayDot2 = max(dot(normals, halfWayVector), 0.0)*max(dot(normals, halfWayVector), 0.0);
    float numerator   = roughness4;
    float denominator = (halwayDot2 * (roughness4 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;
    return numerator / denominator;
}

float schlickGeometryGGX(float dotWorldViewVector, float roughness)
{
    float roughnessKoef = ((roughness + 1.0)*(roughness + 1.0)) / 8.0;
    float numerator   = dotWorldViewVector;
    float denominator = dotWorldViewVector * (1.0 - roughnessKoef) + roughnessKoef;
    return numerator / denominator;
}

float smithGeometry(vec3 processedNormals, vec3 worldViewVector, vec3 lightPosition, float roughness)
{
    float worldViewVectorDot = max(dot(processedNormals, worldViewVector), 0.0);
    float lightDot = max(dot(processedNormals, lightPosition), 0.0);
    float ggx2 = schlickGeometryGGX(worldViewVectorDot, roughness);
    float ggx1 = schlickGeometryGGX(lightDot, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 startFresnelSchlick)
{
    return startFresnelSchlick + (1.0 - startFresnelSchlick) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 startFresnelSchlick, float roughness){
    return startFresnelSchlick + (max(vec3(1.0 - roughness), startFresnelSchlick) - startFresnelSchlick) * pow(1.0 - cosTheta, 5.0);
}

vec3 processPointLight(PointLight light, vec3 normals, vec3 worldViewVector, vec3 startFresnelSchlick, float roughness, float metallic, vec3 albedo){
    vec3 processedLightPos = normalize(light.position - fragmentPosition);
    vec3 halfWay = normalize(worldViewVector + processedLightPos);
    float distance = length(light.position - fragmentPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.color * light.intensity * attenuation;
    float halfWayGGX = distributeGGX(normals, halfWay, roughness);
    float geometrySmith   = smithGeometry(normals, worldViewVector, processedLightPos, roughness);
    vec3 fresnelSchlick    = fresnelSchlick(clamp(dot(halfWay, worldViewVector), 0.0, 1.0), startFresnelSchlick);
    vec3 numerator    = halfWayGGX * geometrySmith * fresnelSchlick;
    float denominator = 4.0 * max(dot(normals, worldViewVector), 0.0) * max(dot(normals, processedLightPos), 0.0) + 0.0001;// + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    vec3 finalFresnelSchlick = vec3(1.0) - fresnelSchlick;
    finalFresnelSchlick *= 1.0 - metallic;
    float NdotL = max(dot(normals, processedLightPos), 0.0);
    return (finalFresnelSchlick * albedo / PI + specular) * radiance * NdotL;
}

vec3 processDirectionaLight(DirectLight light, vec3 normals, vec3 worldViewVector, vec3 startFresnelSchlick, float roughness, float metallic, vec3 albedo){
    vec3 processedLightPos = normalize(-light.direction);
    vec3 halfWay = normalize(worldViewVector + processedLightPos);
    vec3 radiance = light.color * light.intensity;
    float halfWayGGX = distributeGGX(normals, halfWay, roughness);
    float geometrySmith   = smithGeometry(normals, worldViewVector, processedLightPos, roughness);
    vec3 fresnelSchlick    = fresnelSchlick(clamp(dot(halfWay, worldViewVector), 0.0, 1.0), startFresnelSchlick);
    vec3 numerator    = halfWayGGX * geometrySmith * fresnelSchlick;
    float denominator = 4.0 * max(dot(normals, worldViewVector), 0.0) * max(dot(normals, processedLightPos), 0.0) + 0.0001;// + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    vec3 finalFresnelSchlick = vec3(1.0) - fresnelSchlick;
    finalFresnelSchlick *= 1.0 - metallic;
    float NdotL = max(dot(normals, processedLightPos), 0.0);
    return (finalFresnelSchlick * albedo / PI + specular) * radiance * NdotL;
}


vec3 postProcessColor(vec3 color){
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(lightUbo.gammaCorrect));
    return color;
}
vec3 getReflection(float roughness, vec3 reflectanceVec){
    const float MAX_REFLECTION_LOD = 9.0;
    float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilteredMap, reflectanceVec, lodf).rgb;
	vec3 b = textureLod(prefilteredMap, reflectanceVec, lodc).rgb;
	return mix(a, b, lod - lodf);
}
void main() {
    fragmentPosition = texture(verticesSampler, uv).xyz;
    if(fragmentPosition.x!=0 || fragmentPosition.y!=0 || fragmentPosition.z!=0){
        vec3 processedNormals = texture(normalSampler, uv).xyz;
        vec4 albedoSource = texture(albedoSampler, uv);
        vec3 albedo = pow(albedoSource.rgb, vec3(2.2));
        vec4 pbrData = texture(metallicRoughnessEmissiveSampler, uv);

        float metallic = pbrData.r;
        float roughness = pbrData.g;
        vec3 emissive = albedo*pbrData.b;
        float ao = texture(aoMap, uv).r;
        float opacity = albedoSource.a;
        // vec3 reflection = getReflection(roughness, uv);
    
        vec3 worldViewVector = normalize(cameraPosition - fragmentPosition);
        vec3 reflection = getReflection(roughness, reflect(worldViewVector, processedNormals));
        vec3 startFresnelSchlick = vec3(0.04);
        startFresnelSchlick = mix(startFresnelSchlick, albedo, metallic);

        vec3 Lo = vec3(0.0);

        for (int c = 0; c<lightUbo.enabledDirects; c++){
            Lo+=processDirectionaLight(lightUbo.directLights[c], processedNormals, worldViewVector, startFresnelSchlick, roughness, metallic, albedo);
        }
        for (int c = 0; c<lightUbo.enabledPoints; c++){
            Lo+=processPointLight(lightUbo.pointLights[c], processedNormals, worldViewVector, startFresnelSchlick, roughness, metallic, albedo);
        }

        vec3 fresnelRoughness = fresnelSchlickRoughness(max(dot(processedNormals, worldViewVector), 0.0), startFresnelSchlick, roughness);
        vec3 specularContribution = reflection*fresnelRoughness*0.025f;

        vec3 ambient = (vec3(lightUbo.ambientIntensity) * albedo * ao +specularContribution);

        vec3 color = ambient + Lo;

        color+=(emissive*pow(1, lightUbo.emissiveShininess)*lightUbo.emissiveIntensity);
        color = postProcessColor(color);

        fragColor = vec4(color, 1);
    }
    else{
        fragColor = texture(background, uv);
    }
}
