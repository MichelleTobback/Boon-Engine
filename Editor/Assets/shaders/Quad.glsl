#vert

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in int a_ID;

struct VertexOutput
{
	vec4 Color;
};

layout (location = 0) out VertexOutput Output;
layout (location = 1) out flat int v_ID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};


void main()
{
	Output.Color = a_Color;
	v_ID = a_ID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#frag

#version 450 core

struct VertexInput
{
	vec4 Color;
};

layout (location = 0) in VertexInput Input;
layout (location = 1) in flat int v_ID;

layout(location = 0) out vec4 color;
layout(location = 1) out int id;

void main()
{
	color = Input.Color;
	id = v_ID;
}