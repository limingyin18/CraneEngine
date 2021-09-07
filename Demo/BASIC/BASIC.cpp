#include "BASIC.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

BASIC::BASIC(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
	camera.target = Vector3f{ 0.f,0.1f, 0.f };
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

	LOGI("创建anime纹理图像")
	{
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

		tie(imageAnime, imageViewAnime) = createTextureImage(texWidth, texHeight, texChannels, pixels.data());
		descriptorImageInfoAnime.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		descriptorImageInfoAnime.imageView = imageViewAnime.get();
		descriptorImageInfoAnime.sampler = textureSampler.get();
	}

	LOGI("创建冯氏着色模型管线")
	{
		pipelinePassPhong.device = device.get();

		pipelinePassPhong.renderPass = renderPass.get();

		auto vertShaderCode = Loader::readFile("shaders/phong.vert.spv");
		pipelinePassPhong.addShader(vertShaderCode, vk::ShaderStageFlagBits::eVertex);
		auto fragShaderCode = Loader::readFile("shaders/phong.frag.spv");
		pipelinePassPhong.addShader(fragShaderCode, vk::ShaderStageFlagBits::eFragment);
		pipelinePassPhong.bindings[0][0].descriptorType = vk::DescriptorType::eUniformBufferDynamic;

		pipelinePassPhong.buildDescriptorSetLayout();

		pipelinePassPhong.buildPipelineLayout();
		pipelinePassPhong.buildPipeline(pipelineBuilder);
	}

	LOGI("创建chessboard材质")
	{
		materialPhongChessboard.descriptorPool = descriptorPool.get();
		materialPhongChessboard.pipelinePass = &pipelinePassPhong;
		materialPhongChessboard.buildDescriptorSets();

		// 场景参数
		vk::WriteDescriptorSet writeDescriptorSet0B0{
			.dstSet = materialPhongChessboard.descriptorSets[0],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
			.pBufferInfo = &sceneParameterBufferDescriptorInfo };
		materialPhongChessboard.writeDescriptorSets.push_back(writeDescriptorSet0B0);

		// instance
		vk::WriteDescriptorSet writeDescriptorSet0B2{
			.dstSet = materialPhongChessboard.descriptorSets[0],
			.dstBinding = 2,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferInfoInstanceID };
		materialPhongChessboard.writeDescriptorSets.push_back(writeDescriptorSet0B2);



		// 模型位姿
		vk::WriteDescriptorSet writeDescriptorSet0B1{
		.dstSet = materialPhongChessboard.descriptorSets[0],
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &modelMatrixBufferDescriptorInfo };
		materialPhongChessboard.writeDescriptorSets.push_back(writeDescriptorSet0B1);

		// 纹理
		vk::WriteDescriptorSet writeDescriptorSet1B0{
			.dstSet = materialPhongChessboard.descriptorSets[1],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &descriptorImageInfoBlank };
		materialPhongChessboard.writeDescriptorSets.push_back(writeDescriptorSet1B0);
	}
	chessboard.setVertices([](uint32_t, Vertex& v) {v.position *= 100; });
	renderables.emplace_back(&chessboard, &materialPhongChessboard);

	LOGI("创建box材质")
	{
		materialPhongBox.descriptorPool = descriptorPool.get();
		materialPhongBox.pipelinePass = &pipelinePassPhong;
		materialPhongBox.buildDescriptorSets();

		// 场景参数
		vk::WriteDescriptorSet writeDescriptorSet0B0{
			.dstSet = materialPhongBox.descriptorSets[0],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
			.pBufferInfo = &sceneParameterBufferDescriptorInfo };
		materialPhongBox.writeDescriptorSets.push_back(writeDescriptorSet0B0);

		// instance
		vk::WriteDescriptorSet writeDescriptorSet0B2{
			.dstSet = materialPhongBox.descriptorSets[0],
			.dstBinding = 2,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferInfoInstanceID };
		materialPhongBox.writeDescriptorSets.push_back(writeDescriptorSet0B2);

		// 模型位姿
		vk::WriteDescriptorSet writeDescriptorSet0B1{
		.dstSet = materialPhongBox.descriptorSets[0],
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &modelMatrixBufferDescriptorInfo };
		materialPhongBox.writeDescriptorSets.push_back(writeDescriptorSet0B1);

		// 纹理
		vk::WriteDescriptorSet writeDescriptorSet1B0{
			.dstSet = materialPhongBox.descriptorSets[1],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &descriptorImageInfoAnime };
		materialPhongBox.writeDescriptorSets.push_back(writeDescriptorSet1B0);
	}
	renderables.emplace_back(&box, &materialPhongBox);

	for (uint32_t i = 0; i < 10000*10; ++i)
	{
		renderables.emplace_back(&box, &materialPhongBox);
		
		float x = 100*normal_dist(rand_generator);
		float y = 0*normal_dist(rand_generator);
		float z = 100*normal_dist(rand_generator);

		renderables.back().transformMatrix.block<3, 1>(0, 3) = Vector3f{ x, y, z };
	}

	LOGI("创建剔除管线")
	{
		pipelinePassCull.device = device.get();
		auto shaderCodeCull = Loader::readFile("shaders/cull.comp.spv.");
		pipelinePassCull.addShader(shaderCodeCull, vk::ShaderStageFlagBits::eCompute);
		pipelinePassCull.buildDescriptorSetLayout();
		pipelinePassCull.buildPipelineLayout();
		pipelinePassCull.buildPipeline(nullptr);

		materialCull.descriptorPool = descriptorPool.get();
		materialCull.pipelinePass = &pipelinePassCull;
		materialCull.buildDescriptorSets();

		
		// cull data
		vk::WriteDescriptorSet writeDescriptorSet0B5{
			.dstSet = materialCull.descriptorSets[0],
			.dstBinding = 5,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferInfoCullData };
		materialCull.writeDescriptorSets.push_back(writeDescriptorSet0B5);

		// object candidate
		vk::WriteDescriptorSet writeDescriptorSet0B0{
			.dstSet = materialCull.descriptorSets[0],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferInfoCullObjCandidate };
		materialCull.writeDescriptorSets.push_back(writeDescriptorSet0B0);

		// draw command
		vk::WriteDescriptorSet writeDescriptorSet0B1{
			.dstSet = materialCull.descriptorSets[0],
			.dstBinding = 1,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferInfoIndirect };
		materialCull.writeDescriptorSets.push_back(writeDescriptorSet0B1);

		// flat batch
		vk::WriteDescriptorSet writeDescriptorSet0B2{
			.dstSet = materialCull.descriptorSets[0],
			.dstBinding = 2,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferDrawsFlat };
		materialCull.writeDescriptorSets.push_back(writeDescriptorSet0B2);

		// instance id
		vk::WriteDescriptorSet writeDescriptorSet0B3{
			.dstSet = materialCull.descriptorSets[0],
			.dstBinding = 3,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferInfoInstanceID };
		materialCull.writeDescriptorSets.push_back(writeDescriptorSet0B3);
	}
}
