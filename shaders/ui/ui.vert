#version 440
in vec3 in_Position;
in vec2 in_Uvs;
in vec4 in_Color;
out vec2 texcoord;
out vec3 position;
out vec4 vcolor;


void main(void)
{
    gl_Position = vec4(in_Position, 1.0);
    position = gl_Position.xyz;
    texcoord = in_Uvs;
    vcolor = in_Color;
}
