#version 330

// Everything in View Space
uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGDiffuse;

out vec3 fColor;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

void main()
{
    vec3 position = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0));
    vec3 normal = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));

    vec3 kd = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));


	float distToPointLight = length(uPointLightPosition - position);
    	vec3 dirToPointLight = (uPointLightPosition - position) / distToPointLight;

	fColor = kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + uPointLightIntensity * max(0., dot(normal, dirToPointLight)) / (distToPointLight * distToPointLight));

 
}
