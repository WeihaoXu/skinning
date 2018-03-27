#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>


#define SMALL_NUM 0.00000001f	// used to avoid division overflow

struct LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);
void create_bone_mesh(LineMesh& bone_mesh);
void create_cylinder_mesh(LineMesh& cylinder_mesh);
void create_axes_mesh(LineMesh& axes_mesh);

float line_segment_distance(const glm::vec3& line1_start, const glm::vec3& line1_end, 
							const glm::vec3& line2_start, const glm::vec3& line2_end);
glm::fquat quaternion_between_two_directs(glm::vec3 from, glm::vec3 to);
float angle_between_two_directs_2D(glm::vec2 direct1, glm::vec2 direct2);
void printMat4(const glm::mat4& mat);


#endif
