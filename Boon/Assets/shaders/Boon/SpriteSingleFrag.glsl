struct BoonSpriteVertexInput
{
    vec4 Color;
    vec2 TexCoord;
};

layout (location = 0) in BoonSpriteVertexInput Input;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_Id;

layout (binding = 0) uniform sampler2D u_Texture;

layout(std140, binding = 1) uniform Object
{
    mat4 u_world;
    int u_ID;
};

vec4 Boon_SampleSpriteTexture()
{
    return Input.Color * texture(u_Texture, Input.TexCoord);
}

void Boon_OutputSprite(vec4 color)
{
    if (color.a == 0.0)
        discard;

    o_Color = color;
    o_Id = u_ID;
}
