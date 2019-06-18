#version 430 

layout(set=0,binding=0,std140) uniform TriData
{
    vec4 Position;
};

layout(set=1,binding=1) uniform sampler2DArray TextureGroup;

layout(location=0) in vec2 fTexCoord;

layout(location=0) out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(texture(TextureGroup,vec3(fTexCoord.xy,Position.w)).rgb,1.0);
}
