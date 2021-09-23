#include "CLOTH.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

CLOTH::CLOTH(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
	//preferPresentMode = vk::PresentModeKHR::eFifo;
	camera.target = Vector3f{ 0.f, 1.8f, 0.f };
	camera.rotation[0] = -0.0f;
	camera.cameraMoveSpeed = 1.f;
}

CLOTH::~CLOTH()
{
	vkDeviceWaitIdle(device.get());
}

void CLOTH::updateApp()
{
	updateEngine();

	// physics update
	//pbd.dt = 0.1f;
	pbd.dt = dt;
	//pbd.rigidbodies[1]->rotation *= Quaternionf(AngleAxisf(1.f*dt, Vector3f(1.0f, 1.0f, 1.0f).normalized()));
	Vector3f u{ 2.f, 0.f, 0.f };
	float drag = 1.f;
	float lift = 1.f;

	for(size_t i = 0; i < cloak.mesh->indices.size(); i = i + 3)
	{
		size_t indexA = cloak.mesh->indices[i];
		size_t indexB = cloak.mesh->indices[i+1];
		size_t indexC = cloak.mesh->indices[i+2];
		CranePhysics::Particle &a = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexA].get());
		CranePhysics::Particle &b = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexB].get());
		CranePhysics::Particle &c = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexC].get());

		Vector3f AB = a.positionPrime - b.positionPrime;
		Vector3f AC = a.positionPrime - c.positionPrime;
		Vector3f ABC = AB.cross(AC);
		float area = ABC.norm() / 2;

		Vector3f vRel = (a.velocity + b.velocity + c.velocity)/3 - u;
		Vector3f n = ABC.normalized();
		n = vRel.dot(n) > 0 ? n : -n;

		Vector3f fd = drag * vRel.squaredNorm() * area * vRel.dot(n) * -vRel;
		float cosq = vRel.normalized().cross(n).norm();
		Vector3f fl = lift * vRel.squaredNorm() * area * cosq * (n.cross(vRel).cross(vRel));

		a.velocity += dt * a.invMass *(fd+fl);

		b.velocity += dt * b.invMass *(fd+fl);

		c.velocity += dt * c.invMass *(fd+fl);
	}

	pbd.run();

	cloak.mesh->setVertices([this](uint32_t i, Crane::Vertex &v){v.position = this->pbd.rigidbodies[offsetCloak+i]->position - this->cloak.position;});
	cloak.mesh->recomputeNormals();

	sphereTest.setPosition(pbd.rigidbodies.back()->position);
	LOGI("位置 {}, {}, {}", sphereTest.position.x(), sphereTest.position.y(), sphereTest.position.z());
	LOGI("速度 {}, {}, {}", pbd.rigidbodies.back()->velocity.x(), pbd.rigidbodies.back()->velocity.y(), pbd.rigidbodies.back()->velocity.z());

	/*
	for(size_t i = 0; i < renderables.size(); ++i)
	{
		Matrix4f t = (Translation3f(pbd.rigidbodies[i]->position) * pbd.rigidbodies[i]->rotation).matrix();
		renderables[i].transformMatrix = t;
	}*/

	auto modelMatrixPtr = modelMatrix.data();
	for (auto &v : renderables)
	{
		*(reinterpret_cast<Eigen::Matrix4f *>(modelMatrixPtr)) = *v.transformMatrix;
		modelMatrixPtr += modelMatrixOffset;
	}
	modelMatrixBuffer.update(modelMatrix.data());

	size_t offset = chessboard.mesh->data.size();
	for (size_t i = 0; i < cloak.mesh->data.size(); i++)
		vertices[offset++] = cloak.mesh->data[i];
	vertBuff.update(vertices.data());
}

void CLOTH::setImgui()
{
	static size_t count = 0;

	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowPos(ImVec2{ 0.f, 0.f });
	ImGui::Text("count:\t%d", count++);
	ImGui::Text("FPS:\t%.3f ms/frame (%.1f FPS)", dt, 1.f / dt);
	ImGui::Text("camera position: %5.1f %5.1f %5.1f", camera.getCameraPos().x(), camera.getCameraPos().y(), camera.getCameraPos().z());
	ImGui::Text("camera rotation: %5.1f %5.1f %5.1f", camera.rotation.x(), camera.rotation.y(), camera.rotation.z());
	ImGui::End();
}

void CLOTH::createAssetApp()
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
	createCloak();
	createDragon();
	createSoldiers();
	createCubeTest();
	createSphereTest();
}
