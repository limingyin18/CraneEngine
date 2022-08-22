#include "Render.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

void Render::createAsset()
{
    createCameraPushConstant();
    createSceneParametersUniformBuffer();

    LOGI("create sampler");
    vk::SamplerCreateInfo samplerInfo{
    .magFilter = vk::Filter::eNearest,
    .minFilter = vk::Filter::eNearest,
    .mipmapMode = vk::SamplerMipmapMode::eNearest,
    .addressModeU = vk::SamplerAddressMode::eRepeat,
    .addressModeV = vk::SamplerAddressMode::eRepeat,
    .addressModeW = vk::SamplerAddressMode::eRepeat,
    .anisotropyEnable = VK_TRUE,
    .maxAnisotropy = physicalDevice.getProperties().limits.maxSamplerAnisotropy,
    .compareEnable = VK_FALSE,
    .compareOp = vk::CompareOp::eAlways,
    .borderColor = vk::BorderColor::eIntOpaqueBlack,
    .unnormalizedCoordinates = VK_FALSE };
    textureSampler = device->createSamplerUnique(samplerInfo);

    LOGI("create basic color");
    vector<uint8_t> blankPiexls{ 255, 255, 255, 255 };
    vector<uint8_t> lilacPiexls{ 179, 153, 255, 255 };
    std::tie(imageBlank, imageViewBlank) = createTextureImage(1, 1, 4, blankPiexls.data());
    std::tie(imageLilac, imageViewLilac) = createTextureImage(1, 1, 4, lilacPiexls.data());

    descriptorImageInfoBlank.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    descriptorImageInfoBlank.imageView = imageViewBlank.get();
    descriptorImageInfoBlank.sampler = textureSampler.get();

    descriptorImageInfoLilac.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    descriptorImageInfoLilac.imageView = imageViewLilac.get();
    descriptorImageInfoLilac.sampler = textureSampler.get();

    createAssetApp();

    assemblePrimitives();
    if (!renderables.empty()) buildRenderable();
}

void Render::createCameraPushConstant()
{
    LOGI("������� push constant");

    cameraPushConstants.resize(swapchainImages.size());
    for (auto& c : cameraPushConstants)
    {
        Eigen::Vector4f position;
        position.head(3) = camera.getCameraPos();
        position[3] = 1.0f;
        c.position = position;
        c.projView = camera.projection * camera.view;
    }
}

void Render::createSceneParametersUniformBuffer()
{
    LOGI("����������������");

    sceneParameters.ambientColor = { 0.1f, 0.1f, 0.1f, 1.0f };
    sceneParameters.fogColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    sceneParameters.fogDistances = { 0.2f, 0.2f, 0.2f, 1.0f };
    sceneParameters.sunlightColor = { 0.9f, 0.9f, 0.9f, 1.0f };
    Eigen::Vector3f sunDirection = { 0.f, -1.f, -0.5f };
    sunDirection.normalize();
    sceneParameters.sunlightDirection = { sunDirection.x(), sunDirection.y(), sunDirection.z(), 1.0f };

    vector<SceneParameters> sceneParametersData(swapchainImages.size(), sceneParameters);
    sceneParametersUniformOffset = padUniformBufferSize(sizeof(SceneParameters), (VkPhysicalDeviceProperties)physicalDevice.getProperties());

    const size_t sceneParameterBufferSize = swapchainImages.size() * sceneParametersUniformOffset;
    sceneParameterBuffer.create(*vmaAllocator, sceneParameterBufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    sceneParameterBuffer.update(sceneParametersData.data());

    sceneParameterBufferDescriptorInfo.buffer = sceneParameterBuffer.buffer;
    sceneParameterBufferDescriptorInfo.offset = 0;
    sceneParameterBufferDescriptorInfo.range = sizeof(SceneParameters);
}

void Render::createDescriptorPool()
{
    LOGI("������������");
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
    { {.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000 } };

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{ .maxSets = 1000, .poolSizeCount = (uint32_t)descriptorPoolSizes.size(), .pPoolSizes = descriptorPoolSizes.data() };

    descriptorPool = device->createDescriptorPoolUnique(descriptorPoolCreateInfo);
}