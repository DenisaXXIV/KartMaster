#version 330 core
out  vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

// uniform vec3 lightPos;
uniform vec3 lightDirection;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perspective division
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Map to the [0,1] interval
    projCoords = projCoords * 0.5 + 0.5;
    // Get the nearest depth value on the path pointing to the current fragment from the light's point of view (using the [0,1] interval fragPosLight coordinates)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get the depth of the current fragment from the light's point of view
    float currentDepth = projCoords.z;
    
    // calculate deviation
    vec3 normal = normalize(fs_in.Normal);
    float bias = max(0.001 * (1.0 - dot(normal, lightDirection)), 0.00001);
    
    // Check if the current frag is in shadow
    // float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // PCF optimizes jagged shadow edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // When the depth of the fragment exceeds 1.0, no shadow is drawn to avoid the phenomenon of shadows in the distance
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.8);

    // ambient ambient light
    vec3 ambient = 0.3 * color;
    
    // diffuse diffuse
    float diff = max(dot(lightDirection, normal), 0.0);
    vec3 diffuse = diff * color;   // The darker the texture color, the weaker the diffuse reflection
    
    // specular specular reflection
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDirection, normal);
    
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    
    FragColor = vec4(lighting, 1.0);
}