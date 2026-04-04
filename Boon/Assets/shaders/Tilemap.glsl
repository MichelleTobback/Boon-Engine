#vert

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(std140, binding = 1) uniform Object
{
	mat4 u_world;
	int u_ID;
};


void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;

	gl_Position = u_ViewProjection * u_world * vec4(a_Position, 1.0);
}

#frag

#version 450 core

struct VertexInput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) in VertexInput Input;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_Id;

layout (binding = 0) uniform sampler2D u_Texture;

layout(std140, binding = 1) uniform Object
{
	mat4 u_world;
	int u_ID;
};

void main()
{
	vec4 texColor = Input.Color * texture(u_Texture, Input.TexCoord);

	if (texColor.a == 0.0)
		discard;

	o_Color = texColor;
	o_Id = u_ID;
}