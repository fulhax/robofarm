#version 440
in vec3 in_Position;
in vec2 in_Uvs;
in vec4 in_Color;
out vec2 texcoord;
out vec3 position;
out vec4 vcolor;

mat4 m = mat4(2.0 / 512.0, 0.0, 0.0, 0.0,
              0.0, -2.0 / 512.0, 0.0, 0.0,
              0.0, 0.0, 1.0, 0.0,
              0.0, 0.0, 0.0, 1.0);

void main(void)
{
    gl_Position = m * vec4(in_Position, 1.0);
    position = gl_Position.xyz;
    texcoord = in_Uvs;
    vcolor = in_Color;
}
