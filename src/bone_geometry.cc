#include "config.h"
#include "bone_geometry.h"
#include "procedure_geometry.h"
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


// calculate bone_transform_mats that transform cylinders to bones
void Skeleton::calculate_bone_transforms() {
	bone_transforms.clear();
	glm::mat4 curr_transform;
	for(int i = 0; i < joints.size(); i++) {
		int parent_index = joints[i].parent_index;
		glm::vec3 translate;
		// glm::mat4 translate;
		float length = 0;
		if(parent_index == -1) {	// root
			// translate = glm::translate(glm::vec3(0.0, 0.0, 0.0));	// whatever. A root joints doesn't refer to any bones
			translate = glm::vec3(0.0f, 0.0f, 0.0f);
			length = 10;
		}
		else {
			translate = glm::vec3(0.0f, 1.0f, 0.0f);	// for debug
			// translate = glm::translate(glm::vec3(0.0, 1.0, 0.0));	// for debug
			// translate = (joints[i].position + joints[parent_index].position) * 0.5f;
			std::cout << "bone center: (" << translate.x << ", " << translate.y << ", " << translate.z << ")" << std::endl;	// lengths are normal
			length =  glm::length(joints[i].position - joints[parent_index].position);
			std::cout << "length of joint " << i << ": " << length << std::endl;	// lengths are normal
		}
		// curr_transform = glm::scale(curr_transform, glm::vec3(1.0, length, 1.0));
		// curr_transform =  glm::toMat4(joints[i].orientation) * curr_transform;	// rotate cylinder

		curr_transform = glm::translate(curr_transform, translate.x, translate.y, translate.z);
		// curr_transform = translate * curr_transform;
		bone_transforms.push_back(curr_transform);
	}
}


// my implementation for getting bone transform matrix:
const glm::mat4 Skeleton::getBoneTransform(int joint_index) const
{
	return bone_transforms[joint_index];
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
			std::cout << "join " << jointId << ": (" << wcoord.x << ", " << wcoord.y << ", " << wcoord.z << ")" << std::endl;
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
			curr_joint.orientation = glm::fquat();	// identity.
			curr_joint.rel_orientation = glm::fquat();	// identity.
			// std::cout << "root joint" << std::endl;
		}
		else {
			Joint& parent_joint = skeleton.joints[curr_joint.parent_index];
			// calculate orientation w.r.t. y axis.
			glm::vec3 y_direct(0.0, 1.0, 0.0);
			glm::vec3 init_direct = curr_joint.init_position - parent_joint.init_position;
			curr_joint.orientation = quaternion_between_two_directs(y_direct, init_direct);
			
			// init T as identity.
			curr_joint.rel_orientation = glm::fquat();	// relative orientation w.r.t. parent. Init as identity.

			// insert current joint into parent's children list
			parent_joint.children.push_back(curr_joint.joint_index);
			// std::cout << "non-root joint. number: " << curr_joint.joint_index << " parent: " << curr_joint.parent_index << std::endl;
		}
		// skeleton.bone_transforms.push_back(glm::mat4(1.0));	// for tmp use
	}
	skeleton.calculate_bone_transforms();
}

void Mesh::updateAnimation()
{
	skeleton.refreshCache();
}

int Mesh::getNumberOfBones() const
{
	return skeleton.joints.size();
}

glm::vec3 Mesh::getJointPosition(int joint_index) const
{
	return skeleton.joints[joint_index].position;
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

