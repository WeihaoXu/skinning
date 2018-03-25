#ifndef BONE_GEOMETRY_H
#define BONE_GEOMETRY_H

#include <ostream>
#include <vector>
#include <map>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <mmdadapter.h>

struct BoundingBox {
	BoundingBox()
		: min(glm::vec3(-std::numeric_limits<float>::max())),
		max(glm::vec3(std::numeric_limits<float>::max())) {}
	glm::vec3 min;
	glm::vec3 max;
};

struct Joint {
	Joint()
		: joint_index(-1),
		  parent_index(-1),
		  position(glm::vec3(0.0f)),
		  init_position(glm::vec3(0.0f))
	{
	}
	Joint(int id, glm::vec3 wcoord, int parent)
		: joint_index(id),
		  parent_index(parent),
		  position(wcoord),
		  init_position(wcoord)
	{
	}

	int joint_index;
	int parent_index;
	glm::vec3 position;             // position of the joint
	glm::fquat orientation;         // rotation w.r.t. initial configuration
	glm::fquat rel_orientation;     // rotation w.r.t. it's parent. Note: it's not necessary to align the local coord sys. with the bone direction. So it could be initialized as identity.
	glm::vec3 init_position;        // initial position of this joint
	glm::vec3 init_offset;
	std::vector<int> children;
};

struct LineMesh {
	std::vector<glm::vec4> vertices;
	std::vector<glm::uvec2> indices;
};

struct Skeleton {
	std::vector<Joint> joints;

	std::vector<glm::vec3> joint_trans; // cache for uniforms	// position
	std::vector<glm::fquat> joint_rot; // cache for uniforms	// orientation

	void refreshCache();
	const glm::vec3* collectJointTrans() const;
	const glm::fquat* collectJointRot() const;

	// FIXME: create skeleton and bone data structures
	std::vector<glm::mat4> bone_transforms;	
	std::vector<glm::mat4> d_matrices;
	std::vector<glm::mat4> u_matrices;

	// void calculate_bone_transforms();
	const glm::mat4 getBoneTransform(int joint_index) const;

};

struct Mesh {
	Mesh();
	~Mesh();
	std::vector<glm::vec4> vertices;
	/*
	 * Static per-vertex attrributes for Shaders
	 */
	std::vector<int32_t> joint0;
	std::vector<int32_t> joint1;
	std::vector<float> weight_for_joint0; // weight_for_joint1 can be calculated
	std::vector<glm::vec3> vector_from_joint0;
	std::vector<glm::vec3> vector_from_joint1;
	std::vector<glm::vec4> vertex_normals;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::vec2> uv_coordinates;
	std::vector<glm::uvec3> faces;

	std::vector<Material> materials;
	BoundingBox bounds;
	Skeleton skeleton;

	void loadPmd(const std::string& fn);
	void updateAnimation();
	int getNumberOfBones() const;
	glm::vec3 getCenter() const { return 0.5f * glm::vec3(bounds.min + bounds.max); }

	glm::vec3 getJointPosition(int joint_index) const;
	void deform(const int bone_index, const glm::fquat& rotate_quat);	// rotate a bone and recompute all children's data
	glm::vec3 compute_joint_world_position(int bone_index);
	void downward_update_bones(int bone_index);

private:
	void computeBounds();
	void computeNormals();
};


#endif
