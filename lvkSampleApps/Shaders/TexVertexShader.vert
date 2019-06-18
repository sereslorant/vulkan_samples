#version 430

layout(set=0,binding=0,std140) uniform UniformData
{
    vec4 Position;
};

layout(location=0) in vec2 Vertex;
layout(location=1) in vec2 TexCoord;

layout(location=0) out vec2 fTexCoord;

void main()
{
    fTexCoord = TexCoord;
    
    gl_Position = vec4(Vertex + Position.xy,0.0,1.0);
}
