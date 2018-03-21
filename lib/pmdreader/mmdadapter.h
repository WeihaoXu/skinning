/*
 * For students:
 * This header include essential interfaces for loading PMD files
 * You may hack its implementation but it's not recommended (a waste of time).
 */
#ifndef MMD_ADAPTER_H
#define MMD_ADAPTER_H

#include "material.h"
#include <image.h>
#include <string>

class MMDAdapter;

struct SparseTuple {
	int vid;
	int jid0;
	int jid1;
	float weight0;
	SparseTuple(int v, int j0, int j1, float w)
		: vid(v), jid0(j0), jid1(j1), weight0(w)
	{
	}
};

class MMDReader {
public:
	MMDReader();
	~MMDReader();

	/*
	 * Open a PMD model file.
	 * Input
	 *      fn: file name
	 * Return:
	 *      true: file opened successfully
	 *      false: file failed to open
	 */
	bool open(const std::string& fn);
	/*
	 * Get mesh data from an opened model file
	 * Output:
	 *      V: list of vertices
	 *      F: list of faces
	 *      N: list of vertex normals
	 *      UV: texture UV coordinates for each vertex.
	 */
	void getMesh(std::vector<glm::vec4>& V,
		     std::vector<glm::uvec3>& F,
		     std::vector<glm::vec4>& N,
		     std::vector<glm::vec2>& UV);
	/*
	 * Get list of materials
	 * Check Material struct (in material.h) for details
	 */
	void getMaterial(std::vector<Material>&);
	/*
	 * Get a joint for given ID
	 * Input:
	 *      id: the Joint ID
	 * Output:
	 *      wcoord: the world coordinates of bone end.
	 *      parent: the parent joint. -1 means there is no parent.
	 * Return:
	 *      true: the Joint ID is valid
	 *      false: the Joint ID in invalid.
	 * Note: The range of joint IDs are always starting from zero and
	 *       ending with some positive number. You may assume the joints
	 *       is a tree whose root is Joint 0, which is guaranteed by this
	 *       adapter.
	 *
	 *       The bone structure in actual PMD files is a forest.
	 */
	bool getJoint(int id, glm::vec3& wcoord, int& parent);
	/*
	 * Get a list of tuples representing the vertex-joint weight.
	 * See SparseTuple for more details
	 * Output:
	 *      tup: an array of SparseTuple object
	 * 
	 * Note: if a vertex was binded to two joints, then only one would be
	 *       placed into tup array, because the other one is supposed to
	 *       get calculated in shaders on-the-fly, which is cheaper than
	 *       reading another weight from VRAM.
	 */
	void getJointWeights(std::vector<SparseTuple>& tup);
private:
	std::unique_ptr<MMDAdapter> d_;
};

#endif
