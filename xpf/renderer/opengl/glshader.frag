#version 330 core
in vec4 frag_color;
in vec2 frag_texture_coord;
out vec4 FragColor;

uniform int u_command_id;
uniform vec2 u_size; // width & height of quad
uniform vec4 u_corner_radius;
uniform vec4 u_border_thickness;
uniform vec4 u_border_color;
uniform sampler2D texture0;

float sdCornerCircle(vec2 p)
{
    return length(p - vec2(0.0,-1.0)) - sqrt(2.0);
}

float sdRoundBox(vec2 p, vec2 b, vec4 r)
{
    // select corner radius
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    // box coordinates
    vec2 q = abs(p)-b + r.x;
    // distance to sides
    if (min(q.x, q.y) < 0.0) return max(q.x, q.y) - r.x;
    // rotate 45 degrees, offset by r and scale by r*sqrt(0.5) to canonical corner coordinates
    vec2 uv = vec2(abs(q.x - q.y), q.x + q.y- r.x ) / r.x;
    // compute distance to corner shape
    float d= sdCornerCircle(uv);
    // undo scale
    return d * r.x * sqrt(0.5);
}

float rect_corner(float width, const float height, float cr, float x, float y) {
    float s0 = 0.95;
    float s1 = .99;

    float cr2 = cr * cr;
    float u = x - (width - cr);
    float v = y - (height - cr);

    float cx = (u * abs(u) + v * abs(v))/cr2;
    return smoothstep(s0, s1, cx);
}

float rect_round_corners(
    float x, float y,
    float width, float height,
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
    const float x, const float y,
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

void main() {
    if (u_command_id == 0 || u_command_id == 1)
        FragColor = frag_color;
    else if (u_command_id == 2) // multiplicative tint
        FragColor = texture(texture0, frag_texture_coord) * frag_color;
    else if (u_command_id == 3) // text
        FragColor = vec4(frag_color.rgb, texture(texture0, frag_texture_coord).r);
    else if (u_command_id == 4) // rounded_rectangle
    {
        float scaler = u_size.y > u_size.x ? u_size.y : u_size.x;
        vec2 boxcoord = u_size.y > u_size.x 
            ? vec2(u_size.x / u_size.y, 1.0)
            : vec2(1.0, u_size.y / u_size.x);

        // https://iquilezles.org/articles/roundedboxes/
        vec2 ux = (frag_texture_coord * 2.0 - 1.0) * vec2(u_size.x, u_size.y) / scaler;
        float d = sdRoundBox(
            ux,
            boxcoord,
            vec4(
            2.0 * u_corner_radius.z / scaler,
            2.0 * u_corner_radius.y / scaler,
            2.0 * u_corner_radius.w / scaler,
            2.0 * u_corner_radius.x / scaler));

        float blurStep = 2.5 / scaler;

        vec4 col =  d > -0.0001 ? vec4(0.0) : frag_color;
        FragColor = mix( col, vec4(frag_color.rgb,0.0), 1.0-smoothstep(0.0, blurStep, abs(d)) );
    }
    else if (u_command_id == 5 || u_command_id == 12) // rounded_rectangle_with_border || rounded_rectangle_with_border_dots
    {
        vec4 borderColor = u_border_color;
        vec4 fillColor = frag_color;

        float scaler = u_size.y > u_size.x ? u_size.y : u_size.x;
        vec2 boxcoord = u_size.y > u_size.x 
            ? vec2(u_size.x / u_size.y, 1.0)
            : vec2(1.0, u_size.y / u_size.x);

        float insideWidth = u_size.x - u_border_thickness.x - u_border_thickness.z;
        float insideHeight = u_size.y - u_border_thickness.y - u_border_thickness.w;
        float leftborder = u_border_thickness.x / u_size.x;
        float topborder = (u_border_thickness.y - .5) / scaler;

        vec2 ux = (frag_texture_coord * 2.0 - 1.0) * vec2(u_size.x, u_size.y) / scaler;
        float d = sdRoundBox(
            ux,
            boxcoord,
            vec4(
            max(0.1, 2.0 * u_corner_radius.z) / scaler,
            max(0.1, 2.0 * u_corner_radius.y) / scaler,
            max(0.1, 2.0 * u_corner_radius.w) / scaler,
            max(0.1, 2.0 * u_corner_radius.x) / scaler));

        vec2 ux2 = (ux) * vec2(u_size.x/insideWidth, u_size.y/insideHeight)
                     + vec2(
                        (u_border_thickness.z - u_border_thickness.x) / scaler,
                        (u_border_thickness.w - u_border_thickness.y) / scaler);
        float din = sdRoundBox(
            ux2,
            boxcoord,
            vec4(
                max(0.1, 2.0 * (u_corner_radius.z - (u_border_thickness.w + u_border_thickness.z) * 0.5)) / scaler,
                max(0.1, 2.0 * (u_corner_radius.y - (u_border_thickness.y + u_border_thickness.z) * 0.5)) / scaler,
                max(0.1, 2.0 * (u_corner_radius.w - (u_border_thickness.w + u_border_thickness.x) * 0.5)) / scaler,
                max(0.1, 2.0 * (u_corner_radius.x - (u_border_thickness.y + u_border_thickness.x) * 0.5)) / scaler));

        float blurStep = 0.25 / scaler;

        if (u_command_id == 12) // rounded_rectangle_with_border_dots
        {
            borderColor = borderColor * mod(
                floor(frag_texture_coord.x * u_size.x * .5) +
                floor(frag_texture_coord.y * u_size.y * .5),
                2.0f);
        }

        vec4 col =  din > -0.0001 ? borderColor : fillColor;
        col = mix(col, mix(borderColor, fillColor, .3), 1.0-smoothstep(0.0, blurStep ,abs(din)) );
        vec4 colout = col;

        col =  d > 0 ? vec4(0) : colout;
        FragColor = mix(col, vec4(colout.rgb, 0), 1.0-smoothstep(0.0, blurStep, abs(d)) );
    }
    else if (u_command_id == 40) // rounded_rectangle
    {
        float width = u_size.x * .5;
        float height = u_size.y * .5;

        vec2 pos = frag_texture_coord;
        float x = (pos.x * 2 - 1) * width;
        float y = (pos.y * 2 - 1) * height;

        float alpha = rect_round_corners(
            x, y, width, height,
            u_corner_radius.x,
            u_corner_radius.y,
            u_corner_radius.z,
            u_corner_radius.w);

        FragColor = vec4(frag_color.rgb, (1.0 - alpha) * frag_color.a);
    }
    else if (u_command_id == 50) // rounded_rectangle_with_border
    {
        float width = u_size.x * .5;
        float height = u_size.y * .5;

        vec2 pos =frag_texture_coord;
        float x = (pos.x * 2 - 1) * width;
        float y = (pos.y * 2 - 1) * height;

        // outter corners
        float cr_topleft     = u_corner_radius.x;
        float cr_topright    = u_corner_radius.y;
        float cr_bottomright = u_corner_radius.z;
        float cr_bottomleft  = u_corner_radius.w;

        float alpha = rect_round_corners(
            x, y, width, height,
            cr_topleft,
            cr_topright,
            cr_bottomright,
            cr_bottomleft);

        // inner corners
        float border_left = u_border_thickness.x;
        float border_top = u_border_thickness.y;
        float border_right = u_border_thickness.z;
        float border_bottom = u_border_thickness.w;
        float cr_inner = (cr_topleft + cr_topright + cr_bottomright + cr_bottomleft) * .125; // half of avg outter corner radius

        float alpha2 = rect_round_corners_inner(
            x, y, width, height,
            cr_inner,
            border_left - cr_inner,
            border_top - cr_inner,
            border_right - cr_inner,
            border_bottom - cr_inner);

        float a = (1-step(x, (-width +  border_left))) * step(x, (width -  border_right));
        float b = (1-step(y, (-height + border_top)))  * step(y, (height - border_bottom));

        vec4 borderColor = u_border_color;
        vec4 fillColor = frag_color;

        float fillMult = ((a * b)*(1-alpha2));
        float borderMult = (1-(a * b)*(1-alpha2))*(1-alpha);

        FragColor = fillColor * fillMult + borderColor * borderMult;
    }
    else
    {
        FragColor = vec4(.5,0,.5,.5);
    }
}
