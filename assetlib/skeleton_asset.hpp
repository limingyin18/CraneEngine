#pragma once

#include <array>
#include "asset_loader.h"

static constexpr uint32_t NUM_BONES_PER_VERTEX = 4;

namespace assets
{

	struct VertexBoneData
	{
		uint32_t index;

		float weight;
	};

    /**
     * @brief A single bone of a skeleton.
     * A bone has a name, a transform matrix, child nodes hierarchy
	 * a number of influences on vertices in @BoneVerticesData
     * 
     */
    struct Bone
    {
		std::string name;
		std::array<float,16> transform;

        std::vector<Bone*>child;
    };

	struct BoneVerticesData
	{
		std::vector<uint32_t> indices;
		std::vector<float> weights;
	};

	struct SkeletonInfo {
		uint32_t numBones; // the number of bones this skeleton contains
		std::vector<uint32_t> numBoneVertices;
		uint32_t fullsize;
		CompressionMode compressionMode;
		std::string originalFile;
	};

    SkeletonInfo read_skeleton_info(AssetFile *file);
    void unpack_skeleton(SkeletonInfo *info, const char* sourceBuffer, size_t sourceSize, char* boneWeightsBuffer);
    AssetFile pack_skeleton(const SkeletonInfo &info, const std::vector<char> &buffer);
}
