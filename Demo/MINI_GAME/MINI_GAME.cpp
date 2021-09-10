#include "MINI_GAME.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

MINI_GAME::MINI_GAME(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
	preferPresentMode = vk::PresentModeKHR::eFifo;
	camera.target = Vector3f{ 0.f, 1.0f, 0.f };
	camera.rotation[0] = -0.0f;
	camera.cameraMoveSpeed = 0.1f;
}

MINI_GAME::~MINI_GAME()
{
	vkDeviceWaitIdle(device.get());
}

void MINI_GAME::updateApp()
{
	updateEngine();

	renderables[2].transformMatrix.block<3, 1>(0, 3) = Vector3f{ 10*sin(dtAll), 0.f, 10*cos(dtAll) };
	// physics update
	pbd.dt = dt;
	//pbd.rigidbodies[1]->rotation *= Quaternionf(AngleAxisf(1.f*dt, Vector3f(1.0f, 1.0f, 1.0f).normalized()));
	Vector3f u{ 2.f, 0.f, 0.f };
	float drag = 1.f;
	float lift = 1.f;

	/*
	for(size_t i = 0; i < cloak.indices.size(); i = i + 3)
	{
		size_t indexA = cloak.indices[i];
		size_t indexB = cloak.indices[i+1];
		size_t indexC = cloak.indices[i+2];
		CranePhysics::Particle &a = *dynamic_cast<Particle*>(pbd.rigidbodies[1+indexA].get());
		CranePhysics::Particle &b = *dynamic_cast<Particle*>(pbd.rigidbodies[1+indexB].get());
		CranePhysics::Particle &c = *dynamic_cast<Particle*>(pbd.rigidbodies[1+indexC].get());

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

	//for(size_t i = 0; i < renderables.size(); ++i)
	//{
	//	Matrix4f t = (Translation3f(pbd.rigidbodies[i]->position) * pbd.rigidbodies[i]->rotation).matrix();
	//	renderables[i].transformMatrix = t;
	//}

	auto modelMatrixPtr = modelMatrix.data();
	for (auto &v : renderables)
	{
		*(reinterpret_cast<Eigen::Matrix4f *>(modelMatrixPtr)) = v.transformMatrix;
		modelMatrixPtr += modelMatrixOffset;
	}
	modelMatrixBuffer.update(modelMatrix.data());

	renderables[1].mesh->setVertices([this](uint32_t i, Crane::Vertex &v){v.position = this->pbd.rigidbodies[1+i]->position - modelCloak;});
	renderables[1].mesh->recomputeNormals(renderables[1].mesh->data);
	size_t offset = 0;
	for (auto &v : renderables)
	{
		for (size_t i = 0; i < v.mesh->data.size(); i++)
			vertices[offset++] = v.mesh->data[i];
	}
	vertBuff.update(vertices.data());
	*/
	//for(size_t i = 0; i < renderables.size(); ++i)
//{
//	Matrix4f t = (Translation3f(pbd.rigidbodies[i]->position) * pbd.rigidbodies[i]->rotation).matrix();
//	renderables[i].transformMatrix = t;
//}

	auto modelMatrixPtr = modelMatrix.data();
	for (auto& v : renderables)
	{
		*(reinterpret_cast<Eigen::Matrix4f*>(modelMatrixPtr)) = v.transformMatrix;
		modelMatrixPtr += modelMatrixOffset;
	}
	modelMatrixBuffer.update(modelMatrix.data());
}

void MINI_GAME::setImgui()
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

void MINI_GAME::createAssetApp()
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

	LOGI("创建chessboard")
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

		chessboard.setVertices([](uint32_t, Vertex& v) {v.position *= 100; });
		renderables.emplace_back(&chessboard, &materialPhongChessboard);
	}

	LOGI("创建cloak")
	{
		materialPhongCloak.descriptorPool = descriptorPool.get();
		materialPhongCloak.pipelinePass = &pipelinePassPhong;
		materialPhongCloak.buildDescriptorSets();

		// 场景参数
		vk::WriteDescriptorSet writeDescriptorSet0B0{
			.dstSet = materialPhongCloak.descriptorSets[0],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
			.pBufferInfo = &sceneParameterBufferDescriptorInfo };
		materialPhongCloak.writeDescriptorSets.push_back(writeDescriptorSet0B0);

		// instance
		vk::WriteDescriptorSet writeDescriptorSet0B2{
			.dstSet = materialPhongCloak.descriptorSets[0],
			.dstBinding = 2,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &descriptorBufferInfoInstanceID };
		materialPhongCloak.writeDescriptorSets.push_back(writeDescriptorSet0B2);

		// 模型位姿
		vk::WriteDescriptorSet writeDescriptorSet0B1{
		.dstSet = materialPhongCloak.descriptorSets[0],
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.pBufferInfo = &modelMatrixBufferDescriptorInfo };
		materialPhongCloak.writeDescriptorSets.push_back(writeDescriptorSet0B1);

		// 纹理
		vk::WriteDescriptorSet writeDescriptorSet1B0{
			.dstSet = materialPhongCloak.descriptorSets[1],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &descriptorImageInfoLilac };
		materialPhongCloak.writeDescriptorSets.push_back(writeDescriptorSet1B0);


		renderables.emplace_back(&cloak, &materialPhongCloak);
		renderables.back().transformMatrix.block<3, 1>(0, 3) = modelCloak;
	}

	LOGI("读取罗马士兵模型")
	{
		/*
		assets::AssetFile file;
		if (!assets::load_binaryfile((string("assets/RomanSoldier/") + "Roman-Soldier.pfb").c_str(), file))
			throw std::runtime_error(string("Error when loading mesh "));
		assets::PrefabInfo prefabInfo = assets::read_prefab_info(&file);

		load_prefab(prefabInfo, Eigen::Matrix4f::Identity(), renderables, pipelinePassPhong);
		*/

		string name = "Roman-Soldier";
		assets::AssetFile file;
		if (!assets::load_binaryfile((string("assets/RomanSoldier/") + "Roman-Soldier.mesh").c_str(), file))
			throw std::runtime_error(string("Error when loading mesh "));
		assets::MeshInfo meshInfo = assets::read_mesh_info(&file);

		if (loadMeshs.find(name) == loadMeshs.end())
		{
			loadMeshs[name] = make_shared<MeshBase>();
			loadMeshs[name]->data.resize(meshInfo.vertexBuferSize / sizeof(Vertex));
			loadMeshs[name]->indices.resize(meshInfo.indexBuferSize / sizeof(uint32_t));
			assets::unpack_mesh(&meshInfo, file.binaryBlob.data(), file.binaryBlob.size(),
				reinterpret_cast<char*>(loadMeshs[name]->data.data()),
				reinterpret_cast<char*>(loadMeshs[name]->indices.data()));

			materials[name].descriptorPool = descriptorPool.get();
			materials[name].pipelinePass = &pipelinePassPhong;
			materials[name].buildDescriptorSets();

			// 场景参数
			vk::WriteDescriptorSet writeDescriptorSet0B0{
				.dstSet = materials[name].descriptorSets[0],
				.dstBinding = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
				.pBufferInfo = &sceneParameterBufferDescriptorInfo };
			materials[name].writeDescriptorSets.push_back(writeDescriptorSet0B0);

			// instance
			vk::WriteDescriptorSet writeDescriptorSet0B2{
				.dstSet = materials[name].descriptorSets[0],
				.dstBinding = 2,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eStorageBuffer,
				.pBufferInfo = &descriptorBufferInfoInstanceID };
			materials[name].writeDescriptorSets.push_back(writeDescriptorSet0B2);

			// 模型位姿
			vk::WriteDescriptorSet writeDescriptorSet0B1{
			.dstSet = materials[name].descriptorSets[0],
			.dstBinding = 1,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &modelMatrixBufferDescriptorInfo };
			materials[name].writeDescriptorSets.push_back(writeDescriptorSet0B1);
		}

		vk::WriteDescriptorSet writeDescriptorSet1B0{
			.dstSet = materials[name].descriptorSets[1],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &descriptorImageInfoBlank };
		materials[name].writeDescriptorSets.push_back(writeDescriptorSet1B0);
		loadMeshs[name].get()->setVertices([](uint32_t i, Vertex& v) {v.color = { 1.f, 1.f, 1.f }; });
		loadMeshs[name].get()->recomputeNormals(loadMeshs[name].get()->data);

		for (uint32_t i = 0; i < 1 * 1; ++i)
		{
			renderables.emplace_back(loadMeshs[name].get(), &materials[name]);

			float x = 0 * normal_dist(rand_generator);
			float y = 0 * normal_dist(rand_generator);
			float z = 0 * normal_dist(rand_generator);

			renderables.back().transformMatrix.block<3, 1>(0, 3) = Vector3f{ x, y, z };
		}
	}

	LOGI("读取龙模型")
	{
		string name = "Dragon";
		assets::AssetFile file;
		if (!assets::load_binaryfile((string("assets/") + "stanfordDragon.mesh").c_str(), file))
			throw std::runtime_error(string("Error when loading mesh "));
		assets::MeshInfo meshInfo = assets::read_mesh_info(&file);

		if (loadMeshs.find(name) == loadMeshs.end())
		{
			loadMeshs[name] = make_shared<MeshBase>();
			loadMeshs[name]->data.resize(meshInfo.vertexBuferSize / sizeof(Vertex));
			loadMeshs[name]->indices.resize(meshInfo.indexBuferSize / sizeof(uint32_t));
			assets::unpack_mesh(&meshInfo, file.binaryBlob.data(), file.binaryBlob.size(),
				reinterpret_cast<char*>(loadMeshs[name]->data.data()),
				reinterpret_cast<char*>(loadMeshs[name]->indices.data()));

			materials[name].descriptorPool = descriptorPool.get();
			materials[name].pipelinePass = &pipelinePassPhong;
			materials[name].buildDescriptorSets();

			// 场景参数
			vk::WriteDescriptorSet writeDescriptorSet0B0{
				.dstSet = materials[name].descriptorSets[0],
				.dstBinding = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
				.pBufferInfo = &sceneParameterBufferDescriptorInfo };
			materials[name].writeDescriptorSets.push_back(writeDescriptorSet0B0);

			// instance
			vk::WriteDescriptorSet writeDescriptorSet0B2{
				.dstSet = materials[name].descriptorSets[0],
				.dstBinding = 2,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eStorageBuffer,
				.pBufferInfo = &descriptorBufferInfoInstanceID };
			materials[name].writeDescriptorSets.push_back(writeDescriptorSet0B2);

			// 模型位姿
			vk::WriteDescriptorSet writeDescriptorSet0B1{
			.dstSet = materials[name].descriptorSets[0],
			.dstBinding = 1,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &modelMatrixBufferDescriptorInfo };
			materials[name].writeDescriptorSets.push_back(writeDescriptorSet0B1);
		}

		vk::WriteDescriptorSet writeDescriptorSet1B0{
			.dstSet = materials[name].descriptorSets[1],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &descriptorImageInfoBlank };
		materials[name].writeDescriptorSets.push_back(writeDescriptorSet1B0);
		loadMeshs[name].get()->setVertices([](uint32_t i, Vertex& v) {v.color = { 1.f, 1.f, 1.f }; });
		loadMeshs[name].get()->recomputeNormals(loadMeshs[name].get()->data);

		for (uint32_t i = 0; i < 1 * 1; ++i)
		{
			renderables.emplace_back(loadMeshs[name].get(), &materials[name]);

			float x = 0 * normal_dist(rand_generator);
			float y = 5.f + 0 * normal_dist(rand_generator);
			float z = -5.f * normal_dist(rand_generator);
			renderables.back().transformMatrix(0, 0) = 10.f;
			renderables.back().transformMatrix(1, 1) = 10.f;
			renderables.back().transformMatrix(2, 2) = 10.f;
			renderables.back().transformMatrix.block<3, 1>(0, 3) = Vector3f{ x, y, z };

			Eigen::Matrix4f rot = (
				AngleAxis<float>(90, Vector3f{ 1, 0, 0 }) *
				Translation<float, 3>(-Vector3f{ 0, 0, 0 })
				)
				.matrix();

			renderables.back().transformMatrix =  renderables.back().transformMatrix * rot;
		}
	}


	// physics
	auto cube0 = std::make_shared<CranePhysics::Cube>();
	cube0->invMass = 0.f;
	cube0->position = modelChessboard;
	cube0->width = 100.f;
	cube0->depth = 100.f;
	cube0->height = 0.1f;
	pbd.rigidbodies.push_back(cube0);

	for (uint32_t i = 0; i < cloak.data.size(); ++i)
	{
		auto particle = std::make_shared<CranePhysics::Particle>();
		particle->radius = 0.0005f;
		particle->position = cloak.data[i].position + modelCloak;
		particle->positionPrime = particle->position;

		particle->invMass = (i - i % 11) / 11.f;
		pbd.rigidbodies.push_back(particle);
	}

	/*
	for (uint32_t i = 0; i < 11; ++i)
	{
		for (uint32_t j = 1; j < 11; ++j)
		{
			float d = (pbd.rigidbodies[2+i]->position - pbd.rigidbodies[2+j * 11 + i]->position).norm();
			pbd.constraints.emplace_back(std::make_shared<LongRangeAttachmentConstraint>(*pbd.rigidbodies[2+i+(j-1)*11], *pbd.rigidbodies[2+j*11+i], d, 1.0f));
		}
	}*/

	float compress = 1.0f, stretch = 1.0f;
	for (uint32_t i = 0; i < cloak.indices.size(); i = i + 3)
	{
		size_t indexA = cloak.indices[i];
		size_t indexB = cloak.indices[i + 1];
		size_t indexC = cloak.indices[i + 2];

		CranePhysics::Rigidbody& a = *(pbd.rigidbodies[1 + indexA]);
		CranePhysics::Rigidbody& b = *(pbd.rigidbodies[1 + indexB]);
		CranePhysics::Rigidbody& c = *(pbd.rigidbodies[1 + indexC]);
		float distAB = (a.position - b.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(a, b, distAB, compress, stretch));

		float distAC = (a.position - c.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(a, c, distAC, compress, stretch));

		float distBC = (b.position - c.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(b, c, distBC, compress, stretch));
	}

	/*
	vector<Vector3f> box1Vertex(box1.data.size());
	for (uint32_t i = 0; i < box1.data.size(); ++i)
		box1Vertex[i] = box1.data[i].position;
	vector<unsigned> volume;
	vector<uint32_t> indices;
	uint32_t width = 2, height = 2, depth = 2;
	Voxelize(box1Vertex.data(), box1Vertex.size(), box1.indices.data(), box1.indices.size() / 3,
			 width, height, depth, volume, phyBox1.getAabb().first, phyBox1.getAabb().second);

	for (uint32_t i = 0, offset = 2; i < volume.size(); ++i)
	{
		if (volume[i] == 1)
		{
			uint32_t z = i / (width * height);
			uint32_t y = i % (width * height) / width;
			uint32_t x = i % (width * height) % width;
			pbd.mRigiBodies.emplace_back(1.0f);
			pbd.mRigiBodies.back().mBaryCenter = phyBox1.getAabb().first +
												 Vector3f(phyBox1.mLength.x() / width * x,
														  phyBox1.mLength.y() / height * y,
														  phyBox1.mLength.z() / depth * z);
			pbd.mRigiBodies.back().mVelocity = Vector3f(0.f, 0.f, 0.f);
			indices.emplace_back(offset++);
		}
	}
	pbd.mConstraints.emplace_back(std::make_shared<ShapeMatchingConstraint>(pbd, indices.size(), indices));
	*/
}
