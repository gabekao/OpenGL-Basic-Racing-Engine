#version 440 core
out vec3 N;
out vec3 L;
out vec3 E;
out vec3 H;
out vec3 Lspot;
out vec3 Hspot;
out vec4 eyePosition;

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform vec4 lightPosition;
uniform vec4 spotlightPosition;     // Spotlight position (2nd light)
uniform mat4 Projection;
uniform mat4 ModelView;

 uniform vec4 lightDiffuse;
 uniform vec4 lightSpecular; 
 uniform vec4 lightAmbient;
 uniform vec4 surfaceDiffuse;
 uniform vec4 surfaceSpecular;
 uniform float shininess;
 uniform vec4 surfaceAmbient;
 uniform vec4  surfaceEmissive;

void main()
{
    gl_Position = Projection * ModelView * vec4(vertexPosition, 1.0);

    eyePosition = ModelView * vec4(vertexPosition, 1.0);

    // Point light values
    vec4 eyeLightPos = lightPosition;

	N = normalize(ModelView * vec4(vertexNormal,0.0)).xyz;
    L = normalize(eyeLightPos.xyz - eyePosition.xyz);
    E = -normalize(eyePosition.xyz);
    H = normalize(L + E);

    // Spotlight values
	vec4 eyeSpotPosition = spotlightPosition;
    Lspot = normalize(eyeSpotPosition.xyz - eyePosition.xyz);
    Hspot = normalize(Lspot + E);
}

