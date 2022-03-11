#version 330 core

layout(location = 0) out vec4 f_Color;

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
	f_Color = texture(u_Texture, v_TexCoord) * u_Color;
}