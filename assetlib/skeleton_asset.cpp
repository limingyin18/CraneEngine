#include "skeleton_asset.hpp"
#include "nlohmann/json.hpp"
#include "lz4.h"

using namespace assets;

assets::SkeletonInfo assets::read_skeleton_info(assets::AssetFile *file)
{
    assets::SkeletonInfo info;

    nlohmann::json metadata = nlohmann::json::parse(file->json);

    info.numBones = metadata["num_bones"];
    info.numBoneVertices = metadata["num_bone_vertices"].get<std::vector<uint32_t>>();
    info.fullsize = metadata["fullsize"];
    info.originalFile = metadata["original_file"];
    std::string compressionString = metadata["compression"];
    info.compressionMode = parse_compression(compressionString.c_str());

    return info;
}

AssetFile assets::pack_skeleton(const SkeletonInfo &info, const std::vector<char> &buffer)
{
    AssetFile file;
    file.type[0] = 'B';
    file.type[1] = 'O';
    file.type[2] = 'N';
    file.type[3] = 'E';
    file.version = 1;

    nlohmann::json metadata;
    metadata["num_bones"] = info.numBones;
    metadata["num_bone_vertices"] = info.numBoneVertices;
    metadata["fullsize"] = info.fullsize;
    metadata["original_file"] = info.originalFile;
    metadata["compression"] = "LZ4";
    file.json = metadata.dump();

    size_t fullsize = buffer.size();

    // compress buffer and copy it into the file struct
    size_t compressStaging = LZ4_compressBound(static_cast<int>(fullsize));

    file.binaryBlob.resize(compressStaging);

    int compressedSize = LZ4_compress_default(buffer.data(), file.binaryBlob.data(), static_cast<int>(buffer.size()), static_cast<int>(compressStaging));
    file.binaryBlob.resize(compressedSize);

    return file;
}

void assets::unpack_skeleton(SkeletonInfo *info, const char *sourceBuffer, size_t sourceSize, char* boneWeightsBuffer)
{
    // decompressing into temporal vector. TODO: streaming decompress directly on the buffers
    std::vector<char> decompressedBuffer;
    decompressedBuffer.resize(info->fullsize);

    LZ4_decompress_safe(sourceBuffer, decompressedBuffer.data(), static_cast<int>(sourceSize), static_cast<int>(decompressedBuffer.size()));

	memcpy(boneWeightsBuffer, decompressedBuffer.data(), info->fullsize);
}