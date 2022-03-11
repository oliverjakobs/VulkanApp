#version 330 core

layout(location = 0) out vec4 f_Color;

in vec2 v_TexCoords;
in float v_TexIndex;

uniform sampler2D u_Textures[8];

void main()
{
	int index = int(v_TexIndex);
	f_Color = texture(u_Textures[index], v_TexCoords);
}