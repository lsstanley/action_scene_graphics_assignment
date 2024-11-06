// Minimal fragment shader

#version 410 core

in vec4 fcolour;
in vec3 fposition; 
in vec3 fnormal; 
in vec3 flight_dir; 
out vec4 outputColor;

uniform uint colourmode, lightmode, is_emissive, is_attenuation;
uniform vec4 lightpos;
uniform float light_intensity; // K in attenuation equation

vec3 ambient = vec3(0.2, 0.2, 0.2);
vec3 specular_colour = vec3(1.0, 0.8, 0.6);
void main()
{
  vec4 vcolour = fcolour;
  vec3 vposition = fposition; 
  vec3 vnormal = fnormal; 
  vec3 vlight_dir = flight_dir; 
  vec4 diffuse_colour;
  vec4 diff_lighting;
  float shininess = 8.0;
  vec3 V = normalize(-vposition);
  vec3 N = normalize(vnormal); 
  vec3 L = vlight_dir - vposition;
  float distanceToLight = length(L);
  L = normalize(L);
  vec3 S = (L+V)/2;
  vec3 R = reflect(-L,N); 

  if (colourmode == 1)
    diffuse_colour = fcolour;
  else
    diffuse_colour = vec4(0.0,1.0,0,1.0);
  
  
  vec3 diffuse = max(dot(N,L),0)*diffuse_colour.xyz;
  vec3 lighting;
  
  if(lightmode == 0){ // phong specular
    lighting = pow(max(dot(R,V), 0.0), shininess) * specular_colour;
  }else if(lightmode == 1){ //blinn phong
    lighting = pow(max(dot(S,N), 0.0), shininess) * specular_colour;
  }

  float attenuation;
  if (is_attenuation != 1){
    attenuation = 1.0f;
  }else{
    attenuation = 1.0f/(light_intensity + light_intensity*distanceToLight + light_intensity*pow(distanceToLight,2));
  }

  vec3 emissive = vec3(0.0); 
  if(is_emissive == 1) emissive = vec3(1.0, 1.0, 0.8);
  //define vertex colour 
  ambient = diffuse_colour.xyz * 0.2; 
  vcolour = vec4(attenuation*(ambient + diffuse+ lighting + emissive), 1.0);
	outputColor = vcolour;
}
