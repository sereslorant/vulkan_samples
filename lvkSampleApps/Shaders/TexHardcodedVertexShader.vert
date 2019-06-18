#version 430

layout(set=0,binding=0,std140) uniform UniformData
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

vec2 TexCoords[6] = vec2[](
    vec2(0.0,0.0),
    vec2(0.0,1.0),
    vec2(1.0,1.0),
    vec2(0.0,0.0),
    vec2(1.0,1.0),
    vec2(1.0,0.0)
);

layout(location=0) out vec2 fTexCoord;

void main()
{
    fTexCoord   = TexCoords[gl_VertexIndex];
    gl_Position = vec4(Positions[gl_VertexIndex] + Position.xy,0.0,1.0);
}
