#include "BASIC.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

BASIC::BASIC(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win) 
{
	camera.target = Vector3f{ 0.f, 2.5f, 0.f };
	camera.rotation[0] = -0.2f;
}

BASIC::~BASIC()
{
	device->waitIdle();
}

void BASIC::updateApp()
{
	updateEngine();
}

void BASIC::setImgui()
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

void BASIC::createAssetApp()
{
	LOGI("创建应用资产");

	SDL2_IMGUI_BASE::createAssetApp();
	
	auto &pipelinePass = pipelinePasss[PipelineType::PHONG];
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

	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{
				.descriptorPool = descriptorPool.get(),
				.descriptorSetCount = static_cast<uint32_t>(pipelinePass.setLayouts.size()),
				.pSetLayouts = pipelinePass.descriptorSetLayouts.data() };
	materialPhong.descriptorSets = device->allocateDescriptorSets(descriptorSetAllocateInfo);

	materialPhong.imageView = textureImageView.get();
	materialPhong.imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	materialPhong.imageInfo.imageView =	materialPhong.imageView;
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

	materialPhong.pipelinePass = &pipelinePass;

	renderables.emplace_back(&floorEnv, &materialPhong);
	renderables.emplace_back(&cube, &materialPhong);
}
