#version 330 core

/*default camera matrices. do not modify.*/
layout(std140) uniform camera
{
    mat4 projection;	/*camera's projection matrix*/
    mat4 view;			/*camera's view matrix*/
    mat4 pvm;			/*camera's projection*view*model matrix*/
    mat4 ortho;			/*camera's ortho projection matrix*/
    vec4 position;		/*camera's position in world space*/
};

/* set light ubo. do not modify.*/
struct light
{
	ivec4 att; 
	vec4 pos; // position
	vec4 dir;
	vec4 amb; // ambient intensity
	vec4 dif; // diffuse intensity
	vec4 spec; // specular intensity
	vec4 atten;
	vec4 r;
};
layout(std140) uniform lights
{
	vec4 amb;
	ivec4 lt_att; // lt_att[0] = number of lights
	light lt[4];
};

/*input variables*/
in vec3 vtx_normal; // vtx normal in world space
in vec3 vtx_position; // vtx position in world space
in vec3 vtx_model_position; // vtx position in model space
in vec4 vtx_color;
in vec2 vtx_uv;
in vec3 vtx_tangent;

uniform vec3 ka;            /* object material ambient */
uniform vec3 kd;            /* object material diffuse */
uniform vec3 ks;            /* object material specular */
uniform float shininess;    /* object material shininess */

uniform sampler2D tex_color;   /* texture sampler for color */
uniform sampler2D tex_normal;   /* texture sampler for normal vector */

/*output variables*/
out vec4 frag_color;

vec4 shading_texture_with_color() 
{
    vec4 color = vec4(0.0);     //// we set the default color to be black, update its value in your implementation below
    vec2 uv = vtx_uv;           //// the uv coordinates you need to read texture values

    /* your implementation starts */
    color = texture(tex_color, uv);
    /* your implementation ends */

    return color;
}

vec3 shading_texture_with_phong(light li, vec3 e, vec3 p, vec3 s, vec3 n)
{
    vec4 color = vec4(0.0);
    vec3 tex_color = shading_texture_with_color().rgb;
    
    /* your implementation starts */
    vec3 l = normalize(s - p); //Light Reflection
    vec3 v = normalize(e - p); //View Direction
    vec3 r = reflect(-l, n); //Reflection Direction
    vec3 ambient = li.amb.rgb * ka;
    vec3 specular = ks * li.spec.rgb * pow(max(dot(v, r), 0.0), shininess); //Specular Component
    vec3 diffuse = tex_color * kd * li.dif.rgb * max(dot(n, l), 0.0); //Diffuse Shading using texture color
    /* your implementation ends */

    return vec3(ambient + diffuse + specular);
}

vec3 calc_bitangent(vec3 N, vec3 T) 
{
    vec3 B = vec3(0.0);     //// the bitangent vector you need to calculate

    /* your implementation starts */
    B = normalize(cross(N, T));
    /* your implementation ends */
    
    return B;
}

mat3 calc_TBN_matrix(vec3 T, vec3 B, vec3 N) 
{
    mat3 TBN = mat3(0.0);   //// the TBN matrix you need to calculate

    /* your implementation starts */
    TBN = mat3(T, B, N); //T, B, and N vectors form the columns of the TBN matrix
    /* your implementation ends */

    return TBN;
}

vec3 read_normal_texture()
{
    vec3 normal = texture(tex_normal, vtx_uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    return normal;
}

vec3 calc_perturbed_normal(mat3 TBN, vec3 normal) 
{
    vec3 perturbed_normal = vec3(0.0);
    
    /* your implementation starts */
    perturbed_normal = normalize(TBN * normal); //Apply TBN matrix to the normal matrix
    /* your implementation ends */
    
    return perturbed_normal;
}

vec4 shading_texture_with_normal_mapping()
{
    vec3 e = position.xyz;              //// eye position
    vec3 p = vtx_position;              //// surface position

    vec3 N = normalize(vtx_normal);     //// normal vector
    vec3 T = normalize(vtx_tangent);    //// tangent vector

    vec3 perturbed_normal = vec3(0.0);  //// perturbed normal

    /* your implementation starts */
    vec3 B = calc_bitangent(N, T);
    mat3 TBN = calc_TBN_matrix(T, B, N);
    //Read the normal from the normal map and calculate the perturbed normal
    vec3 normal_map = read_normal_texture();
    perturbed_normal = calc_perturbed_normal(TBN, normal_map);
    /* your implementation ends */

    vec3 final_color = vec3(0.0);
    for (int i = 0; i < lt_att[0]; i++) {
        final_color += shading_texture_with_phong(lt[i], e, p, lt[i].pos.rgb, perturbed_normal);
    }
    return vec4(final_color, 1.0);
}

void main()
{
    frag_color = shading_texture_with_normal_mapping();
}