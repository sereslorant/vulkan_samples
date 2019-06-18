#version 430

layout(binding=0,std140) uniform UniformData
{
    vec4 Position;
};

layout(location=0) in vec2 Vertex;

void main()
{
    gl_Position = vec4(Vertex + Position.xy,0.0,1.0);
}
