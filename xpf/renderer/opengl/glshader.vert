#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTextureCoord;
uniform mat4 u_projection;
uniform mat4 u_view_matrix;
uniform mat4 u_transform;

out vec4 frag_color;
out vec2 frag_texture_coord;

void main() {
    frag_color = aColor;
    frag_texture_coord = aTextureCoord;
    gl_Position = u_projection * u_view_matrix * u_transform * vec4(aPos.x, aPos.y, 0.0, 1.0);
}
