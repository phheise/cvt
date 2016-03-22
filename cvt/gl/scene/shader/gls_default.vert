
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 NORMM;

in vec3 in_Vertex;
in vec3 in_Normal;
in vec2 in_TexCoord;

out vec3 vtx_Normal;
out vec2 vtx_TexCoord;
out vec4 vtx_Position;

void main()
{
    vtx_Normal	 = normalize( NORMM * in_Normal );
    vtx_TexCoord = in_TexCoord;
    vtx_Position = MV  * vec4( in_Vertex, 1.0 );
    gl_Position	 = MVP * vec4( in_Vertex, 1.0 );
}
