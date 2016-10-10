attribute vec2 a_position;

uniform vec3 color;
uniform float pointSize;
uniform float screen_halfWidth;
uniform float screen_halfHeight;

varying vec3 v_color;

void main(void)
{
    vec2 pos = a_position;
    pos.x = pos.x / screen_halfWidth - 1.0;
    pos.y = 1.0 - pos.y / screen_halfHeight;

    gl_Position =  vec4(pos, 0.0, 1.0);
    gl_PointSize = pointSize;
    v_color = color;
}
