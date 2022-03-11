#version 330 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec2 a_Offset;
layout (location = 3) in int a_Frame;

out vec2 v_TexCoord;

uniform int u_Rows = 1;
uniform int u_Columns = 1;
uniform float u_TileSize;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	float x = (a_Frame % u_Columns) * (1.0f / u_Columns);
	float y = 1 - (1.0f / u_Rows) - ((a_Frame / u_Columns) * (1.0f / u_Rows));

	float S = (u_Columns > 0) ? 1.0f / u_Columns : 1.0f;
	float T = (u_Rows > 0) ? 1.0f / u_Rows : 1.0f;

	v_TexCoord = vec2(x, y) + vec2(a_TexCoord.x * S, a_TexCoord.y * T);
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position * u_TileSize + a_Offset, 0.0, 1.0);
}