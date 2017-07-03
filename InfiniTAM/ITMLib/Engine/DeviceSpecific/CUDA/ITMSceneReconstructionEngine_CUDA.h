// Copyright 2014-2015 Isis Innovation Limited and the authors of InfiniTAM

#pragma once

#include "../../ITMSceneReconstructionEngine.h"
#include "../../../../ORUtils/MemoryBlock.h"

#include <queue>

namespace ITMLib
{
	namespace Engine
	{
		struct VisibleBlockInfo {
			size_t count;
			size_t frameIdx;
			ORUtils::MemoryBlock<Vector3i> *blockCoords;
		};

		template<class TVoxel, class TIndex>
		class ITMSceneReconstructionEngine_CUDA : public ITMSceneReconstructionEngine < TVoxel, TIndex >
		{};

		// Reconstruction engine for voxel hashing.
		template<class TVoxel>
		class ITMSceneReconstructionEngine_CUDA<TVoxel, ITMVoxelBlockHash> : public ITMSceneReconstructionEngine < TVoxel, ITMVoxelBlockHash >
		{
		private:
			void *allocationTempData_device;
			void *allocationTempData_host;
			unsigned char *entriesAllocType_device;
			Vector4s *blockCoords_device;

			// Keeps track of recent lists of visible block IDs. Used by the voxel decay.
			std::queue<VisibleBlockInfo> frameVisibleBlocks;
			// Used by the voxel decay code to keep track of empty blocks which can be safely
			// deleted and reused.
			int *blocksToDeallocate_device;
			int *blocksToDeallocateCount_device;
			int *lastFreeBlockId_device;
			// Used to avoid data races when deleting elements from the hash table.
			int *locks_device;

			// The maximum number of blocks which can be deallocated in one operation.
			const int maxBlocksToDeallocate = 16000;

			long totalDecayedBlockCount = 0L;
			size_t frameIdx = 0;	 // TODO: proper frame counting

		public:
			void ResetScene(ITMScene<TVoxel, ITMVoxelBlockHash> *scene);

			void AllocateSceneFromDepth(ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const ITMView *view, const ITMTrackingState *trackingState,
				const ITMRenderState *renderState, bool onlyUpdateVisibleList = false);

			void IntegrateIntoScene(ITMScene<TVoxel, ITMVoxelBlockHash> *scene, const ITMView *view, const ITMTrackingState *trackingState,
				const ITMRenderState *renderState);

			void Decay(ITMScene<TVoxel, ITMVoxelBlockHash> *scene,
					   int maxWeight,
					   int minAge,
					   bool forceAllVoxels) override;

			size_t GetDecayedBlockCount() override;

			ITMSceneReconstructionEngine_CUDA(void);
			~ITMSceneReconstructionEngine_CUDA(void);
		};

		// Reconstruction engine for plain voxel arrays (vanilla Kinectfusion-style).
		template<class TVoxel>
		class ITMSceneReconstructionEngine_CUDA<TVoxel, ITMPlainVoxelArray> : public ITMSceneReconstructionEngine < TVoxel, ITMPlainVoxelArray >
		{
		public:
			void ResetScene(ITMScene<TVoxel, ITMPlainVoxelArray> *scene);

			void AllocateSceneFromDepth(ITMScene<TVoxel, ITMPlainVoxelArray> *scene, const ITMView *view, const ITMTrackingState *trackingState,
				const ITMRenderState *renderState, bool onlyUpdateVisibleList = false);

			void IntegrateIntoScene(ITMScene<TVoxel, ITMPlainVoxelArray> *scene, const ITMView *view, const ITMTrackingState *trackingState,
				const ITMRenderState *renderState);

		  void Decay(ITMScene<TVoxel, ITMPlainVoxelArray> *scene,
					 int maxWeight,
					 int minAge,
					 bool forceAllVoxels) override;

		  size_t GetDecayedBlockCount() override;
		};
	}
}
