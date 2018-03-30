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
#include <glm/gtc/type_ptr.hpp>

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
		// joint_rot[i] = joints[i].rel_orientation;
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
// remember that every bone is pointing from parent to child!
const glm::mat4 Skeleton::getBoneTransform(int joint_index) const
{
	// return bone_transforms[joint_index];
	const Joint& curr_joint = joints[joint_index];
	const Joint& parent_joint = joints[curr_joint.parent_index];

	float length = glm::length(curr_joint.position - parent_joint.position);
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0), glm::vec3(1.0, length, 1.0));

	glm::fquat rotate_quat = quaternion_between_two_directs(glm::vec3(0.0, 1.0, 0.0), curr_joint.position - parent_joint.position);
	glm::mat4 rotate_matrix = glm::toMat4(rotate_quat);

	glm::vec3 translate = parent_joint.position;
	glm::mat4 translate_matrix = glm::translate(glm::mat4(1.0f), translate);


	return translate_matrix * rotate_matrix * scale_matrix;

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

		curr_joint.orientation = glm::fquat();
		curr_joint.rel_orientation = glm::fquat();
		if(curr_joint.parent_index != -1) {
			Joint& parent_joint = skeleton.joints[curr_joint.parent_index];
			parent_joint.children.push_back(curr_joint.joint_index);
		}	
		// skeleton.bone_transforms.push_back(glm::mat4(1.0));	// for tmp use
	}

	// load wieghts
	std::vector<SparseTuple> sparse_tuples;
	mr.getJointWeights(sparse_tuples);

	for(SparseTuple& tuple : sparse_tuples) {
		int vid = tuple.vid;
		joint0.push_back(tuple.jid0);
		weight_for_joint0.push_back(tuple.weight0);
		vector_from_joint0.push_back(glm::vec3(vertices[vid]) - skeleton.joints[tuple.jid0].position);

		if(tuple.jid1 == -1) {
			joint1.push_back(0);	// avoid joints[-1] access
			vector_from_joint1.push_back(glm::vec3(0.0, 0.0, 0.0));
			// std::cout << "joint1 equals -1" << std::endl;
		}
		else {
			joint1.push_back(tuple.jid1);
			vector_from_joint1.push_back(glm::vec3(vertices[vid]) - skeleton.joints[tuple.jid1].position);
		}
	}

	updateAnimation();
}

void Mesh::rotate_bone(const int bone_index, const glm::fquat& rotate_quat) {
	Joint& curr_joint = skeleton.joints[bone_index];
	Joint& parent_joint = skeleton.joints[curr_joint.parent_index];
	parent_joint.rel_orientation = rotate_quat * parent_joint.rel_orientation;
	parent_joint.orientation = rotate_quat * parent_joint.orientation;
	update_children(parent_joint, rotate_quat);
}


void Mesh::update_children(Joint& parent_joint, const glm::fquat& rotate_quat) {
	for(int child_index : parent_joint.children) {
		Joint& child_joint = skeleton.joints[child_index];
		child_joint.orientation = rotate_quat * child_joint.orientation;
		child_joint.position = parent_joint.position + parent_joint.orientation * (child_joint.init_position - parent_joint.init_position);
		update_children(child_joint, rotate_quat);
	}
}

void Mesh::translate_root(glm::vec3 offset) {
	for(Joint& joint : skeleton.joints) {
		joint.position = joint.position + offset;
	}
}


void Mesh::savePose(const std::string& pose_file) {

}

void Mesh::loadPose(const std::string& pose_file) {

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



