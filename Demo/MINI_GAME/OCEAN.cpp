#include "OCEAN.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

constexpr int N = 32;
constexpr int M = 32;
constexpr float LX = 100.0f;
constexpr float LZ = 100.0f;
constexpr float AMPLITUDE = 100.f;
constexpr float WIND_SPEED = 100.0f;
constexpr complex<float> WIND_DIRECTION = { 1.f, 1.f };
constexpr float CHOPPY_FACTOR = 1.0f;


OCEAN::OCEAN(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win), ocean{ N, M }, oceanAmpl(N, M, LX, LZ, AMPLITUDE, WIND_SPEED, WIND_DIRECTION, CHOPPY_FACTOR)
{
	//preferPresentMode = vk::PresentModeKHR::eFifo;

	camera.target = Vector3f{ 0.f, 2.5f, 0.f };
	camera.rotation[0] = -0.2f;
}

OCEAN::~OCEAN()
{
	device->waitIdle();
}

void OCEAN::updateApp()
{
	updateEngine();

	t.update(&dtAll);
	computeQueue.submit(1, &computeSubmitInfo, vk::Fence{});
	computeQueue.waitIdle();
}

void OCEAN::setImgui()
{
	static size_t count = 0;

	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowPos(ImVec2{ 0.f, 0.f });
	ImGui::Text("count:\t%d", count++);
	ImGui::Text("FPS:\t%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("camera position: %5.1f %5.1f %5.1f", camera.getCameraPos().x(), camera.getCameraPos().y(), camera.getCameraPos().z());
	ImGui::Text("camera rotation: %5.1f %5.1f %5.1f", camera.rotation.x(), camera.rotation.y(), camera.rotation.z());
	ImGui::End();
}

void OCEAN::bindIff2SSBO(Material& material, vk::DescriptorBufferInfo& info)
{
	vk::WriteDescriptorSet writeDescriptorSet0{
		.dstSet = material.descriptorSets[0],
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &info };
	material.writeDescriptorSets.push_back(writeDescriptorSet0);

	vk::WriteDescriptorSet writeDescriptorSet1{
		.dstSet = material.descriptorSets[0],
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.pBufferInfo = &NBInfo };
	material.writeDescriptorSets.push_back(writeDescriptorSet1);
	device->updateDescriptorSets(material.writeDescriptorSets.size(),
		material.writeDescriptorSets.data(), 0, nullptr);
}

void OCEAN::createOceanSSBO(Buffer& buff, vk::DescriptorBufferInfo& info, uint32_t binding)
{
	size_t size = N * M * 2 * 4;
	buff.create(*vmaAllocator, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VmaAllocationCreateFlagBits{});
	info.buffer = buff.buffer;
	info.range = size;
	vk::WriteDescriptorSet writeDescriptorSet{
	.dstSet = materialAmpl.descriptorSets[0],
	.dstBinding = binding,
	.descriptorCount = 1,
	.descriptorType = vk::DescriptorType::eStorageBuffer,
	.pBufferInfo = &info };
	materialAmpl.writeDescriptorSets.push_back(writeDescriptorSet);
}

void OCEAN::createOceanUniform(Buffer& buff, vk::DescriptorBufferInfo& info, uint32_t binding)
{
	size_t size = 4;
	buff.create(*vmaAllocator, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	info.buffer = buff.buffer;
	info.range = size;
	vk::WriteDescriptorSet writeDescriptorSet{
	.dstSet = materialAmpl.descriptorSets[0],
	.dstBinding = binding,
	.descriptorCount = 1,
	.descriptorType = vk::DescriptorType::eUniformBuffer,
	.pBufferInfo = &info };
	materialAmpl.writeDescriptorSets.push_back(writeDescriptorSet);
}

void OCEAN::createAssetApp()
{
	LOGI("创建应用资产");

	SDL2_IMGUI_BASE::createAssetApp();

	// pipeline phong
	auto& pipelinePass = pipelinePasss[PipelineType::PHONG];
	pipelinePass.pipelineType = PipelineType::PHONG;
	pipelinePass.device = device.get();
	pipelinePass.renderPass = renderPass.get();

	auto vertShaderCode = Loader::readFile("shaders/phong.vert.spv");
	pipelinePass.addShader(vertShaderCode, vk::ShaderStageFlagBits::eVertex);
	auto fragShaderCode = Loader::readFile("shaders/phong.frag.spv");
	pipelinePass.addShader(fragShaderCode, vk::ShaderStageFlagBits::eFragment);
	pipelinePass.bindings[0][0].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	pipelinePass.bindings[0][1].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	pipelinePass.buildLayout();

	// create texture
	assets::AssetFile textureYume;
	if (!load_binaryfile("assets/anime.tx", textureYume))
		throw runtime_error("load asset failed");

	assets::TextureInfo textureInfo = assets::read_texture_info(&textureYume);
	VkDeviceSize imageSize = textureInfo.textureSize;
	vector<uint8_t> pixels(imageSize);
	assets::unpack_texture(&textureInfo, textureYume.binaryBlob.data(), textureYume.binaryBlob.size(), (char*)pixels.data());

	int texWidth, texHeight, texChannels;
	texChannels = 4;
	texWidth = textureInfo.pages[0].width;
	texHeight = textureInfo.pages[0].height;

	tie(textureImage, textureImageView) = createTextureImage(texWidth, texHeight, texChannels, pixels.data());

	// material phong
	materialPhong.descriptorPool = descriptorPool.get();
	materialPhong.pipelinePass = &pipelinePass;
	materialPhong.buildSets();

	materialPhong.imageView = textureImageView.get();
	materialPhong.imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	materialPhong.imageInfo.imageView = materialPhong.imageView;
	materialPhong.imageInfo.sampler = textureSampler.get();

	vk::WriteDescriptorSet writeDescriptorSet0{
		.dstSet = materialPhong.descriptorSets[1],
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.pImageInfo = &materialPhong.imageInfo };
	materialPhong.writeDescriptorSets.push_back(writeDescriptorSet0);

	vk::WriteDescriptorSet writeDescriptorSet1{
		.dstSet = materialPhong.descriptorSets[0],
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
		.pBufferInfo = &modelMatrixBufferDescriptorInfo };
	materialPhong.writeDescriptorSets.push_back(writeDescriptorSet1);

	vk::WriteDescriptorSet writeDescriptorSet2{
		.dstSet = materialPhong.descriptorSets[0],
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
		.pBufferInfo = &sceneParameterBufferDescriptorInfo };
	materialPhong.writeDescriptorSets.push_back(writeDescriptorSet2);


	//floorEnv.setVertices([](uint32_t i, Vertex& v) {v.position *= 100.f; });
	renderables.emplace_back(&floorEnv, &materialPhong);
	renderables.emplace_back(&cube, &materialPhong);


	// pipeline pass ampl
	amplPipelinePass.pipelineType = PipelineType::COMPUTE;
	amplPipelinePass.device = device.get();

	auto amplShaderCode = Loader::readFile("shaders/ampl.comp.spv");
	amplPipelinePass.addShader(amplShaderCode, vk::ShaderStageFlagBits::eCompute);
	amplPipelinePass.buildLayout();
	vk::PipelineShaderStageCreateInfo computeShaderStageCreateInfo{ .stage = vk::ShaderStageFlagBits::eCompute,
	.module = amplPipelinePass.shaderModules[0].get(), .pName = "main" };

	vk::ComputePipelineCreateInfo computePipelineCreateInfo{ .stage = computeShaderStageCreateInfo,
		.layout = amplPipelinePass.layout.get(), };

	amplPipelinePass.pipeline = device->createComputePipelineUnique(pipelineCache.get(), computePipelineCreateInfo);

	// material ampl
	materialAmpl.descriptorPool = descriptorPool.get();
	materialAmpl.pipelinePass = &amplPipelinePass;
	materialAmpl.buildSets();

	createOceanSSBO(ampl, amplInfo, 0);
	createOceanSSBO(normalX, normalXInfo, 1);
	createOceanSSBO(normalZ, normalZInfo, 2);
	createOceanSSBO(dx, dxInfo, 3);
	createOceanSSBO(dz, dzInfo, 4);

	{
		h_tlide_0.create(*vmaAllocator, N * M * 2 * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		h_tlide_0Info.buffer = h_tlide_0.buffer;
		h_tlide_0Info.range = h_tlide_0.size;
		vk::WriteDescriptorSet writeDescriptorSet{
		.dstSet = materialAmpl.descriptorSets[0],
		.dstBinding = 5,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &h_tlide_0Info };
		materialAmpl.writeDescriptorSets.push_back(writeDescriptorSet);

		h_tlide_0.update(oceanAmpl.h_tlide_0.data());
	}

	{
		h_tlide_0_conj.create(*vmaAllocator, N * M * 2 * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		h_tlide_0_conjInfo.buffer = h_tlide_0_conj.buffer;
		h_tlide_0_conjInfo.range = h_tlide_0_conj.size;
		vk::WriteDescriptorSet writeDescriptorSet{
		.dstSet = materialAmpl.descriptorSets[0],
		.dstBinding = 6,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &h_tlide_0_conjInfo };
		materialAmpl.writeDescriptorSets.push_back(writeDescriptorSet);

		h_tlide_0_conj.update(oceanAmpl.h_tlide_0_conj.data());
	}


	createOceanUniform(NB, NBInfo, 7);
	NB.update(&N);
	createOceanUniform(MB, MBInfo, 8);
	MB.update(&M);
	createOceanUniform(Lx, LxInfo, 9);
	Lx.update(&LX);
	createOceanUniform(Lz, LzInfo, 10);
	Lz.update(&LZ);
	createOceanUniform(t, tInfo, 11);
	t.update(&dt);

	device->updateDescriptorSets(materialAmpl.writeDescriptorSets.size(),
		materialAmpl.writeDescriptorSets.data(), 0, nullptr);

	// pipeline pass ifft2
	iff2PipelinePass.pipelineType = PipelineType::COMPUTE;
	iff2PipelinePass.device = device.get();

	auto ifft22ShaderCode = Loader::readFile("shaders/ifft2.comp.spv");
	iff2PipelinePass.addShader(ifft22ShaderCode, vk::ShaderStageFlagBits::eCompute);
	iff2PipelinePass.buildLayout();
	vk::PipelineShaderStageCreateInfo computeShaderStageCreateInfoIfft2{ .stage = vk::ShaderStageFlagBits::eCompute,
	.module = iff2PipelinePass.shaderModules[0].get(), .pName = "main" };

	vk::ComputePipelineCreateInfo computePipelineCreateInfoIfft2{ .stage = computeShaderStageCreateInfoIfft2,
		.layout = iff2PipelinePass.layout.get(), };
	iff2PipelinePass.pipeline = device->createComputePipelineUnique(pipelineCache.get(), computePipelineCreateInfoIfft2);

	// material iff2
	materialIff2Ampl.descriptorPool = descriptorPool.get();
	materialIff2Ampl.pipelinePass = &iff2PipelinePass;
	materialIff2Ampl.buildSets();
	bindIff2SSBO(materialIff2Ampl, amplInfo);

	materialIff2NormalX.descriptorPool = descriptorPool.get();
	materialIff2NormalX.pipelinePass = &iff2PipelinePass;
	materialIff2NormalX.buildSets();
	bindIff2SSBO(materialIff2NormalX, normalXInfo);

	materialIff2NormalZ.descriptorPool = descriptorPool.get();
	materialIff2NormalZ.pipelinePass = &iff2PipelinePass;
	materialIff2NormalZ.buildSets();
	bindIff2SSBO(materialIff2NormalZ, normalZInfo);

	materialIff2Dx.descriptorPool = descriptorPool.get();
	materialIff2Dx.pipelinePass = &iff2PipelinePass;
	materialIff2Dx.buildSets();
	bindIff2SSBO(materialIff2Dx, dxInfo);

	materialIff2Dz.descriptorPool = descriptorPool.get();
	materialIff2Dz.pipelinePass = &iff2PipelinePass;
	materialIff2Dz.buildSets();
	bindIff2SSBO(materialIff2Dz, dzInfo);

	// pipeline pass ifft2
	auto shaderCodeSign = Loader::readFile("shaders/sign.comp.spv");

	pipelinePassSign.pipelineType = PipelineType::COMPUTE;
	pipelinePassSign.device = device.get();
	pipelinePassSign.addShader(shaderCodeSign, vk::ShaderStageFlagBits::eCompute);
	pipelinePassSign.buildLayout();

	vk::PipelineShaderStageCreateInfo computeShaderStageCreateInfoSign{ .stage = vk::ShaderStageFlagBits::eCompute,
		.module = pipelinePassSign.shaderModules[0].get(), .pName = "main" };
	vk::ComputePipelineCreateInfo computePipelineCreateInfoSign{ .stage = computeShaderStageCreateInfoSign,
		.layout = pipelinePassSign.layout.get(), };
	pipelinePassSign.pipeline = device->createComputePipelineUnique(pipelineCache.get(), computePipelineCreateInfoSign);

	// material sign
	materialSignAmpl.descriptorPool = descriptorPool.get();
	materialSignAmpl.pipelinePass = &pipelinePassSign;
	materialSignAmpl.buildSets();
	bindIff2SSBO(materialSignAmpl, amplInfo);

	materialSignNormalX.descriptorPool = descriptorPool.get();
	materialSignNormalX.pipelinePass = &pipelinePassSign;
	materialSignNormalX.buildSets();
	bindIff2SSBO(materialSignNormalX, normalXInfo);

	materialSignNormalZ.descriptorPool = descriptorPool.get();
	materialSignNormalZ.pipelinePass = &pipelinePassSign;
	materialSignNormalZ.buildSets();
	bindIff2SSBO(materialSignNormalZ, normalZInfo);

	materialSignDx.descriptorPool = descriptorPool.get();
	materialSignDx.pipelinePass = &pipelinePassSign;
	materialSignDx.buildSets();
	bindIff2SSBO(materialSignDx, dxInfo);

	materialSignDz.descriptorPool = descriptorPool.get();
	materialSignDz.pipelinePass = &pipelinePassSign;
	materialSignDz.buildSets();
	bindIff2SSBO(materialSignDz, dzInfo);

	// ocean
	ocean.setVertices([](uint32_t i, Vertex& v) {v.position *= 100.f; });

	pipelinePassOcean.pipelineType = PipelineType::PHONG;
	pipelinePassOcean.device = device.get();
	pipelinePassOcean.renderPass = renderPass.get();

	auto shaderCodeOceanVert = Loader::readFile("shaders/ocean.vert.spv");
	pipelinePassOcean.addShader(shaderCodeOceanVert, vk::ShaderStageFlagBits::eVertex);
	auto shaderCodeOceanFrag = Loader::readFile("shaders/ocean.frag.spv");
	pipelinePassOcean.addShader(shaderCodeOceanFrag, vk::ShaderStageFlagBits::eFragment);
	pipelinePassOcean.bindings[0][0].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	pipelinePassOcean.bindings[0][1].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	pipelinePassOcean.buildLayout();

	pipelineBuilder.vi.setVertexAttributeDescriptionCount(pipelineBuilder.vertexInputAttributeDescriptions.size());
	pipelineBuilder.vi.setVertexBindingDescriptionCount(pipelineBuilder.vertexInputBindingDescriptions.size());
	pipelinePassOcean.pipeline = pipelineBuilder.build(pipelinePassOcean.pipelineShaderStageCreateInfos, pipelinePassOcean.layout.get(), pipelinePassOcean.renderPass);

	// ocean material
	materialOcean.descriptorPool = descriptorPool.get();
	materialOcean.pipelinePass = &pipelinePassOcean;
	materialOcean.buildSets();

	lambda.create(*vmaAllocator, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	lambdaInfo.buffer = lambda.buffer;
	lambdaInfo.range = lambda.size;
	lambda.update(&oceanAmpl.lambda);

	vk::WriteDescriptorSet writeDescriptorSetOcean0{
	.dstSet = materialOcean.descriptorSets[0],
	.dstBinding = 2,
	.descriptorCount = 1,
	.descriptorType = vk::DescriptorType::eUniformBuffer,
	.pBufferInfo = &lambdaInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean0);

	vk::WriteDescriptorSet writeDescriptorSetOcean1{
		.dstSet = materialOcean.descriptorSets[0],
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
		.pBufferInfo = &modelMatrixBufferDescriptorInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean1);

	vk::WriteDescriptorSet writeDescriptorSetOcean2{
		.dstSet = materialOcean.descriptorSets[0],
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
		.pBufferInfo = &sceneParameterBufferDescriptorInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean2);

	vk::WriteDescriptorSet writeDescriptorSetOcean3{
	.dstSet = materialOcean.descriptorSets[1],
	.dstBinding = 0,
	.descriptorCount = 1,
	.descriptorType = vk::DescriptorType::eStorageBuffer,
	.pBufferInfo = &amplInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean3);

	vk::WriteDescriptorSet writeDescriptorSetOcean4{
		.dstSet = materialOcean.descriptorSets[1],
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &normalXInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean4);

	vk::WriteDescriptorSet writeDescriptorSetOcean5{
		.dstSet = materialOcean.descriptorSets[1],
		.dstBinding = 2,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &normalZInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean5);

	vk::WriteDescriptorSet writeDescriptorSetOcean6{
		.dstSet = materialOcean.descriptorSets[1],
		.dstBinding = 3,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &dxInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean6);

	vk::WriteDescriptorSet writeDescriptorSetOcean7{
		.dstSet = materialOcean.descriptorSets[1],
		.dstBinding = 4,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &dzInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean7);

	int index = 0;
	for (const auto& r : renderables)
	{
		index += r.mesh->data.size();
	}
	indexB.create(*vmaAllocator, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	indexB.update(&index);
	indexBInfo.buffer = indexB.buffer;
	indexBInfo.range = indexB.size;
	vk::WriteDescriptorSet writeDescriptorSetOcean8{
	.dstSet = materialOcean.descriptorSets[0],
	.dstBinding = 3,
	.descriptorCount = 1,
	.descriptorType = vk::DescriptorType::eUniformBuffer,
	.pBufferInfo = &indexBInfo };
	materialOcean.writeDescriptorSets.push_back(writeDescriptorSetOcean8);
	renderables.emplace_back(&ocean, &materialOcean);

	materialOcean1.descriptorPool = descriptorPool.get();
	materialOcean1.pipelinePass = &pipelinePassOcean;
	materialOcean1.buildSets();
	materialOcean1.writeDescriptorSets = materialOcean.writeDescriptorSets;
	index = 0;
	for (const auto& r : renderables)
	{
		index += r.mesh->data.size();
	}
	indexB1.create(*vmaAllocator, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	indexB1.update(&index);
	indexBInfo1.buffer = indexB1.buffer;
	indexBInfo1.range = indexB1.size;
	materialOcean1.writeDescriptorSets.back().pBufferInfo = &indexBInfo1;
	renderables.emplace_back(&ocean, &materialOcean1);
	materialOcean1.writeDescriptorSets[0].dstSet = materialOcean1.descriptorSets[0];
	materialOcean1.writeDescriptorSets[1].dstSet = materialOcean1.descriptorSets[0];
	materialOcean1.writeDescriptorSets[2].dstSet = materialOcean1.descriptorSets[0];
	materialOcean1.writeDescriptorSets[3].dstSet = materialOcean1.descriptorSets[1];
	materialOcean1.writeDescriptorSets[4].dstSet = materialOcean1.descriptorSets[1];
	materialOcean1.writeDescriptorSets[5].dstSet = materialOcean1.descriptorSets[1];
	materialOcean1.writeDescriptorSets[6].dstSet = materialOcean1.descriptorSets[1];
	materialOcean1.writeDescriptorSets[7].dstSet = materialOcean1.descriptorSets[1];
	materialOcean1.writeDescriptorSets[8].dstSet = materialOcean1.descriptorSets[0];
	renderables.back().transformMatrix.block<3, 1>(0, 3) = Eigen::Vector3f(200.f, 0.f, 0.f);


	vk::CommandPoolCreateInfo commandPoolCreateInfo{ .queueFamilyIndex = computeQueueFamilyIndex };

	computeCommandPool = device->createCommandPoolUnique(commandPoolCreateInfo);

	// allocate compute command buffer
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{ .commandPool = computeCommandPool.get(),
		.level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
	computeCommandBuffers = device->allocateCommandBuffersUnique(commandBufferAllocateInfo);
	{
		vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		computeCommandBuffers[0]->begin(commandBufferBeginInfo);

		// ampl
		computeCommandBuffers[0]->bindPipeline(vk::PipelineBindPoint::eCompute, materialAmpl.pipelinePass->pipeline.get());
		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialAmpl.pipelinePass->layout.get(), 0, materialAmpl.descriptorSets.size(), materialAmpl.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		vk::MemoryBarrier2KHR memoryBarrier = {
			.srcStageMask =vk::PipelineStageFlagBits2KHR::e2ComputeShader,
			.srcAccessMask =vk::AccessFlagBits2KHR::e2ShaderWrite,
			.dstStageMask = vk::PipelineStageFlagBits2KHR::e2ComputeShader,
			.dstAccessMask = vk::AccessFlagBits2KHR::e2ShaderRead };

		vk::DependencyInfoKHR dependencyInfo = {
			.memoryBarrierCount = 1,
			.pMemoryBarriers = &memoryBarrier
		};
		computeCommandBuffers[0]->pipelineBarrier2KHR(dependencyInfo);

		// ifft2
		computeCommandBuffers[0]->bindPipeline(vk::PipelineBindPoint::eCompute, materialIff2Ampl.pipelinePass->pipeline.get());
		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialIff2Ampl.pipelinePass->layout.get(), 0, materialIff2Ampl.descriptorSets.size(), materialIff2Ampl.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialIff2NormalX.pipelinePass->layout.get(), 0, materialIff2NormalX.descriptorSets.size(), materialIff2NormalX.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialIff2NormalZ.pipelinePass->layout.get(), 0, materialIff2NormalZ.descriptorSets.size(), materialIff2NormalZ.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialIff2Dx.pipelinePass->layout.get(), 0, materialIff2Dx.descriptorSets.size(), materialIff2Dx.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialIff2Dz.pipelinePass->layout.get(), 0, materialIff2Dz.descriptorSets.size(), materialIff2Dz.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->pipelineBarrier2KHR(dependencyInfo);

		// sign
		computeCommandBuffers[0]->bindPipeline(vk::PipelineBindPoint::eCompute, materialSignAmpl.pipelinePass->pipeline.get());
		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialSignAmpl.pipelinePass->layout.get(), 0, materialSignAmpl.descriptorSets.size(), materialSignAmpl.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialSignNormalX.pipelinePass->layout.get(), 0, materialSignNormalX.descriptorSets.size(), materialSignNormalX.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialSignNormalZ.pipelinePass->layout.get(), 0, materialSignNormalZ.descriptorSets.size(), materialSignNormalZ.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialSignDx.pipelinePass->layout.get(), 0, materialSignDx.descriptorSets.size(), materialSignDx.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute, materialSignDz.pipelinePass->layout.get(), 0, materialSignDz.descriptorSets.size(), materialSignDz.descriptorSets.data(), 0, nullptr);
		computeCommandBuffers[0]->dispatch(1, 1, 1);

		computeCommandBuffers[0]->end();
	}

	computeSubmitInfo.commandBufferCount = computeCommandBuffers.size();
	computeSubmitInfo.pCommandBuffers = &computeCommandBuffers[0].get();
}
