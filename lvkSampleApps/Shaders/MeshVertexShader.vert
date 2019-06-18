#version 430

layout(location=0) in vec3 Vertex;
layout(location=1) in vec3 Normal;
layout(location=2) in vec2 TexCoord;

layout(set=0,binding=0,std140) uniform Transform
{
    mat4 MvpMatrix;
    vec4 MatData;
};

layout(location=0) out vec3 fNormal;
layout(location=1) out vec2 fTexCoord;

void main()
{
    fNormal = Normal;
    fTexCoord = TexCoord;
    
    gl_Position = MvpMatrix * vec4(Vertex.xyz,1.0);
}
