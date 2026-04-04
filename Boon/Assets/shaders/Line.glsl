#vert
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout (location = 0) out vec4 v_Color;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

void main()
{
    v_Color = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#frag
#version 450 core


layout (location = 0) in vec4 v_Color;

layout (location = 0) out vec4 o_Color;
layout(location = 1) out int o_Id;

void main()
{
    o_Color = v_Color;
    o_Id = -1;
}