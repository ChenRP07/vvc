/*
 * @Author: lixin
 * @Date: 2023-05-16 11:47:17
 * @LastEditTime: 2023-05-18 11:05:13
 * @Description: 
 * Copyright (c) @lixin, All Rights Reserved.
 */
#include "device_launch_parameters.h"
#include <helper_cuda.h>
#include "cuda/octree.cuh"
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess)
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}
struct Points
{
	float x, y, z;
	float r, g, b;
};

// 起始地址 + Slice的属性 + I帧的数组

/**
 * @description: 函数注释配置模板
 * @param {Points*} cudaData		该帧在 VBO 的起始地址
 * @param {int*} inner_offset		该 Patch 在 帧 内的起始地址
 * @param {int} timestamp			帧序号
 * @param {int*} index				Patch 序号
 * @param {uint8_t*} type			Patch 的解码类型
 * @param {float**} mv				每个 Patch 的 MotionVector
 * @param {uint32_t*} size			每个 Patch 解压后的 点数
 * @param {uint8_t*} qp				
 * @param {uint8_t**} geometry
 * @param {uint32_t*} geometry_size
 * @param {uint8_t**} color
 * @param {uint32_t*} color_size
 * @param {InvertRAHTOctree*} invertRAHTOctree_gpu
 * @return {*}
 */
__global__ void processCUDA(Points* cudaData, int timestamp, int* inner_offset, int* index, uint8_t* type, float** mv, uint32_t* size, uint8_t* qp, uint8_t** geometry, uint32_t* geometry_size, uint8_t** color, uint32_t* color_size, vvc::client::octree::InvertRAHTOctree* invertRAHTOctree_gpu, int patch_size, int unuse)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < patch_size){
		vvc::client::common::Slice_t slice(timestamp, index[idx], type[idx], mv[idx], size[idx], qp[idx], geometry[idx], geometry_size[idx], color[idx], color_size[idx]);
		invertRAHTOctree_gpu[index[idx]].SetSlice(slice);
		// 找到帧内偏移
		int offset = inner_offset[idx];
		for(int i=0; i<size[idx]; i++){
			cudaData[offset + i].x = invertRAHTOctree_gpu[index[idx]].source_cloud_[i].x + unuse*100;
			cudaData[offset + i].y = invertRAHTOctree_gpu[index[idx]].source_cloud_[i].y;
			cudaData[offset + i].z = invertRAHTOctree_gpu[index[idx]].source_cloud_[i].z;
			cudaData[offset + i].r = ((invertRAHTOctree_gpu[index[idx]].source_colors_[i].y + 1.4020f * (invertRAHTOctree_gpu[index[idx]].source_colors_[i].v - 128.0f))) / 255;
			cudaData[offset + i].g = ((invertRAHTOctree_gpu[index[idx]].source_colors_[i].y - 0.3441f * (invertRAHTOctree_gpu[index[idx]].source_colors_[i].u - 128.0f) - 0.7141f * (invertRAHTOctree_gpu[index[idx]].source_colors_[i].v - 128.0f))) / 255;
			cudaData[offset + i].b = ((invertRAHTOctree_gpu[index[idx]].source_colors_[i].y + 1.7720f * (invertRAHTOctree_gpu[index[idx]].source_colors_[i].u - 128.0f))) / 255;
		}

		// if(idx == 1){
		// 	printf("offset = %d\n",offset);
		// 	// invertRAHTOctree_gpu[index[1]].GetPatch();
		// 	for(int i=0; i<size[idx]; i++){
		// 		cudaData[offset + i].x += 100;
		// 	}
		// }
	}
}

extern "C" void launch_cudaProcess(int grid, int block, Points * cudaData, int timestamp, int* inner_offset, int* index, uint8_t* type, float** mv, uint32_t* size, uint8_t* qp, uint8_t** geometry, uint32_t* geometry_size, uint8_t** color, uint32_t* color_size, vvc::client::octree::InvertRAHTOctree* invertRAHTOctree_gpu, int patch_size, int unuse){
	processCUDA <<<1, 1 >>> (cudaData, timestamp, inner_offset, index, type, mv, size, qp, geometry, geometry_size, color, color_size, invertRAHTOctree_gpu, patch_size, unuse);
}
