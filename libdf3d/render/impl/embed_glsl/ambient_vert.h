"                                           \n\
\
attribute vec3 a_vertex3;                   \n\
\
uniform mat4 u_worldViewProjectionMatrix;   \n\
uniform vec4 u_globalAmbient;               \n\
uniform vec4 material_ambient;              \n\
\
varying LOWP vec4 color;                    \n\
\
void main()                                 \n\
{                                           \n\
\
    color = material_ambient * u_globalAmbient; \n\
    color.a = 1.0;                          \n\
\
    gl_Position = u_worldViewProjectionMatrix * vec4( a_vertex3, 1.0 );\n\
}                                           \n\
"