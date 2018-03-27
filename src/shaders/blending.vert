R"zzz(
#version 330 core
uniform vec4 light_position;
uniform vec3 camera_position;

uniform vec3 joint_trans[128];
uniform vec4 joint_rot[128];

in int jid0;
in int jid1;
in float w0;
in vec3 vector_from_joint0;
in vec3 vector_from_joint1;
in vec4 normal;
in vec2 uv;
in vec4 vert;

out vec4 vs_light_direction;
out vec4 vs_normal;
out vec2 vs_uv;
out vec4 vs_camera_direction;

vec3 qtransform(vec4 q, vec3 v) {
	return v + 2.0 * cross(cross(v, q.xyz) - q.w*v, q.xyz);
}




void main() {
	// FIXME: Implement linear skinning here
	
	vec3 position0 = qtransform(joint_rot[jid0], vector_from_joint0) + joint_trans[jid0];
	vec3 position1 = qtransform(joint_rot[jid1], vector_from_joint1) + joint_trans[jid1];
	gl_Position = vec4(w0 * position0 + (1.0 - w0) * position1, 1.0);

	// gl_Position = vert;
	vs_normal = normal;
	vs_light_direction = light_position - gl_Position;
	vs_camera_direction = vec4(camera_position, 1.0) - gl_Position;
	vs_uv = uv;
}
)zzz"
