attribute vec3 a_position;

uniform mat4 mvp;
uniform float pointSize;
uniform vec3 color;

varying vec3 v_color;

void main(void)
{
    gl_Position =  mvp * vec4(a_position, 1.0);
    gl_PointSize = pointSize;
    v_color = color;
}
