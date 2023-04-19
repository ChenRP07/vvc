/* Copyright Notice.
 *
 * Please read the LICENSE file in the project root directory for details
 * of the open source licenses referenced by this source code.
 *
 * Copyright: @ChenRP07, All Right Reserved.
 *
 * Author        : ChenRP07
 * Description   : Implementation of parameters.
 * Create Time   : 2022/12/12 16:38
 * Last Modified : 2022/12/27 14:06
 *
 */

#include "common/parameter.h"

using namespace vvc;

void common::SetDefaultParams(common::PVVCParam_t::Ptr _ptr) {
	try {
		if (!_ptr) {
			throw __EXCEPT__(EMPTY_PARAMS);
		}

		/* 0Bxxxxxxx1 brief 0Bxxxxxx1x normal 0Bxxxxx1xx complete */
		_ptr->log_level = 0x03;

		_ptr->segment.type      = common::DENSE_SEGMENT;
		_ptr->segment.num       = 2048;
		_ptr->segment.iter      = 100;
		_ptr->segment.nn        = 10;
		_ptr->segment.block_num = 8.0f;

		_ptr->thread_num = 30;
        _ptr->zstd_level = 22;

		_ptr->icp.centroid_alignment = true;
		_ptr->icp.correspondence_ths = 100.0f;
		_ptr->icp.iteration_ths      = 100;
		_ptr->icp.mse_ths            = 0.01f;
		_ptr->icp.transformation_ths = 1e-6;
		_ptr->icp.radius_search_ths  = 10.0f;
		_ptr->icp.type               = common::SIMPLE_ICP;
	}
	catch (const common::Exception& e) {
		e.Log();
		throw __EXCEPT__(ERROR_OCCURED);
	}
}

common::PVVCParam_t::Ptr common::CopyParams(common::PVVCParam_t::Ptr _ptr) {
	try {
		if (!_ptr) {
			throw __EXCEPT__(EMPTY_PARAMS);
		}

        common::PVVCParam_t::Ptr p(new common::PVVCParam_t());

        p->log_level = _ptr->log_level;
        p->thread_num = _ptr->thread_num;
        p->zstd_level = _ptr->zstd_level;
        
        p->segment.type = _ptr->segment.type;
        p->segment.num = _ptr->segment.num;
        p->segment.iter = _ptr->segment.iter;
        p->segment.nn = _ptr->segment.nn;
        p->segment.block_num = _ptr->segment.block_num;

        p->icp.centroid_alignment = _ptr->icp.centroid_alignment;
		p->icp.correspondence_ths = _ptr->icp.correspondence_ths;
        p->icp.iteration_ths = _ptr->icp.iteration_ths;
        p->icp.mse_ths = _ptr->icp.mse_ths;
        p->icp.transformation_ths = _ptr->icp.transformation_ths;
        p->icp.radius_search_ths = _ptr->icp.radius_search_ths;
        p->icp.type = _ptr->icp.type;

        return p;
	}
	catch (const common::Exception& e) {
		e.Log();
		throw __EXCEPT__(ERROR_OCCURED);
	}
}
