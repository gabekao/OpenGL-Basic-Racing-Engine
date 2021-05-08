 #version 440 core
 in vec3 N;
 in vec3 L;
 in vec3 E;
 in vec3 H;
 in vec4 eyePosition;

 uniform vec4 lightPosition;
 uniform vec4 lightDiffuse;
 uniform vec4 lightSpecular; 
 uniform vec4 lightAmbient;
 uniform vec4 surfaceDiffuse;
uniform vec4 surfaceSpecular;
uniform float shininess;
uniform vec4 surfaceAmbient;
uniform vec4  surfaceEmissive;

out vec4 color2;
void main()
{
    vec3 Normal = normalize(N);
    vec3 Light  = normalize(lightPosition - eyePosition).xyz;
    vec3 Eye    = normalize(E);
    vec3 Half   = normalize(H);
    

    float Kd = max(dot(Normal, Light), 0.0);
    //float Ks = pow(max(dot(Half, Normal), 0.0), 80.0);
    float Ks = pow(max(dot(reflect(-Light, Normal),Eye), 0.0), shininess);
    float Ka = 0.0;

    vec4 diffuse  = Kd * surfaceDiffuse * lightDiffuse;     //vec4(1,1,1,1); // white
    vec4 specular = Ks * surfaceSpecular * lightSpecular;   //vec4(0,0,0,1); // black
    vec4 ambient  = Ka * surfaceAmbient * lightAmbient;

    color2 = ambient + diffuse + specular;
}
