#version 430

layout(binding=0,std140) uniform UniformData
{
    vec4 Position;
};

vec2 Positions[6] = vec2[](
    vec2(-0.25,-0.25),
    vec2(-0.25, 0.25),
    vec2( 0.25, 0.25),
    vec2(-0.25,-0.25),
    vec2( 0.25, 0.25),
    vec2( 0.25,-0.25)
);

void main()
{
    gl_Position = vec4(Positions[gl_VertexIndex] + Position.xy,0.0,1.0);
}
