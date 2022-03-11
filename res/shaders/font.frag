#version 330 core

layout(location = 0) out vec4 f_Color;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

in vec2 v_TexCoord;

void main()
{
	f_Color = vec4(u_Color.xyz, texture(u_Texture, v_TexCoord).r);
}
