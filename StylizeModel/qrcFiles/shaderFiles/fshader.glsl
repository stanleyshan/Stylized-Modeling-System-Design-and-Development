#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

//uniform vec4 color_ambient;
//uniform vec4 color_diffuse;
//uniform vec4 color_specular;
//uniform vec4 color;
//uniform float shininess;
uniform sampler2D texture;
uniform vec3 light_position;
//uniform vec3 eye_direction;
uniform bool b_face;

varying vec3 v_position;
varying vec3 v_normal;
varying vec2 v_texcoord;

void main()
{
    // Set fragment color from texture
    if(!b_face)
    {
//        vec3 light_direction = normalize(light_position - v_position);
//        vec3 normal = normalize(v_normal);
//        vec3 half_vector = normalize(normalize(light_direction) + normalize(eye_direction));
//        float diffuse = max(0.0, dot(normal, light_direction));
//        float specular = pow(max(0.0, dot(v_normal, half_vector)), shininess);
//        vec4 color = vec4(texture2D(texture, v_texcoord).xyz, 1.0);
//        gl_FragColor = min(color*color_ambient, vec4(1.0)); + diffuse*color_diffuse + specular*color_specular;
        vec3 L = normalize(light_position - v_position);
        float NL = max(dot(normalize(v_normal), L), 0.0);
        vec3 color = texture2D(texture, v_texcoord).xyz;
        vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);
        gl_FragColor = vec4(col, 1.0);
    }
    else
    {
        vec3 color = texture2D(texture, v_texcoord).rgb;
        gl_FragColor = vec4(color, 1.0);
    }
}

