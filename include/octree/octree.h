/* Copyright Notice.
 *
 * Please read the LICENSE file in the project root directory for details
 * of the open source licenses referenced by this source code.
 *
 * Copyright: @ChenRP07, All Right Reserved.
 *
 * Author        : ChenRP07
 * Description   :
 * Create Time   : 2023/03/14 10:21
 * Last Modified : 2023/04/14 10:33
 *
 */

#ifndef _PVVC_OCTREE_H_
#define _PVVC_OCTREE_H_

#include "common/common.h"
#include "common/exception.h"
#include "common/parameter.h"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <cfloat>
#include <numeric>

namespace vvc {
namespace octree {

	/*
	 * @description : According to space center, half space range, subspace
	 * position computes the subspace center.
	 * @param  : {pcl::PointXYZ _center} space center
	 * @param  : {pcl::PointXYZ _range} subspace range
	 * @param  : {int _pos} subspace position, must be an integer from 0 to 7
	 * @return : {pcl::PointXYZ} subspace center
	 * */
	extern pcl::PointXYZ SubSpaceCenter(pcl::PointXYZ _center, pcl::PointXYZ _range, int _pos);

	/*
	 * @description : Check all eight subspaces, true if each subspace is
	 * empty or contains points from each patch.
	 * @param  : {std::vector<std::vector<std::vector<int>>>& _subspaces}
	 * @return : {bool}
	 * */
	extern bool CheckSubSpace(std::vector<std::vector<std::vector<int>>>& _subspaces);

	/*
	 * @description : Check space, true if it is empty or contains points from each patch.
	 * @param  : {std::vector<std::vector<int>>& _space}
	 * @return : {bool}
	 * */
	extern bool CheckSubSpace(std::vector<std::vector<int>>& _space);

	extern std::pair<common::ColorYUV, common::ColorYUV> HaarTransform(std::pair<int, int>& _w, std::pair<common::ColorYUV, common::ColorYUV>& _g);

	extern std::pair<common::ColorYUV, common::ColorYUV> InvertHaarTransform(std::pair<int, int>& _w, std::pair<common::ColorYUV, common::ColorYUV>& _g);

	static uint8_t NodeValue[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

	static int NodeWeight[8][2] = {{0, 0}, {2, 3}, {4, 6}, {5, 7}, {8, 12}, {9, 13}, {10, 14}, {11, 15}};

	struct OctreeNode_t {
		/*
		    For a node, eight subnodes are
		      6————4
		     /|   /|
		    2—+——0 |
		    | 7——+-5
		    |/   |/
		    3————1
		    x/y/z > center ? 0 : 1 -> xyz from 000 to 111
		*/
		uint8_t value;

		/* Children indexes in next layer or in real point cloud */
		std::vector<int> index;

		/* Points number that this node contains
		 * null/0-7/0246/1357/04/15/26/37/0/1/2/3/4/5/6/7 */
		int weight[16];

		/* g_DC/h_xyz/h_xy/h_xy/h_x/h_x/h_x/h_x/g0/g1/g2/g3/g4/g5/g6/g7 */
		common::ColorYUV raht[16];

		/* Default constructor */
		OctreeNode_t() : value{0x00}, index{0}, raht{} {
			for (auto& i : weight) {
				i = 0;
			}
		}

		OctreeNode_t(const OctreeNode_t& _x) : value{_x.value}, index{_x.index} {
			for (int i = 0; i < 16; ++i) {
				this->weight[i] = _x.weight[i];
			}

			for (int i = 0; i < 16; ++i) {
				this->raht[i] = _x.raht[i];
			}
		}

		OctreeNode_t& operator=(const OctreeNode_t& _x) {
			this->value = _x.value;
			this->index = _x.index;

			for (int i = 0; i < 16; ++i) {
				this->weight[i] = _x.weight[i];
			}

			for (int i = 0; i < 16; ++i) {
				this->raht[i] = _x.raht[i];
			}
			return *this;
		}

		void HierarchicalTransform();
		void InvertHierarchicalTransform();
	};

	class OctreeBase {
	  protected:
		/* Range of this tree, for a cube, is edge */
		pcl::PointXYZ tree_range_;

		/* TODO : Deformed octree, range x, y, z and
		 * deflection angles x, y, z */

		/* Tree center */
		pcl::PointXYZ tree_center_;

		/* Tree height */
		int tree_height_;

		/* patchVVC parameters */
		common::PVVCParam_t::Ptr params_;

	  public:
		/*
		 * @description : Constructor.
		 * */
		OctreeBase();

		/*
		 * @description : Deconstructor.
		 * */
		virtual ~OctreeBase() = default;

		/*
		 * @description : Set PVVC parameters.
		 * @params : {std::shared_ptr<common::VVCParam_t> _param}
		 * @return : {}
		 * */
		void SetParams(common::PVVCParam_t::Ptr _param);

		/*
		 * @description : Pure virtual function, an interface used to make octree.
		 * @params : {}
		 * @return : {}
		 * */
		virtual void MakeTree() = 0;
	};

	class SingleOctree : public OctreeBase {};

	class CommonOctree : public OctreeBase {};

	class RAHTOctree : public OctreeBase {
	  private:
		std::vector<std::vector<OctreeNode_t>>         tree_;          /* Tree nodes */
		pcl::PointCloud<pcl::PointXYZRGB>::Ptr         source_cloud_;  /* Geometry of common patch */
		std::shared_ptr<std::vector<common::ColorYUV>> source_colors_; /* YUV colors to be transformed */
		std::vector<common::ColorYUV>                  RAHT_result_;   /* RAHT result */

		/*
		 * @description : Add a node in _height layer.
		 * @param  : {std::vector<int>& _points}
		 * @param  : {const int _height}
		 * @param  : {const pcl::PointXYZ _center}
		 * @param  : {const pcl::PointXYZ _range}
		 * @return : {}
		 * */
		void AddNode(std::vector<int>& _points, const int _height, const pcl::PointXYZ _center, const pcl::PointXYZ _range);

	  public:
		/* Default constructor and deconstructor */
		RAHTOctree();

		virtual ~RAHTOctree() = default;

		/*
		 * @description : Set geometry of common patch
		 * @param  : {pcl::PointCloud<pcl::PointXYZRGB>::Ptr _cloud}
		 * @return : {}
		 * */
		void SetSourceCloud(pcl::PointCloud<pcl::PointXYZRGB>::Ptr _cloud);

		/*
		 * @description : Set color YUV which will be transformed
		 * @param  : {std::shared_ptr<std::vector<common::ColorYUV>> _colors}
		 * @return : {}
		 * */
		void SetSourceColors(std::shared_ptr<std::vector<common::ColorYUV>> _colors);

		/*
		 * @description : Output octree by sequence traversal
		 * @param  : {}
		 * @return : {std::vector<uint8_t>}
		 * */
		std::vector<uint8_t> GetOctree() const;

		/*
		 * @description : Move color result to return value
		 * @param  : {}
		 * @return : {std::vector<common::ColorYUV>}
		 * */
		std::vector<common::ColorYUV> GetRAHTResult();

		std::vector<int> GetRAHTWeights() const;

		/*
		 * @description : Do RAHT
		 * @param  : {}
		 * @return : {}
		 * */
		void RAHT();

		/*
		 * @description : Make octree
		 * @param  : {}
		 * @return : {}
		 * */
		void MakeTree();
	};
}  // namespace octree
}  // namespace vvc
#endif
