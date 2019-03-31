#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;


layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fDiffuse;


uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform vec3 uKd;
uniform sampler2D uKdSampler;

void main()
{
    fPosition = vViewSpacePosition;
    fNormal = normalize(vViewSpaceNormal);

    vec3 kd = uKd * vec3(texture(uKdSampler, vTexCoords));

    fDiffuse = kd;

}
