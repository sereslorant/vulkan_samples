#version 430

layout(location=0) in vec3 fNormal;
layout(location=1) in vec2 fTexCoord;

layout(set=0,binding=0,std140) uniform Transform
{
    mat4 MvpMatrix;
    vec4 MatData;
};

layout(set=1,binding=1) uniform sampler2DArray TextureGroup;

layout(location=0) out vec4 FragmentColor;

void main()
{
    //FragmentColor = vec4(fTexCoord.xy,0.0,1.0);
    FragmentColor = vec4(texture(TextureGroup,vec3(fTexCoord.xy,MatData.w)).rgb,1.0);
}
