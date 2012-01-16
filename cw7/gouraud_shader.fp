#version 330

struct punktowe_swiatlo{

  vec3 position;
  vec3 intensity_diffuse;
  vec3 intensity_specular;
  vec3 attenuation;
};

struct Material {
  float r_ambient; 
  float r_diffuse; 
  float r_spectacular; 
  float alpha; 
};

in vec3 vPosition_eye_space;
in vec3 vertex_normal_eye_space;
in vec2 texture_coordinates;

out vec4 color;

uniform vec3 intensity_ambient_component;
uniform sampler2D texture_unit;
uniform punktowe_swiatlo swiatlo_0;
uniform Material material_0;

void main() {
  vec3 vector_to_swiatlo = swiatlo_0.position - vPosition_eye_space;
  float distance_to_swiatlo = length(vector_to_swiatlo);
  vector_to_swiatlo = normalize(vector_to_swiatlo);
  color = material_0.r_ambient * vec4(intensity_ambient_component, 1);
  float attenuation = 2.0 / (swiatlo_0.attenuation[0] + swiatlo_0.attenuation[1] * distance_to_swiatlo + swiatlo_0.attenuation[2] * pow(distance_to_swiatlo, 2));
  float NdotL = dot(vector_to_swiatlo, vertex_normal_eye_space);
  color += material_0.r_diffuse * texture2D(texture_unit, texture_coordinates) * max(NdotL, 0.0) * vec4(swiatlo_0.intensity_diffuse, 1) * attenuation;
  if(NdotL > 0.0) {
     vec3 H = normalize(vector_to_swiatlo - vPosition_eye_space);
     color += material_0.r_spectacular * vec4(swiatlo_0.intensity_specular, 1) * pow(dot(H, vertex_normal_eye_space), material_0.alpha) * attenuation;
  }
}
