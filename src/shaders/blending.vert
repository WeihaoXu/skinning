R"zzz(
#version 330 core
uniform vec4 light_position;
uniform vec3 camera_position;

uniform vec3 joint_trans[128];
uniform vec4 joint_rot[128];

uniform vec4 dual_quat0[128];
uniform vec4 dual_quat1[128];

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
	return v + 2.0 * cross(cross(v, vec3(q[1], q[2], q[3])) - q[0] * v, vec3(q[1], q[2], q[3]));
}

vec3 rotate(vec4 rot_quat, vec3 v) {
	vec4 tmp_rot_quat = rot_quat / length(rot_quat);
    vec3 q_vec = vec3(tmp_rot_quat[1], tmp_rot_quat[2], tmp_rot_quat[3]);
    return v + cross(q_vec * 2.f, cross(q_vec, v) + v * rot_quat[0]);
}

vec3 dual_quat_transform(vec3 p, vec4 dq_part0, vec4 dq_part1) {
	float norm = length(dq_part0);
	vec4 qblend_0 = dq_part0 / norm;
	vec4 qblend_e = dq_part1 / norm;

	vec3 v0 = vec3(qblend_0[1], qblend_0[2], qblend_0[3]);
	vec3 ve = vec3(qblend_e[1], qblend_e[2], qblend_e[3]);
	vec3 trans = (ve * qblend_0[0] - v0 * qblend_e[0] + cross(v0, ve)) * 2.0f;
	return rotate(qblend_0, p) + trans;
	//return qtransform(qblend_0, p) + trans;
}



/*
Point3 transform(const Point3& p ) const
{
    // As the dual quaternions may be the results from a
    // linear blending we have to normalize it :
    float norm = _quat_0 .norm();
    Quat_cu qblend_0 = _quat_0 / norm;
    Quat_cu qblend_e = _quat_e / norm;

    // Translation from the normalized dual quaternion equals :
    // 2.f * qblend_e * conjugate(qblend_0)
    Vec3 v0 = qblend_0.get_vec_part();
    Vec3 ve = qblend_e.get_vec_part();
    Vec3 trans = (ve*qblend_0.w() - v0*qblend_e.w() + v0.cross(ve)) * 2.f;

    // Rotate
    return qblend_0.rotate(p) + trans;
}
*/


void main() {
	// FIXME: Implement linear skinning here
	
	//vec3 position0 = qtransform(joint_rot[jid0], vector_from_joint0) + joint_trans[jid0];
	//vec3 position1 = qtransform(joint_rot[jid1], vector_from_joint1) + joint_trans[jid1];
	// gl_Position = vec4(w0 * position0 + (1.0 - w0) * position1, 1.0);

	vec4 dual_quat0_avg = w0 * dual_quat0[jid0] + (1 - w0) * dual_quat0[jid1];
	vec4 dual_quat1_avg = w0 * dual_quat1[jid0] + (1 - w0) * dual_quat1[jid1];
	vec3 dual_pos = dual_quat_transform(vec3(vert), dual_quat0_avg, dual_quat1_avg);
	

	gl_Position = vec4(dual_pos, 1.0);

	
	if(dual_pos.x == 0 && dual_pos.y == 0 && dual_pos.z == 0) {
		gl_Position = vert;
	}
	
	

	// gl_Position = vert;
	vs_normal = normal;
	vs_light_direction = light_position - gl_Position;
	vs_camera_direction = vec4(camera_position, 1.0) - gl_Position;
	vs_uv = uv;
}
)zzz"
