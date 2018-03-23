R"zzz(#version 330 core
uniform mat4 bone_transform; // transform the cylinder to the correct configuration
const float kPi = 3.1415926535897932384626433832795;
const float radius = 0.1;	// radius of cylinder
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
in vec4 vertex_position;

// FIXME: Implement your vertex shader for cylinders
// Note: you need call sin/cos to transform the input mesh to a cylinder

void main() {
	float theta = vertex_position.x * 2 * kPi;
	vec4 position = vec4(radius * cos(theta), vertex_position.y, radius * sin(theta), 1);
	mat4 mvp = projection * view * model;
	gl_Position = mvp * bone_transform * position;
}



)zzz"
