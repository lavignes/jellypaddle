#version 130

in vec2 coord;
in vec3 color;
out vec4 frag_color;

void main() {
  gl_Position = vec4((coord.x / 400.0) - 1.0, (coord.y / 300.0) - 1.0, 1.0, 1.0);
  frag_color = vec4(color, 1.0);
}
