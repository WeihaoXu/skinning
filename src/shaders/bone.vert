R"zzz(#version 330 core
const int kMaxBones = 128;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
in int jid;
uniform vec3 joint_trans[128];

void main() {
	mat4 mvp = projection * view * model;
	gl_Position = mvp * vec4(joint_trans[jid], 1.0);
}
)zzz"
