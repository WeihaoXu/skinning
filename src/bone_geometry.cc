#include "config.h"
#include "bone_geometry.h"
#include <fstream>
#include <queue>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

/*
 * For debugging purpose.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	size_t count = std::min(v.size(), static_cast<size_t>(10));
	for (size_t i = 0; i < count; ++i) os << i << " " << v[i] << "\n";
	os << "size = " << v.size() << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const BoundingBox& bounds)
{
	os << "min = " << bounds.min << " max = " << bounds.max;
	return os;
}


// FIXME: Implement bone animation.


void Skeleton::refreshCache()
{
	joint_rot.resize(joints.size());
	joint_trans.resize(joints.size());
	for (size_t i = 0; i < joints.size(); i++) {
		joint_rot[i] = joints[i].orientation;
		joint_trans[i] = joints[i].position;
	}
}

const glm::vec3* Skeleton::collectJointTrans() const
{
	return joint_trans.data();
}

const glm::fquat* Skeleton::collectJointRot() const
{
	return joint_rot.data();
}

// my implementation for getting bone transform matrix:
const glm::mat4 Skeleton::getBoneTransform() const
{
	return bone_transforms[0];	// for tmp test use
}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::loadPmd(const std::string& fn)
{
	MMDReader mr;
	mr.open(fn);
	mr.getMesh(vertices, faces, vertex_normals, uv_coordinates);
	computeBounds();
	mr.getMaterial(materials);

	// FIXME: load skeleton and blend weights from PMD file,
	//        initialize std::vectors for the vertex attributes,
	//        also initialize the skeleton as needed

	// read in joint data.
	int jointId = 0;
	while(true) {
		glm::vec3 wcoord;
		int parentId;
		if(mr.getJoint(jointId, wcoord, parentId)) {
			Joint curr_joint(jointId, wcoord, parentId);
			skeleton.joints.push_back(curr_joint);
			jointId++;
		}
		else {
			break;
		}
	}
	// init orientation, rel_orientation and children list.
	// computation of orientation: https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
	for(int i = 0; i < skeleton.joints.size(); i++) {
		Joint& curr_joint = skeleton.joints[i];
		if(curr_joint.parent_index == -1) {	// this is a root jonit
			curr_joint.orientation = glm::fquat(1.0, 0.0, 0.0, 0.0);	// identity.
			curr_joint.rel_orientation = glm::fquat(1.0, 0.0, 0.0, 0.0);	// identity.
			// std::cout << "root joint" << std::endl;
		}
		else {
			Joint& parent_joint = skeleton.joints[curr_joint.parent_index];
			glm::vec3 x_direct(1.0, 0.0, 0.0);
			glm::vec3 init_direct = glm::normalize(curr_joint.init_position - parent_joint.init_position);
			// curr_joint.orientation.xyz = glm::cross(y_direct, init_direct);
			// curr_joint.orientation.w = std::sqrt(std::pow(glm::length(y_direct), 2) + std::pow(glm::length(init_direct), 2))
			// 								+ glm::dot(y_direct, init_direct);
			glm::vec3 xyz = glm::cross(x_direct, init_direct);
			int w = std::sqrt(2) + glm::dot(x_direct, init_direct);;
			curr_joint.orientation.x = xyz.x;
			curr_joint.orientation.y = xyz.y;
			curr_joint.orientation.z = xyz.z;
			curr_joint.orientation.w = w;
			curr_joint.orientation = glm::normalize(curr_joint.orientation);

			curr_joint.rel_orientation = glm::fquat(1.0, 0.0, 0.0, 0.0);	// relative orientation w.r.t. parent. Init as identity.

			parent_joint.children.push_back(curr_joint.joint_index);
			// std::cout << "non-root joint. number: " << curr_joint.joint_index << " parent: " << curr_joint.parent_index << std::endl;
		}
		skeleton.bone_transforms.push_back(glm::mat4(1.0));
	}

}

void Mesh::updateAnimation()
{
	skeleton.refreshCache();
}

int Mesh::getNumberOfBones() const
{
	return skeleton.joints.size();
}

void Mesh::computeBounds()
{
	bounds.min = glm::vec3(std::numeric_limits<float>::max());
	bounds.max = glm::vec3(-std::numeric_limits<float>::max());
	for (const auto& vert : vertices) {
		bounds.min = glm::min(glm::vec3(vert), bounds.min);
		bounds.max = glm::max(glm::vec3(vert), bounds.max);
	}
}

