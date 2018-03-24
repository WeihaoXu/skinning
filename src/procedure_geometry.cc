#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"
#include <iostream>

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

// Compute minimum distance between two line segments. Reference: http://geomalgorithms.com/a07-_distance.html
//    Input:  start and end points of two line segments
//    Return: the shortest distance between two lines.
float line_segment_distance(const glm::vec3& line1_start, const glm::vec3& line1_end, 
							const glm::vec3& line2_start, const glm::vec3& line2_end)
{
	glm::vec3 u = line1_end - line1_start;
	glm::vec3 v = line2_end - line2_start;
	glm::vec3 w = line1_start - line2_start;

    float    a = glm::dot(u,u);         // always >= 0
    float    b = glm::dot(u,v);
    float    c = glm::dot(v,v);         // always >= 0
    float    d = glm::dot(u,w);
    float    e = glm::dot(v,w);
    float    D = a*c - b*b;        // always >= 0
    float    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
    float    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

    // compute the line parameters of the two closest points
    if (D < SMALL_NUM) { // the lines are almost parallel
        sN = 0.0;         // force using point P0 on segment S1
        sD = 1.0;         // to prevent possible division by 0.0 later
        tN = e;
        tD = c;
    }
    else {                 // get the closest points on the infinite lines
        sN = (b*e - c*d);
        tN = (a*e - b*d);
        if (sN < 0.0) {        // sc < 0 => the s=0 edge is visible
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0) {            // tc < 0 => the t=0 edge is visible
        tN = 0.0;
        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else if (-d > a)
            sN = sD;
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {      // tc > 1  => the t=1 edge is visible
        tN = tD;
        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else {
            sN = (-d +  b);
            sD = a;
        }
    }
    // finally do the division to get sc and tc
    sc = (std::abs(sN) < SMALL_NUM ? 0.0 : sN / sD);
    tc = (std::abs(tN) < SMALL_NUM ? 0.0 : tN / tD);

    // get the difference of the two closest points
    glm::vec3   dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

    return glm::length(dP);   // return the closest distance

}

// find the quaternion transform one direction into another.
// reference: https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
// It seems not working
/*
glm::fquat quaternion_between_two_directs(const glm::vec3& direct1, const glm::vec3& direct2) {
	glm::vec3 unit_dir1 = glm::normalize(direct1);
	glm::vec3 unit_dir2 = glm::normalize(direct2);
	glm::vec3 xyz = glm::cross(unit_dir1, unit_dir2);
	int w = std::sqrt(2) + glm::dot(unit_dir1, unit_dir2);
	glm::fquat result(xyz.x, xyz.y, xyz.z, w);
	result = glm::normalize(result);
	return result;
}
*/

// reference: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/#how-do-i-find-the-rotation-between-2-vectors-
glm::fquat quaternion_between_two_directs(glm::vec3 start, glm::vec3 dest){
	start = glm::normalize(start);
	dest = glm::normalize(dest);

	float cosTheta = dot(start, dest);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f){
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
			rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = glm::normalize(rotationAxis);
		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
	}

	rotationAxis = glm::cross(start, dest);

	float s = sqrt( (1+cosTheta)*2 );
	float invs = 1 / s;

	return glm::fquat(
		s * 0.5f, 
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);

}

float angle_between_two_directs_2D(glm::vec2 direct1, glm::vec2 direct2) {
	direct1 = glm::normalize(direct1);
	direct2 = glm::normalize(direct2);
	float theta = glm::acos(glm::dot(direct1, direct2));
	float angle_direct = direct1.x * direct2.y - direct1.y * direct2.x;	// cross product, determine rot direct
	if(angle_direct < 0) {
		return theta;
	}
	return -theta;

}

void printMat4(const glm::mat4& mat) {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
