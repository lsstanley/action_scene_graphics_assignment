// Starter vertex shader for lab3
// THe goal is to update this shader to implement Gourand shading
// which is per-vertex lighting

#version 410

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal_in;


// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode, lightmode, is_emissive, is_attenuation;
uniform vec4 lightpos;
uniform float light_intensity; // K in attenuation equation 
// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
out vec3 fposition; 
out vec3 fnormal;
out vec3 flight_dir;
vec3 ambient = vec3(0.2, 0.2, 0.2);

void main()
{
	vec4 diffuse_colour;
	vec4 diff_lighting; 
	vec4 position_h = vec4(position, 1.0);
	mat4 model_view = view * model;

	vec3 light_dir = (lightpos.xyz);
  flight_dir = light_dir;

	// if (colourmode == 1)
	// 	diffuse_colour = colour;
	// else
	// 	diffuse_colour = vec4(0.0, 1.0, 0, 1.0);
	//

	//diffusion variables
	mat3 n_matrix = transpose(inverse(mat3(model_view)));
	vec3 normal = (n_matrix * normal_in);
  fnormal = normalize(normal);  // normal_view sent to fragment shader
	vec3 light_dir_normalised = normalize(light_dir);


	//phong specular variables
	//vec3 specular_colour = vec3(1.0, 0.8, 0.6);
	float shininess = 8.0;
	vec4 P = model_view*position_h;
  fposition = P.xyz; 
	vec3 V = normalize(-P.xyz);
	vec3 N = normalize(normal);
	vec3 L = light_dir - P.xyz;
  float distanceToLight = length(L); // for attenuation
	L = normalize(L);
	vec3 S = (L+V)/2;
	vec3 R = reflect(-L, N);

	vec3 diffuse = max(dot(N,L),0) * diffuse_colour.xyz;
	vec3 lighting;
  
  fcolour = colour;
	// if (lightmode == 0){  //phong specular
	// 	lighting = pow(max(dot(R,V), 0.0), shininess) * specular_colour;
	// }else if(lightmode == 1){ // blinn phong
	// 	lighting = pow(max(dot(S,N), 0.0), shininess) * specular_colour;
	// }
  
  // float attenuation;
  // if (is_attenuation != 1){
  //   attenuation = 1.0f;
  // }else{ 
  //   attenuation = 1.0f/(light_intensity + light_intensity*distanceToLight + 
  //     light_intensity*pow(distanceToLight,2));
  // }

	// vec3 emissive = vec3(0.0);
	// if(is_emissive == 1) emissive = vec3(1.0,1.0,0.8);
	// // Define the vertex colour
	// ambient = diffuse_colour.xyz * 0.2; 
	// fcolour = vec4(attenuation*(ambient + diffuse + lighting + emissive),1.0) ;

	// Define the vertex position
	gl_Position = projection * view * model * position_h;
}
