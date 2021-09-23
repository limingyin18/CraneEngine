#include "HAIR.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

HAIR::HAIR(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
	preferPresentMode = vk::PresentModeKHR::eFifo;
	camera.target = Vector3f{ 0.f, 1.8f, 0.f };
	camera.cameraMoveSpeed = 0.1f;
}

HAIR::~HAIR()
{
	vkDeviceWaitIdle(device.get());
}

void HAIR::updateApp()
{
	updateEngine();

	pbd.dt = dt;
	pbd.run();

	// update hair particle position
	for (uint32_t i = 0; i < verticesHair.size(); ++i)
	{
		verticesHair[i] = pbd.rigidbodies[hairPhysicsIndexOffset + i]->position;
	}
	// update tangent
	Vector3f tangentFirst = verticesHair[0] - verticesHair[1];
	tangentFirst.normalize();
	tangentsHair[0] = tangentFirst;
	for (uint32_t i = 1; i < verticesHair.size() - 1; ++i)
	{
		Vector3f tangent = (verticesHair[i - 1] - verticesHair[i + 1]) / 2.f;
		tangent.normalize();
		tangentsHair[i] = tangent;
	}
	Vector3f tangentLast = verticesHair[verticesHair.size() - 2] - verticesHair[verticesHair.size() - 1];
	tangentLast.normalize();
	tangentsHair[verticesHair.size() - 1] = tangentLast;

	// normal
	Vector3f normalFirst = tangentsHair[0] - tangentsHair[1];
	normalFirst.normalize();
	normalsHair[0] = normalFirst;
	for (uint32_t i = 1; i < verticesHair.size() - 1; ++i)
	{
		Vector3f normal = (tangentsHair[i - 1] - tangentsHair[i + 1]) / 2.f;
		normal.normalize();
		normalsHair[i] = normal;
	}
	Vector3f normalLast = tangentsHair[verticesHair.size() - 2] - tangentsHair[verticesHair.size() - 1];
	normalLast.normalize();
	normalsHair[verticesHair.size() - 1] = normalLast;

	string name = "hair";
	for (uint32_t i = 0; i < verticesHair.size(); ++i)
	{
		loadMeshs[name]->data[i*2].position = (verticesHair[i] - radius * normalsHair[i]);
		loadMeshs[name]->data[i*2+1].position = (verticesHair[i] + radius * normalsHair[i]);
	}
	loadMeshs[name].get()->recomputeNormals();

	/*
	for(size_t i = 0; i < renderables.size(); ++i)
	{
		Matrix4f t = (Translation3f(pbd.rigidbodies[i]->position) * pbd.rigidbodies[i]->rotation).matrix();
		renderables[i].transformMatrix = t;
	}

	auto modelMatrixPtr = modelMatrix.data();
	for (auto& v : renderables)
	{
		*(reinterpret_cast<Eigen::Matrix4f*>(modelMatrixPtr)) = *v.transformMatrix;
		modelMatrixPtr += modelMatrixOffset;
	}
	modelMatrixBuffer.update(modelMatrix.data());
	*/

	for (size_t i = 0; i < hair.mesh->data.size(); i++)
		vertices[hairRenderableOffset+i] = hair.mesh->data[i];
	vertBuff.update(vertices.data());
}

void HAIR::setImgui()
{
	static size_t count = 0;

	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowPos(ImVec2{ 0.f, 0.f });
	ImGui::Text("count:\t%d", count++);
	ImGui::Text("FPS:\t%.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("camera position: %5.1f %5.1f %5.1f", camera.getCameraPos().x(), camera.getCameraPos().y(), camera.getCameraPos().z());
	ImGui::Text("camera rotation: %5.1f %5.1f %5.1f", camera.rotation.x(), camera.rotation.y(), camera.rotation.z());
	ImGui::End();
}

void HAIR::createAssetApp()
{
	LOGI("创建应用资产");

	SDL2_IMGUI_BASE::createAssetApp();

	LOGI("创建剔除管线")
	{
		pipelinePassCull.device = device.get();
		auto shaderCodeCull = Loader::readFile("shaders/cull.comp.spv.");
		pipelinePassCull.addShader(shaderCodeCull, vk::ShaderStageFlagBits::eCompute);
		pipelinePassCull.buildDescriptorSetLayout();
		pipelinePassCull.buildPipelineLayout();
		pipelinePassCull.buildPipeline(nullptr);

		materialBuilder.descriptorPool = descriptorPool.get();
		materialBuilder.pipelinePass = &pipelinePassCull;

		materialCull = materialBuilder.build();
		materialCull.writeDescriptorSets[0][0].pBufferInfo = &descriptorBufferInfoCullObjCandidate; // object candidate
		materialCull.writeDescriptorSets[0][1].pBufferInfo = &descriptorBufferInfoIndirect; // draw command
		materialCull.writeDescriptorSets[0][2].pBufferInfo = &descriptorBufferDrawsFlat; // flat batch
		materialCull.writeDescriptorSets[0][3].pBufferInfo = &descriptorBufferInfoInstanceID; // instance id
		materialCull.writeDescriptorSets[0][5].pBufferInfo = &descriptorBufferInfoCullData; // cull data
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

	LOGI("创建冯氏着色材质构建工厂")
	{
		materialBuilderPhong.descriptorPool = descriptorPool.get();
		materialBuilderPhong.pipelinePass = &pipelinePassPhong;
		materialBuilderPhong.sceneParameterBufferDescriptorInfo = &sceneParameterBufferDescriptorInfo;
		materialBuilderPhong.modelMatrixBufferDescriptorInfo = &modelMatrixBufferDescriptorInfo;
		materialBuilderPhong.descriptorBufferInfoInstanceID = &descriptorBufferInfoInstanceID;
		materialBuilderPhong.descriptorImageInfoBlank = &descriptorImageInfoBlank;
	}

	createChessboard();
	createHair();
}
