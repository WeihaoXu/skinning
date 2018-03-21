#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

void create_bone_mesh(LineMesh& bone_mesh)
{
	bone_mesh.vertices.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	bone_mesh.vertices.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	bone_mesh.indices.push_back(glm::uvec2(0, 1));
}

/*
 * Create Cylinder from x = -0.5 to x = 0.5
 */
void create_cylinder_mesh(LineMesh& cylinder_mesh)
{
	constexpr int kCylinderGridSizeX = 16;  // number of points in each direction
	constexpr int kCylinderGridSizeY = 3;   // number of points in each direction
	float step_x = 1.0f / (kCylinderGridSizeX - 1);
	float step_y = 1.0f / (kCylinderGridSizeY - 1);
	glm::vec3 p = glm::vec3(-0.5f, 0.0f, 0.0f);

	// Setup the vertices of the lattice.
	// Note: vertex shader is used to generate the actual cylinder
	// Extra Credit: Optionally you can use tessellation shader to draw the
	//               cylinder
	for (int i = 0; i < kCylinderGridSizeY; ++i) {
		p.x = -0.5f;
		for (int j = 0; j < kCylinderGridSizeX; ++j) {
			cylinder_mesh.vertices.push_back(glm::vec4(p, 1.0f));
			p.x += step_x;
		}
		p.y += step_y;
	}

	// Compute the indices, this is just column / row indexing for the
	// vertical line segments and linear indexing for the horizontal
	// line segments.
	for (int n = 0; n < kCylinderGridSizeX * kCylinderGridSizeY; ++n) {
		int row = n / kCylinderGridSizeX;
		int col = n % kCylinderGridSizeX;
		if (col > 0) {
			cylinder_mesh.indices.emplace_back(n - 1, n);
		}
		if (row > 0) {
			cylinder_mesh.indices.emplace_back((row - 1) * kCylinderGridSizeX + col, n);
		}
	}
}

void create_axes_mesh(LineMesh& axes_mesh)
{
	axes_mesh.vertices.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	axes_mesh.vertices.push_back(glm::vec4(2.0f, 0.0f, 0.0f, 1.0f));
	axes_mesh.vertices.push_back(glm::vec4(0.0f, 0.0f, 2.0f, 1.0f));
	axes_mesh.indices.push_back(glm::uvec2(0, 1));
	axes_mesh.indices.push_back(glm::uvec2(0, 2));
}
