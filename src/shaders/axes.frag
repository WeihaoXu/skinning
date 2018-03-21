R"zzz(#version 330 core
out vec4 fragment_color;
flat in vec4 color;
void main() {
	fragment_color = color;
}
)zzz"
