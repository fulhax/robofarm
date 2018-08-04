#version 440
uniform sampler2D texture;
uniform bool hastexture;
out vec4 color;
in vec2 texcoord;
in vec4 vcolor;
void main()
{
    if(hastexture)
    {
        color = vcolor * texture2D(texture, texcoord);
    }
    else
    {
        color = vcolor;
    }
}
