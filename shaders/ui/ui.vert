#version 440
in vec3 in_Position;
in vec2 in_Uvs;
in vec4 in_Color;
out vec2 texcoord;
out vec4 vcolor;

uniform mat4 orthomat;

void main(void)
{
    gl_Position = orthomat * vec4(in_Position, 1.0);
    texcoord = in_Uvs;
    vcolor = in_Color;
}
