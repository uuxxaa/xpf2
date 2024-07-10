cbuffer VertexConstants : register(b0)
{
    float4x4 modelViewProj;
};

cbuffer PixelConstants : register(b0)
{
    int commandId;
    float width;
    float height;
    float corner_topleft;
    float corner_topright;
    float corner_bottomright;
    float corner_bottomleft;
    float border_left;
    float border_top;
    float border_right;
    float border_bottom;
    float fillColor_r;
    float fillColor_g;
    float fillColor_b;
    float fillColor_a;
};

struct VS_Input
{
    float2 pos : POS;
    float4 color : COL;
    float2 uv : TEX;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COL;
    float2 uv : TEXCOORD;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

SamplerState textSampler
{
    Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = mul(float4(input.pos, 0.0f, 1.0f), modelViewProj);
    output.uv = input.uv;
    output.color = input.color;

    return output;
}

float rect_corner(float width, const float height, float cr, float x, float y) {
    float s0 = 0.95;
    float s1 = .99;

    float cr2 = cr * cr;
    float u = x - (width - cr);
    float v = y - (height - cr);

    float cx = (u * abs(u) + v * abs(v)) / cr2;
    return smoothstep(s0, s1, cx);
}

float rect_round_corners(
    const float x, const float y,
    const float width, const float height,
    float corner_topleft,
    float corner_topright,
    float corner_bottomright,
    float corner_bottomleft) {
    float alpha =
        rect_corner(width, height, corner_topleft, -x, -y) +
        rect_corner(width, height, corner_topright, x, -y) +
        rect_corner(width, height, corner_bottomright, x, y) +
        rect_corner(width, height, corner_bottomleft, -x, y);
    return alpha;
}

float rect_round_corners_inner(
    float x, float y,
    const float width, const float height,
    float cr,
    float border_left,
    float border_top,
    float border_right,
    float border_bottom) {
    float alpha =
        rect_corner(width - border_left,  height - border_top,    cr, -x + cr, -y + cr) +
        rect_corner(width - border_right, height - border_top,    cr,  x + cr, -y + cr) +
        rect_corner(width - border_right, height - border_bottom, cr,  x + cr,  y + cr) +
        rect_corner(width - border_left,  height - border_bottom, cr, -x + cr,  y + cr);
    return alpha;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    float4 pixel;
    if (commandId == 0 || commandId == 1)
    {
        pixel = input.color;
    }
    else if (commandId == 2)
    {
        pixel = input.color * mytexture.Sample(mysampler, input.uv);
    }
    else if (commandId == 3) // text
    {
        pixel = input.color * mytexture.Sample(textSampler, input.uv).r;
    }
    else if (commandId == 4) // rounded_rectangle
    {
        float width_2 = width * .5;
        float height_2 = height * .5;

        float2 pos = input.uv;
        float x = (pos.x * 2 - 1) * width_2;
        float y = (pos.y * 2 - 1) * height_2;

        float alpha = rect_round_corners(
            x, y, width_2, height_2,
            corner_topleft,
            corner_topright,
            corner_bottomright,
            corner_bottomleft);

        pixel = float4(input.color.rgb, (1.0 - alpha) * input.color.a);
    }
    else if (commandId == 5) // rounded_rectangle_with_border
    {
        float width_2 = width * .5;
        float height_2 = height * .5;

        float2 pos = input.uv;
        float x = (pos.x * 2 - 1) * width_2;
        float y = (pos.y * 2 - 1) * height_2;

        float alpha = rect_round_corners(
            x, y, width_2, height_2,
            corner_topleft,
            corner_topright,
            corner_bottomright,
            corner_bottomleft);

        float cr_inner = (corner_topleft + corner_topright + corner_bottomright + corner_bottomleft) * .125; // half of avg outter corner radius

        float alpha2 = rect_round_corners_inner(
            x, y, width_2, height_2,
            cr_inner,
            border_left - cr_inner,
            border_top - cr_inner,
            border_right - cr_inner,
            border_bottom - cr_inner);

        float a = (1-step(x, (-width_2 +  border_left))) * step(x, (width_2 - border_right));
        float b = (1-step(y, (-height_2 + border_top)))  * step(y, (height_2 - border_bottom));

        float4 fillColor = float4(fillColor_r, fillColor_g, fillColor_b, fillColor_a);
        float4 borderColor = input.color;

        float fillMult = ((a * b)*(1-alpha2));
        float borderMult = (1-(a * b)*(1-alpha2))*(1-alpha);

        pixel = fillColor * fillMult + borderColor * borderMult;
    }
    else
    {
        pixel = float4(.5,0,.5,.5);
    }

    if (pixel.a < 0.01f)
        discard;

    return pixel;
}
