#include <metal_stdlib>
using namespace metal;
constant float sqrt_1_over_2 = 0.707106781186548f;
constant float sqrt_2 = 1.414213562373095f;

struct Vertex {
    float2 position          [[attribute(0)]];
    float4 color             [[attribute(1)]];
    float2 textureCoordinate [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 color;
    float2 textureCoordinate;
    uint32_t dataOffset;
};

struct VertexData
{
    float4x4 projectionMatrix;
    int commandId;
};

struct GlyphInstanceData
{
    float4 color;               // 16
    float left, top;            // 8
    float right, bottom;        // 8 -- 32
    int32_t bearingX;           // 4
    int32_t width;              // 4
    int32_t bearingY;           // 4
    int32_t height;             // 4
    uint32_t dataOffset;        // 4 -- 40
    uint32_t padding0 = 0, padding1 = 0, padding2 = 0;
};

bool is_eq(float a, float b)
{
    return a - 0.0001 < b && b < a + .0001;
}

float sign(float2 p1, float2 p2, float2 p3)
{
    return sign((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y));
}

float cross2(float2 a, float2 b) { return a.x * b.y - a.y * b.x; }

float normal(float2 p1, float2 p2, float2 p3)
{
    float2 v1 = (p1 - p2);
    float2 v2 = (p1 - p3);
    return sign(cross2(v1, v2));
}

bool isPointInsideTriangle(float2 pt, float2 v1, float2 v2, float2 v3)
{
    float d1 = sign(pt, v1, v2);
    float d2 = sign(pt, v2, v3);
    return d1 == d2 && d2 == sign(pt, v3, v1);
}

vertex VertexOut
vertexShader(
    Vertex in [[stage_in]],
    constant VertexData& vertexData [[buffer(0)]],
    constant void* pInstanceData [[buffer(1)]],
    uint32_t vid [[vertex_id]],
    uint32_t iid [[instance_id]])
{
    VertexOut out;
    out.textureCoordinate = in.textureCoordinate;
    if (vertexData.commandId == 11)
    {
        constant GlyphInstanceData& instanceData = ((constant GlyphInstanceData*)pInstanceData)[iid];
        float2 outpos(0,0);
        if (vid == 0) // top left
        {
            outpos.x = instanceData.left;
            outpos.y = instanceData.top;
            out.textureCoordinate.x = instanceData.bearingX;
            out.textureCoordinate.y = instanceData.bearingY;
        }
        else if (vid == 1) // top right
        {
            outpos.x = instanceData.right;
            outpos.y = instanceData.top;
            out.textureCoordinate.x = (instanceData.bearingX + instanceData.width);
            out.textureCoordinate.y = instanceData.bearingY;
        }
        else if (vid == 2) // bottom left
        {
            outpos.x = instanceData.left;
            outpos.y = instanceData.bottom;
            out.textureCoordinate.x = instanceData.bearingX;
            out.textureCoordinate.y = instanceData.bearingY + instanceData.height;
        }
        else if (vid == 3) // bottom right
        {
            outpos.x = instanceData.right;
            outpos.y = instanceData.bottom;
            out.textureCoordinate.x = instanceData.bearingX + instanceData.width;
            out.textureCoordinate.y = instanceData.bearingY + instanceData.height;
        }

        outpos.x = floor(outpos.x) + .5;

        out.position = vertexData.projectionMatrix * float4(outpos, 0.0, 1.0);
        out.color = instanceData.color;
        out.dataOffset = instanceData.dataOffset;
    }
    else
    {
        out.position = vertexData.projectionMatrix * float4(in.position, 0.0, 1.0);
        out.textureCoordinate = in.textureCoordinate;
        out.color = in.color;
    }
    return out;
}

struct FragmentData {
    int32_t commandId;
    float width;
    float height;
    float scale;
    float4 corner;
    float4 border;
    float4 border_color;
    float time;
    int dataOffset;
};

float SDFCircle(float2 coords, float radius)
{
    const float c_edge = 0.005;
    const float c_smooth = 0.0025;

    float v = coords.x * coords.x + coords.y * coords.y - c_edge * c_edge;
    float2  g = float2(radius * coords.x, radius * coords.y);
    float dist = v / length(g);
    return 1 - smoothstep(c_edge, c_edge + c_smooth, dist);
}

float distanceToLine(float2 p1, float2 p2, float2 point) {
    float a = p1.y-p2.y;
    float b = p2.x-p1.x;
    return abs(a*point.x+b*point.y+p1.x*p2.y-p2.x*p1.y) / sqrt(a*a+b*b);
}

float2 getMB(short x1, short y1, short x2, short y2)
{
    // y = mx + b
    float m = divide((y1 - y2), float(x1 - x2));
    float b = (y1 - m * x1);
    return float2(m, b);
}

// Compute barycentric coordinates (s, t, u = 1-s-t) for
// point p with respect to triangle (a, b, c)
// P = P0·s + P1·t + P2·(1-s-t)
float2 barycentric3D(float2 p, float2 a, float2 b, float2 c)
{
    float2 v0 = b - a;
    float2 v1 = c - a;
    float2 v2 = p - a;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = divide(1, (d00 * d11 - d01 * d01));
    float s = (d11 * d20 - d01 * d21) * denom;
    float t = (d00 * d21 - d01 * d20) * denom;
    //float u = 1.0f - v - w;
    return float2(s, t);
}

float2 barycentric2D(float2 p, float2 a, float2 b, float2 c)
{
    float2 v0 = b - a;
    float2 v1 = c - a;
    float2 v2 = p - a;
    float den = divide(1, cross2(v0, v1));
    return float2(
        cross2(v2, v1) * den, // (v2.x * v1.y - v1.x * v2.y) * den,
        cross2(v0, v2) * den // (v0.x * v2.y - v2.x * v0.y) * den
        );
}

float isInsidePath(
    float2 uv,
    constant short* pExtraDataIn)
{
    constant short* pExtraData = pExtraDataIn;
    short dataLength = *(pExtraData);
    pExtraData++; // skip length

    int winding = 0;
    short x1, x2, y1, y2;
    short count = 0;

    for (int i = 0; i < dataLength; i+=2)
    {
        if (count == 0)
        {
            count = pExtraData[i];
            i++;
            x1 = pExtraData[i];
            y1 = pExtraData[i+1];
            count--;
            continue;
        }

        x2 = pExtraData[i];
        y2 = pExtraData[i+1];
        count--;

        if ((y1 <= uv.y && uv.y < y2) || (y2 <= uv.y && uv.y < y1))
        {
            float px = x1;
            if (x1 != x2) // diagonal line
            {
                float2 mb = getMB(x1, y1, x2, y2);
                px = divide((uv.y - mb.y), mb.x);
            }

            if (uv.x > px)
                winding += (y1 < y2) ? -1 : 1;
        }

        x1 = x2;
        y1 = y2;
    }

    constant short* p = pExtraDataIn + dataLength;
    short cLen = *(p++);
    for (int i = 0; i < cLen; i++)
    {
        float2 p0(p[0], p[1]);
        float2 p1(p[2], p[3]);
        float2 p2(p[4], p[5]);
        float2 bc = barycentric2D(uv, p0, p1, p2);
        float s = bc.x;
        float t = bc.y;
        if (s >= 0 && t >= 0 && (s+t)<=1)
        {
            float d = (s * .5 + t);
            if ((d * d) < t)
                return winding == 0 ? 1 : 0;
        }
        p += 6;
    }

    return winding != 0 ? 1 : 0;
}

// https://chilliant.com/rgb2hsv.html
float4 HUEtoRGB(float H)
{
    H *= 6;
    float R = abs(H - 3) - 1;
    float G = 2 - abs(H - 2);
    float B = 2 - abs(H - 4);
    return saturate(float4(R,G,B, 1.0));
}

float sdfCircle(float2 uv, float2 center, float r) {
    return smoothstep(0, 1.0 / r, (r - length(uv - center)));
}

float sdSegment(float2 p, float2 a, float2 b )
{
  float2 pa = p-a, ba = b-a;
  float h = clamp(dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length(pa - ba*h);
}

float sdTriangle(float2 p, float2 p0, float2 p1, float2 p2)
{
    float2 e0 = p1-p0, e1 = p2-p1, e2 = p0-p2;
    float2 v0 = p -p0, v1 = p -p1, v2 = p -p2;
    float2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
    float2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
    float2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
    float s = sign( e0.x*e2.y - e0.y*e2.x );
    float2 d = min(min(float2(dot(pq0,pq0), s*(v0.x*e0.y-v0.y*e0.x)),
                     float2(dot(pq1,pq1), s*(v1.x*e1.y-v1.y*e1.x))),
                     float2(dot(pq2,pq2), s*(v2.x*e2.y-v2.y*e2.x)));
    return -sqrt(d.x)*sign(d.y);
}

float4 DrawTris(
    float2 uv,
    constant short* pDataIn,
    float scale)
{
    constant short* p = pDataIn;
    short contourCount = *(p++)+1;
    float s, t;
    float hue = 0;
    int winding = 0;
    short type = 0;
    short cLen = 0;
    short cLenRead = 0;

    while (contourCount)
    {
        if (cLen == 0)
        {
            type = *(p++);
            cLenRead = cLen = *(p++);
            contourCount--;
        }
        else
        {
            float2 p0(*p++, *p++);
            float2 p1(*p++, *p++);
            float2 p2(*p++, *p++);
            cLen -= 6;

            float2 bc = barycentric2D(uv, p0, p1, p2);
            s = bc.x;
            t = bc.y;
            if ((s + t) <= 1 && s >= 0 && t >= 0)
            {
                if (type == 0)
                {
                    winding++;
                    hue = float(cLen) / cLenRead;
                    p += cLen;
                    cLen = 0;
                }
                else
                {
                    float d = (s * .5 + t);
                    float dx = ((d * d) - t);
                    if (dx * type >= 0)
                    {
                        winding++;
                        hue = float(cLen) / cLenRead;
                        p += cLen;
                        cLen = 0;
                    }
                }
            }
        }
#if 0        
        short type = *(p++);
        short cLen = *(p++);
        if (type == 0)
        {
            for (int i = 0; i < cLen; i += 6)
            {
                float2 p0(*p++, *p++);
                float2 p1(*p++, *p++);
                float2 p2(*p++, *p++);
                if (isPointInsideTriangle(uv, p0, p1, p2))
                {
                    winding++;
                    //hue = float(i) / cLen;
                    p += (cLen - i - 6);
                    break;
                }
            }
        }
        else // if (type == -1 || type == 1) // quadratic bezier
        {
            for (int i = 0; i < cLen; i += 6)
            {
                float2 p0(*p++, *p++);
                float2 p1(*p++, *p++);
                float2 p2(*p++, *p++);
                if (isPointInsideTriangle(uv, p0, p1, p2))
                {
                    float2 bc = barycentric2D(uv, p0, p1, p2);
                    float s = bc.x;
                    float t = bc.y;
                    float d = (s * .5 + t);
                    float dx = (d * d) - t;
                    if (dx * type > 0)
                    {
                        winding++;
                        //hue = float(i) / cLen;
                        p += (cLen - i - 6);
                        break;
                    }
                }
            }
        }
#endif
    }

    float3 clr = HUEtoRGB(hue).rgb;
    return float4(clr, (winding % 2));// == 1 ? 1.0 : .3);
}

float sdCornerCircle(float2 p)
{
    return length(p - float2(0.0,-1.0)) - sqrt_2;
}

float sdRoundBox(float2 p, float2 b, float4 r)
{
    // select corner radius
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    // box coordinates
    float2 q = abs(p)-b + r.x;
    // distance to sides
    if (min(q.x, q.y) < 0.0) return max(q.x, q.y) - r.x;
    // rotate 45 degrees, offset by r and scale by r*sqrt(0.5) to canonical corner coordinates
    float2 uv = float2(abs(q.x - q.y), q.x + q.y - r.x ) / r.x;
    // compute distance to corner shape
    float d = sdCornerCircle(uv);
    // undo scale
    return d * r.x * sqrt_1_over_2;
}

float2x2 rotationMatrix(float angle)
{
    float sine = sin(angle), cosine = cos(angle);
    return float2x2(
        cosine, -sine,
        sine,    cosine );
}

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method.
constant float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio

float gold_noise(float2 xy, float seed){
    return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

float4 drawGlyph(float2 pos, float4 color, constant short* pGlyphDataStart, uint32_t dataOffset, float scale)
{
    short glyphType = *(pGlyphDataStart);
    pGlyphDataStart += dataOffset;
    if (glyphType == 1)
    {
        const float six = 1.0 / (6.0 * scale);
        const float twl = 1.0 / (12.0 * scale);
        pos += .5;
        // __________
        // |  |  | *|
        // | *|  |  |
        // |--|--|--|
        // |  | *|  |
        // |  |  |* |
        // |--|--|--|
        // |* |  |  |
        // |  |* |  |
        // |__|__|__|
        //
        // 1st row
        float red = 0; float green = 0; float blue = 0;
        red += DrawTris(pos + float2(six+six+twl,-(six+six+twl)), pGlyphDataStart, scale).a;
        red += DrawTris(pos + float2(-(six+twl),-(six+twl)), pGlyphDataStart, scale).a;
        // 2nd row
        float4 hue = DrawTris(pos + float2(twl,-twl), pGlyphDataStart, scale);
        green += hue.a;
        green += DrawTris(pos + float2(six+twl,twl), pGlyphDataStart, scale).a;
        // 3rd row
        blue += DrawTris(pos + float2(-(six+six+twl),(six+twl)), pGlyphDataStart, scale).a;
        blue += DrawTris(pos + float2(-(twl), (six+six+twl)), pGlyphDataStart, scale).a;
        float intensity = (red + green + blue) / 6.0f;
        // if (intensity < 0.9999)
        {
            float rg = (1 - (red + green)/4.0f);
            float rgb = (1 - intensity);
            float gb = (1 - (green+blue)/4.0f);
            intensity = 1 - (rg + rgb + gb) / 3.0;
        }

        return float4(color.rgb, color.a * intensity);
//        return float4(hue.rgb, color.a * intensity);
//        return float4(HUEtoRGB(pos.y/1000).rgb, color.a * intensity);
    }

    const float six = 1.0 / (6.0 * scale);
    const float twl = 1.0 / (12.0 * scale);
    pos += .5;
    // __________
    // |  |  | *|
    // | *|  |  |
    // |--|--|--|
    // |  | *|  |
    // |  |  |* |
    // |--|--|--|
    // |* |  |  |
    // |  |* |  |
    // |__|__|__|
    //
    // 1st row
    float red = 0; float green = 0; float blue = 0;
    red += isInsidePath(pos + float2(six+six+twl,-(six+six+twl)), pGlyphDataStart);
    red += isInsidePath(pos + float2(-(six+twl),-(six+twl)), pGlyphDataStart);
    // 2nd row
    green += isInsidePath(pos + float2(twl,-twl), pGlyphDataStart);
    green += isInsidePath(pos + float2(six+twl,twl), pGlyphDataStart);
    // 3rd row
    blue += isInsidePath(pos + float2(-(six+six+twl),(six+twl)), pGlyphDataStart);
    blue += isInsidePath(pos + float2(-(twl), (six+six+twl)), pGlyphDataStart);
    float intensity = (red + green + blue) / 6.0f;
    //if (intensity < 0.9999)
    {
        float rg = (1 - (red + green)/4.0f);
        float rgb = (1 - intensity);
        float gb = (1 - (green+blue)/4.0f);
        intensity = 1 - (rg + rgb + gb) / 3.0;
    }

    return float4(color.rgb, color.a * intensity);
}

fragment float4 fragmentShader(
    VertexOut in [[stage_in]],
    constant FragmentData& fragData [[buffer(1)]],
    constant short* pExtraData [[buffer(2)]],
    texture2d<half> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]])
{
    if (fragData.commandId == 11) // RenderCommandId::glyphs
    {
        return drawGlyph(in.textureCoordinate, in.color, pExtraData, in.dataOffset, fragData.scale);
    }
    else if (fragData.commandId == 10) // RenderCommandId::glyph
    {
        return drawGlyph(in.textureCoordinate, in.color, pExtraData, fragData.dataOffset, fragData.scale);
    }
    else if (fragData.commandId == 6 || fragData.commandId == 7) // bezier_triangle
    {
        float2 pos = in.textureCoordinate;
        float x = pos.x;
        float y = pos.y;
        float fill = y > x * x;
        if (fragData.commandId == 6) { fill = !fill; } // counter clockwise

        return float4(in.color.rgb, fill ? 0 : in.color.a);
    }
    else if (fragData.commandId == 0 || fragData.commandId == 1)
    {
        return float4(in.color.rgb, in.color.a);
    }
    else if (fragData.commandId == 2)
    {
//        constexpr sampler linearTextureSampler (
//            mag_filter::linear,
//            min_filter::linear);

        // Sample the texture to obtain a color
        const half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);

        // return the color of the texture
        return float4(colorSample) * in.color;
    }
    else if (fragData.commandId == 3) // text
    {
        constexpr sampler nearestTextureSampler (
            coord::normalized,
            mag_filter::nearest,
            min_filter::nearest);

        // Sample the texture to obtain a color
        const half4 colorSample = colorTexture.sample(nearestTextureSampler, in.textureCoordinate);

        // return the color of the texture
        return float4(in.color.rgb, colorSample.r);
    }
    else if (fragData.commandId == 4) // rounded_rectangle
    {
        float scaler = fragData.height > fragData.width ? fragData.height : fragData.width;
        float2 boxcoord = fragData.height > fragData.width
            ? float2(fragData.width / fragData.height, 1.0)
            : float2(1.0, fragData.height / fragData.width);

        // https://iquilezles.org/articles/roundedboxes/
        float2 ux = (in.textureCoordinate * 2.0 - 1.0) * float2(fragData.width, fragData.height) / scaler;
        float d = sdRoundBox(
            ux,
            boxcoord,
            float4(
            2.0 * fragData.corner.z / scaler,
            2.0 * fragData.corner.y / scaler,
            2.0 * fragData.corner.w / scaler,
            2.0 * fragData.corner.x / scaler));

        float blurStep = 2.5 / scaler;

        float4 col =  d > -0.0001 ? float4(0.0) : in.color;
        col = mix(col, float4(in.color.rgb,0.0), 1.0 - smoothstep(0.0,blurStep,abs(d)));
        return col;
    }
    else if (fragData.commandId == 5 || fragData.commandId == 12) // rounded_rectangle
    {
        float4 fillColor = in.color;
        float4 borderColor = fragData.border_color;

        float scaler = fragData.height > fragData.width ? fragData.height : fragData.width;
        float2 boxcoord = fragData.height > fragData.width 
            ? float2(fragData.width / fragData.height, 1.0)
            : float2(1.0, fragData.height / fragData.width);

        float insideWidth = fragData.width - fragData.border.x - fragData.border.z;
        float insideHeight = fragData.height - fragData.border.y - fragData.border.w;

        float2 ux = (in.textureCoordinate * 2.0 - 1.0) * float2(fragData.width, fragData.height) / scaler;
        float d = sdRoundBox(
            ux,
            boxcoord,
            float4(
            max(0.1, 2.0 * fragData.corner.z) / scaler,
            max(0.1, 2.0 * fragData.corner.y) / scaler,
            max(0.1, 2.0 * fragData.corner.w) / scaler,
            max(0.1, 2.0 * fragData.corner.x) / scaler));

        float2 ux2 = (ux) * float2(fragData.width/insideWidth, fragData.height/insideHeight)
                     + float2(
                        (fragData.border.z - fragData.border.x) / scaler,
                        (fragData.border.w - fragData.border.y) / scaler);
        float din = sdRoundBox(
            ux2,
            boxcoord,
            float4(
                max(0.1, 2.0 * (fragData.corner.z - (fragData.border.w + fragData.border.z) * .5)) / scaler,
                max(0.1, 2.0 * (fragData.corner.y - (fragData.border.y + fragData.border.z) * .5)) / scaler,
                max(0.1, 2.0 * (fragData.corner.w - (fragData.border.w + fragData.border.x) * .5)) / scaler,
                max(0.1, 2.0 * (fragData.corner.x - (fragData.border.y + fragData.border.x) * .5)) / scaler));

        float blurStep = 1.0 / scaler;

        if (fragData.commandId == 12) // rounded_rectangle_with_border || rounded_rectangle_with_border_dots
        {
            borderColor = borderColor * fmod(
                floor(in.textureCoordinate.x * fragData.width * .5) +
                floor(in.textureCoordinate.y * fragData.height * .5),
                2.0f);
        }

        float4 col =  din > -0.0001 ? borderColor : fillColor;
        col = mix(col, mix(borderColor, fillColor, .3), 1.0 - smoothstep(0.0, blurStep, abs(din)) );
        float4 colout = col;

        col =  d > -0.0001 ? float4(0) : colout;
        col = mix(col, float4(colout.rgb, 0), 1.0 - smoothstep(0.0, blurStep, abs(d)) );
        return col;
    }
    else
    {
        return float4(.5,0,.5,.5);
    }
}
// https://github.com/z4gon/metal-render-pipeline/blob/main/Game%20Engine/Shader/Metal/Structs.metal