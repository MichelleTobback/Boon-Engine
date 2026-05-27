#vert

#version 450 core

// @vertex vec3 a_Position
// @vertex vec4 a_Color
// @vertex vec2 a_TexCoord
// @vertex float a_TexIndex
// @vertex float a_TilingFactor
// @vertex int a_ID

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_ID;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat float v_TexIndex;
layout (location = 4) out flat int v_ID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};


void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	Output.TilingFactor = a_TilingFactor;
	v_TexIndex = a_TexIndex;
	v_ID = a_ID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#frag

#version 450 core

// @texture_array u_Textures 0 32

#include "Boon/SpriteBatchFrag.glsl"

void main()
{
	vec4 texColor = Boon_SampleSpriteTexture();
	Boon_OutputSprite(texColor);
}
