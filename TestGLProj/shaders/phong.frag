#version 440 core
in vec3 N;
in vec3 L;
in vec3 E;
in vec3 H;
in vec4 eyePosition;

// Cook-Torrence Model
#define PI 3.1415926538
uniform bool useCTM;

uniform vec4 lightPosition;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular; 
uniform vec4 lightAmbient;
uniform vec4 surfaceDiffuse;
uniform vec4 surfaceSpecular;
uniform float shininess;
uniform vec4 surfaceAmbient;
uniform vec4  surfaceEmissive;

out vec4 color;


float GetCTSpec()
{
    vec3 _N = normalize(N);
    vec3 _V = normalize(E); ////normalize(lightDirection.xyz); // vLight - vSurface
    vec3 _L = normalize(lightPosition - eyePosition).xyz; ////normalize(vertexDirection.xyz); // vEye - vSurface
    vec3 _H = normalize(H);
    
    float roughness = 0.5; // rms slope of the surface microfacets (Values: ~ 0.4 to 5.0)   // Beckerman Distribution
    float eta = 1.0; // Index of refraction (IOR) of conductor (Values: ~ 1.0 to 4.0)       // Fresnel Term
    float k_a = 1.0; // absorption coef of conductor (Values: ~ 1.0 to 4.0)                 // Fresnel Term
    
    // Beckerman distribution
    float alpha = acos(dot(_N, _H));
    float beck_D = exp( -pow(tan(alpha),2.0)/pow(roughness,2.0) ) / ( PI*pow(roughness,2.0)*pow(cos(alpha),4.0) );

    // Geometric attenuation term
    float gat_G = min( min( 1.0, 2*dot(_H,_N)*dot(_V,_N)/(dot(_V,_H)) ), 2*dot(_H,_N)*dot(_L,_N)/(dot(_V,_H)) );

    // Fresnel term (approximation)
    float theta = acos(dot(_V, _H));
    float fres_F = ( pow(eta-1.0,2) + 4*eta*pow(1-cos(theta),5) + pow(k_a,2) ) / ( pow(eta+1,2) + pow(k_a,2) );

    // Cook-Torrance Model
    float model_CT = beck_D * gat_G * fres_F / (PI * dot(_V, _N) * dot(_N, _L));
    return model_CT;
}

void main()
{
    vec3 Normal = normalize(N);
    vec3 Light  = normalize(lightPosition - eyePosition).xyz;
    vec3 Eye    = normalize(E);
    vec3 Half   = normalize(H);   

    float Kd = max(dot(Normal, Light), 0.0);
    //float Ks = pow(max(dot(Half, Normal), 0.0), 80.0);
    float Ks = pow(max(dot(reflect(-Light, Normal),Eye), 0.0), shininess);
    float Ka = 0.002;

    // Toggle c-t model
    if(useCTM)
        Ks = GetCTSpec();

    vec4 diffuse  = Kd * surfaceDiffuse * lightDiffuse;     //vec4(1,1,1,1); // white
    vec4 specular = Ks * surfaceSpecular * lightSpecular;   //vec4(0,0,0,1); // black
    vec4 ambient  = Ka * surfaceAmbient * lightAmbient;
    

    color = ambient + diffuse + specular;
}
