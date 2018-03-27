/*
 * Reference: The QuatTrans2UDQ and DQToMatrix methods are cited from https://www.cs.utah.edu/~ladislav/dq/index.html.
 */

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
	return v + 2.0 * cross(cross(v, vec3(q[0], q[1], q[2])) - q[3] * v, vec3(q[0], q[1], q[2]));
}

struct DualQuat {
	vec4 dual_quat_0;
	vec4 dual_quat_1;
};

DualQuat QuatTrans2UDQ(vec4 q0, vec3 t)
{
	q0 = vec4(q0.w, q0.x, q0.y, q0.z);

	DualQuat dq;
	// non-dual part (just copy q0):
	for (int i=0; i<4; i++) dq.dual_quat_0[i] = q0[i];
	// dual part:
	dq.dual_quat_1[0] = -0.5*(t[0]*q0[1] + t[1]*q0[2] + t[2]*q0[3]);
	dq.dual_quat_1[1] = 0.5*( t[0]*q0[0] + t[1]*q0[3] - t[2]*q0[2]);
	dq.dual_quat_1[2] = 0.5*(-t[0]*q0[3] + t[1]*q0[0] + t[2]*q0[1]);
	dq.dual_quat_1[3] = 0.5*( t[0]*q0[2] - t[1]*q0[1] + t[2]*q0[0]);
	return dq;
}


mat3x4 DQToMatrix(vec4 Qn, vec4 Qd)
{	
	mat3x4 M;
	float len2 = dot(Qn, Qn);
	float w = Qn.x, x = Qn.y, y = Qn.z, z = Qn.w;
	float t0 = Qd.x, t1 = Qd.y, t2 = Qd.z, t3 = Qd.w;
		
	M[0][0] = w*w + x*x - y*y - z*z; M[0][1] = 2*x*y - 2*w*z; M[0][2] = 2*x*z + 2*w*y;
	M[1][0] = 2*x*y + 2*w*z; M[1][1] = w*w + y*y - x*x - z*z; M[1][2] = 2*y*z - 2*w*x; 
	M[2][0] = 2*x*z - 2*w*y; M[2][1] = 2*y*z + 2*w*x; M[2][2] = w*w + z*z - x*x - y*y;
	
	M[0][3] = -2*t0*x + 2*w*t1 - 2*t2*z + 2*y*t3;
	M[1][3] = -2*t0*y + 2*t1*z - 2*x*t3 + 2*w*t2;
	M[2][3] = -2*t0*z + 2*x*t2 + 2*w*t3 - 2*t1*y;
	
	M /= (len2+ 0.0001f);
	
	return M;	
}

// basic dual quaternion skinning:
vec4 dqs(vec4 position, vec4 blendDQ_0, vec4 blendDQ_1)
{
	mat3x4 M = DQToMatrix(blendDQ_0, blendDQ_1);
	vec3 out_position = position * M;
	out_position.y;
	return vec4(out_position, 1.0);
}


void main() {
	// FIXME: Implement linear skinning here
	
	// vec3 position0 = qtransform(joint_rot[jid0], vector_from_joint0) + joint_trans[jid0];
	// vec3 position1 = qtransform(joint_rot[jid1], vector_from_joint1) + joint_trans[jid1];
	// gl_Position = vec4(w0 * position0 + (1.0 - w0) * position1, 1.0);

	DualQuat dq0 = QuatTrans2UDQ(joint_rot[jid0], joint_trans[jid0]);
	DualQuat dq1 = QuatTrans2UDQ(joint_rot[jid1], joint_trans[jid1]);
	
	vec4 pos0 = dqs(vec4(vector_from_joint0, 1.0), dq0.dual_quat_0, dq0.dual_quat_1);
	vec4 pos1 = dqs(vec4(vector_from_joint1, 1.0), dq1.dual_quat_0, dq1.dual_quat_1);
	gl_Position = w0 * pos0 + (1 - w0) * pos1;

	// gl_Position = vert;
	vs_normal = normal;
	vs_light_direction = light_position - gl_Position;
	vs_camera_direction = vec4(camera_position, 1.0) - gl_Position;
	vs_uv = uv;
}
)zzz"
