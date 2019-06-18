#version 400

vec2 Positions[6] = vec2[](
    vec2(-0.5,-0.5),
    vec2(-0.5, 0.5),
    vec2( 0.5, 0.5),
    vec2(-0.5,-0.5),
    vec2( 0.5, 0.5),
    vec2( 0.5,-0.5)
);

void main()
{
    gl_Position = vec4(Positions[gl_VertexIndex],0.0,1.0);
}
