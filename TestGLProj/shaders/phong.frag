#version 440 core

#define NUM_HEADLIGHTS 2
 
 in vec3 N;
 in vec3 L;
 in vec3 E;
 in vec3 H;
 in vec3 L_spot[NUM_HEADLIGHTS];
 in vec3 H_spot[NUM_HEADLIGHTS];
 in vec4 eyePosition;
 out vec4 color;
 
 
// Cook-Torrence Model
#define PI 3.1415926538
uniform bool useCTM;

uniform vec4 lightPosition;
uniform mat4 Projection;
uniform mat4 ModelView;
uniform mat4 View;                  // View matrix

uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 lightAmbient;
uniform vec4 surfaceDiffuse;
uniform vec4 surfaceSpecular;
uniform float shininess;
uniform vec4 surfaceAmbient;
uniform vec4  surfaceEmissive;

// Obsolete
uniform vec4 spotlightPosition;     // Spotlight position (2nd light)
uniform vec4 spotlightDirection;    // Spotlight direction


uniform int headlightIndex;
struct Spotlight {
    vec4 position;                  // Spotlight position (2nd light)
    vec4 direction;                 // Spotlight direction
};
uniform Spotlight headlights[NUM_HEADLIGHTS];
uniform float cutOffAngle;         // Angle used to determine cutoff
uniform float spotExponent;        // Spotlight exponent


uniform bool flashOn;
uniform bool spotlightActive;



// Prototypes
float GetCTSpec(vec3 _N, vec3 _V, vec3 _L, vec3 _H);
vec4 calcPointLight();
vec4 calcSpotlight(Spotlight spot, vec3 _H);

void main()
{
    // Calculate colors for point light
    vec4 pointLight = calcPointLight();

    // Calculate colors for spotlight
    vec4 spotlights;
    if (flashOn || spotlightActive){
        spotlights  = calcSpotlight(headlights[0], H_spot[0]);
        spotlights += calcSpotlight(headlights[1], H_spot[1]);
    }

    // Set FragColor to the sum of above values
    color = surfaceEmissive + pointLight + spotlights;
}

// Calculate K specular
float GetCTSpec(vec3 _N, vec3 _V, vec3 _L, vec3 _H)
{
    //vec3 _N = normalize(N);
    //vec3 _V = normalize(E); ////normalize(lightDirection.xyz); // vLight - vSurface
    //vec3 _L = normalize(lightPosition - eyePosition).xyz; ////normalize(vertexDirection.xyz); // vEye - vSurface
    //vec3 _H = normalize(H);
    
    float roughness = 0.3; // rms slope of the surface microfacets (Values: ~ 0.4 to 5.0)   // Beckerman Distribution
    float eta = 2.0; // Index of refraction (IOR) of conductor (Values: ~ 1.0 to 4.0)       // Fresnel Term
    float k_a = 2.0; // absorption coef of conductor (Values: ~ 1.0 to 4.0)                 // Fresnel Term
    
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

// Calculates color from point light
vec4 calcPointLight()
{
    vec3 Normal = normalize(N);
    vec3 Light  = normalize(lightPosition - eyePosition).xyz;
    vec3 Eye    = normalize(E);
    vec3 Half   = normalize(H);
	
    float Kd = max(dot(Normal, Light), 0.0);
    float Ks = pow(max(dot(reflect(-Light, Normal),Eye), 0.0), shininess);
    float Ka = 0.002;

    if (useCTM)
        Ks = GetCTSpec(Normal, Eye, Light, Half);

    vec4 diffuse  = Kd * lightDiffuse*surfaceDiffuse;
    vec4 specular = Ks * lightSpecular*surfaceSpecular;
    vec4 ambient  = Ka * lightAmbient*surfaceAmbient;

    // Return ambient, diffuse and specular
    return ambient + diffuse + specular;
}

// Calculates color from spotlight
vec4 calcSpotlight(Spotlight spot, vec3 _H)
{
    
    // Same as point light, except using spotlight position instead of light position, as well as Hspot
    vec3 Normal = normalize(N);
    vec3 Light  = normalize(spot.position - eyePosition).xyz;       // Spotlight position
    vec3 Eye    = normalize(E);
    vec3 Half   = normalize(_H);                                    // Hspot
	

    float Kd = max(dot(Normal, Light), 0.0);
    float Ks = pow(max(dot(reflect(-Light, Normal),Eye), 0.0), shininess);

    // Cook-Torrance Model
    //if (useCTM)
    //    Ks = GetCTSpec(Normal, Eye, Light, Half);

    vec4 diffuse  = Kd * lightDiffuse*surfaceDiffuse;
    vec4 specular = Ks * lightSpecular*surfaceSpecular;

    // Float holding final spotlight effect
    float spotlightEffect = 0.0;

    // Direction from spotlight to vertex
    vec3 v;

    // If stationary spotlight is activate multiply eye position by view inverse
    if (spotlightActive)
        v = normalize(spot.position - inverse(View) * eyePosition).xyz;
    else
        v= normalize(spot.position - eyePosition).xyz;

    // Calculate spotlight angle and spotlight cutoff 
    float angle = dot(v, normalize(-spot.direction).xyz);
    float cutoff = radians(cutOffAngle);

    // If angle is less than cutoff, generate calculate spotlight effect, else set to 0
    if (acos(angle) < cutoff)
        spotlightEffect = pow(max(angle, 0), spotExponent);
    else
        spotlightEffect = 0;

    // Return spotlight effect's intensity on diffuse + specular
    return spotlightEffect * ( diffuse + specular);
}

