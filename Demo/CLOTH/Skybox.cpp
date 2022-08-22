#include "Skybox.hpp"

using namespace std;
using namespace Crane;

void SkyboxActor::init(Crane::Engine* c, CranePhysics::PositionBasedDynamics* p)
{
    ctx = c;

    LOGI("create skybox");

    Eigen::Vector3f pos{ 0.f, 0.f, 0.f };
    transformer.setPosition(pos);

    cubeGP = make_shared<GraphicsPrimitive>();
    {
        string name = "skybox";
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
            ctx->meshRepository[name] = make_shared<Crane::Cube>(2);
        shared_ptr<Crane::Cube> mesh = dynamic_pointer_cast<Crane::Cube>(ctx->meshRepository[name]);
        //mesh->setVertices([](uint32_t i, Vertex &v)
                          //{ v.color = {1.0f, 0.f, 0.f}; });
        cubeGP->mesh = mesh;
    }
    {

        string nameImage = "hdr";
        {
            string imageName = "StandardCubeMap.tx";
            assets::AssetFile textureFile;
            if (!load_binaryfile((string("assets/") + imageName).c_str(), textureFile))
                throw runtime_error("load asset failed");

            assets::TextureInfo textureInfo = assets::read_texture_info(&textureFile);
            VkDeviceSize imageSize = textureInfo.textureSize;
            vector<uint8_t> pixels(imageSize);
            assets::unpack_texture(&textureInfo, textureFile.binaryBlob.data(), textureFile.binaryBlob.size(), (char*)pixels.data());

            int texWidth, texHeight, texChannels;
            texChannels = 4;
            texWidth = textureInfo.pages[0].width;
            texHeight = textureInfo.pages[0].height;


            VkImage image = VK_NULL_HANDLE;
            VmaAllocation imageMemory = VK_NULL_HANDLE;
            VkExtent3D extent;
            extent.width = texWidth / 4;
            extent.height = texHeight / 3;
            extent.depth = 1;
            {
                VkImageCreateInfo imageCreateInfo{};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.extent = extent;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 6;
                imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo allocInfo{};
                allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                auto res = vmaCreateImage(*ctx->vmaAllocator, &imageCreateInfo, &allocInfo, &image, &imageMemory, nullptr);
                if (res != VK_SUCCESS)
                    throw std::runtime_error("failed to create image!");
            }

            {
                vk::CommandBuffer cmdBuff = ctx->beginSingleTimeCommands();
                Buffer stagingBuffer = Buffer::createStagingBuffer(pixels.data(), imageSize, *ctx->vmaAllocator);

                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 6;

                VkPipelineStageFlags sourceStage;
                VkPipelineStageFlags destinationStage;
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

                vkCmdPipelineBarrier(cmdBuff, sourceStage, destinationStage, 0, 0, nullptr, 0,
                    nullptr, 1, &barrier);


                vector<VkBufferImageCopy> regions(6);
                for (uint32_t i = 0; i < 6; ++i)
                {
                    regions[i].bufferOffset = 0;
                    regions[i].bufferRowLength = 0;
                    regions[i].bufferImageHeight = 0;
                    regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    regions[i].imageSubresource.mipLevel = 0;
                    regions[i].imageSubresource.baseArrayLayer = i;
                    regions[i].imageSubresource.layerCount = 1;
                    regions[i].imageOffset = { 0, 0, 0 };
                    regions[i].imageExtent = extent;
                }

                vkCmdCopyBufferToImage(cmdBuff, stagingBuffer.buffer, image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, regions.data());


                //VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 6;

                //VkPipelineStageFlags sourceStage;
                //VkPipelineStageFlags destinationStage;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                vkCmdPipelineBarrier(cmdBuff, sourceStage, destinationStage, 0, 0, nullptr, 0,
                    nullptr, 1, &barrier);



                ctx->endSingleTimeCommands(cmdBuff);
            }

            {
                vk::ImageViewCreateInfo imageViewCreateInfo{ .image = image,
                                                            .viewType = vk::ImageViewType::eCube,
                                                            .format = vk::Format::eR8G8B8A8Unorm,
                                                            .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                                                 .baseMipLevel = 0,
                                                                                 .levelCount = 1,
                                                                                 .baseArrayLayer = 0,
                                                                                 .layerCount = 6} };

                ctx->loadImageViews[nameImage] = ctx->device->createImageViewUnique(imageViewCreateInfo);
            }

            ctx->descriptorImageInfos[nameImage].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            ctx->descriptorImageInfos[nameImage].imageView = ctx->loadImageViews[nameImage].get();
            ctx->descriptorImageInfos[nameImage].sampler = ctx->textureSampler.get();


            string name = "phongSkybox";
            if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            {
                ctx->materialBuilderPhong.descriptorInfos[1][0].second = &ctx->descriptorImageInfos[nameImage];
                ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
                ctx->materialBuilderPhong.descriptorInfos[1][0].second = &ctx->descriptorImageInfoBlank;
            }

            cubeGP->material = ctx->materialRepository[name];
        }
    }
    {
        Eigen::Vector3f pos{ 0.f, 0.f, 0.f };
        cubeGP->transformer.setPosition(pos);
    }
    cubeGP->transformer.setTransformParent(&transformer.getTransformWorld());
    primitives.push_back(cubeGP);
}

