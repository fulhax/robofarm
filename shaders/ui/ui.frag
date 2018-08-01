#version 440
out vec4 color;
in vec2 texcoord;
in vec4 vcolor;
void main()
{
    color = vcolor;
}
