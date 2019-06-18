#version 430

layout(location=0) in vec3 fNormal;
layout(location=1) in vec2 fTexCoord;

layout(location=0) out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(fNormal.xyz*0.5 + 0.5,1.0);
}
