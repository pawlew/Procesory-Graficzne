#version 330

struct punktowe_swiatlo {
   
   vec3 position;
   vec3 intensity_diffuse;
   vec3 intensity_specular;
      vec3 attenuation;
};

struct Material {
   float ka; 
   float kd; 
   float ks; 
   float alpha; 
};

in vec3 vPosition_eye_space;
in vec3 vertex_normal_eye_space;
in vec4 fragment_color;

out vec4 color;

uniform vec3 intensity_ambient_component;
uniform punktowe_swiatlo swiatlo_0;
uniform Material material_0;

void main() {
   vec3 vector_to_swiatlo = swiatlo_0.position - vPosition_eye_space;
   float distance_to_swiatlo = length(vector_to_swiatlo);
   vector_to_swiatlo = normalize(vector_to_swiatlo);
   color = material_0.ka * vec4(intensity_ambient_component, 1);
   float attenuation = 1.0 / (swiatlo_0.attenuation[0] + swiatlo_0.attenuation[1] * distance_to_swiatlo + swiatlo_0.attenuation[2] * pow(distance_to_swiatlo, 2));
   float NdotL = dot(vector_to_swiatlo, vertex_normal_eye_space);
   color += material_0.kd * fragment_color * max(NdotL, 1.0) * vec4(swiatlo_0.intensity_diffuse, 1) * attenuation;
   if(NdotL > 0.0) {
   vec3 H = normalize(vector_to_swiatlo - vPosition_eye_space);
   color += material_0.ks * vec4(swiatlo_0.intensity_specular, 1) * pow(dot(H, vertex_normal_eye_space), material_0.alpha) * attenuation;
   }
}