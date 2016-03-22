struct Material {
    vec4 Ka;
    vec4 Kd;
    vec4 Ks;
    float shininess;
    sampler2D ambientmap;
    sampler2D diffusemap;
    sampler2D specularmap;
    sampler2D normalmap;
};

uniform Material material;

struct Light {
    vec4 position;
    
    vec4 La;
    vec4 Ld;
    vec4 Ls;
    vec3 attenuation;
};

uniform int   numlights;
uniform Light light[ MAX_LIGHTS ];

in vec2 vtx_TexCoord;
in vec3 vtx_Normal;
in vec4 vtx_Position;

out vec4 result;

vec4 PhongLightModel( int l, vec4 position, vec3 norm )
{
    vec3 s = normalize( vec3( light[ l ].position - position ) );
    vec3 v = normalize( -position.xyz );
    vec3 r = normalize( -reflect( s, norm ) );

    vec4 Ka, Kd, Ks;
    float shininess;

#ifdef GLSMATERIAL_AMBIENTMAP
    Ka = texture( material.ambientmap, vtx_TexCoord );
#else
    Ka = material.Ka;
#endif

#ifdef GLSMATERIAL_DIFFUSEMAP
   Kd = texture( material.diffusemap, vtx_TexCoord );
#else
   Kd = material.Kd;
#endif

#ifdef GLSMATERIAL_SPECULARMAP
   Ks = texture( material.specularmap, vtx_TexCoord );
   shininess = Ks.a;
#else
   Ks = material.Ks;
   shininess = material.shininess;
#endif

    vec4 ambient = light[ l ].La * Ka;
    ambient.a = 0.0;

    float sDotN = max( dot( s, norm ) , 0.0 );
    vec4 diffuse = vec4( clamp( light[ l ].Ld.rgb * Kd.rgb * sDotN, 0.0, 1.0 ), Kd.a );

    vec4 spec = vec4( 0.0, 0.0, 0.0, Ks.a );
    if( sDotN > 0.0 )
        spec = clamp( light[ l ].Ls * Ks * pow( max( dot( r , v ), 0.0 ), shininess ), 0.0, 1.0 );
    spec.a = 0.0;

    return ambient + diffuse + spec;
}


void main( void )
{
    result = vec4( 0.0 );

    vec3 normal;

#ifdef GLSMATERIAL_NORMALMAP
    normal = vtx_Normal; //texture( material.normalmap, vtx_TexCoord ).xyz;
#else
    normal = vtx_Normal;
#endif

    for( int i = 0; i < numlights; i++ ) {
       result += PhongLightModel( i, vtx_Position, normal );
    }

} 
