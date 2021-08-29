#include "CLOTH.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

CLOTH::CLOTH(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win) 
{
	//preferPresentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR ;
	camera.target = Vector3f{0.f, 2.5f, 0.f};
	camera.rotation[0] = -0.2f;
}

CLOTH::~CLOTH()
{
	vkDeviceWaitIdle(device.get());
}

void CLOTH::updateApp()
{
	updateEngine();

	dt = 0.01f;
	// physics update
	pbd.dt = dt;
	//pbd.rigidbodies[1]->rotation *= Quaternionf(AngleAxisf(1.f*dt, Vector3f(1.0f, 1.0f, 1.0f).normalized()));
	Vector3f u{2.f, 0.f, 0.f};
	float drag = 1.f;
	float lift = 1.f;

	for(size_t i = 0; i < cloak.indices.size(); i = i + 3)
	{
		size_t indexA = cloak.indices[i];
		size_t indexB = cloak.indices[i+1];
		size_t indexC = cloak.indices[i+2];
		CranePhysics::Particle &a = *dynamic_cast<Particle*>(pbd.rigidbodies[2+indexA].get());
		CranePhysics::Particle &b = *dynamic_cast<Particle*>(pbd.rigidbodies[2+indexB].get());
		CranePhysics::Particle &c = *dynamic_cast<Particle*>(pbd.rigidbodies[2+indexC].get());

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

	for(size_t i = 0; i < renderables.size(); ++i)
	{
		Matrix4f t = (Translation3f(pbd.rigidbodies[i]->position) * pbd.rigidbodies[i]->rotation).matrix();
		renderables[i].transformMatrix = t;
	}

	auto modelMatrixPtr = modelMatrix.data();
	for (auto &v : renderables)
	{
		*(reinterpret_cast<Eigen::Matrix4f *>(modelMatrixPtr)) = v.transformMatrix;
		modelMatrixPtr += modelMatrixOffset;
	}
	modelMatrixBuffer.update(modelMatrix.data());

	renderables[1].mesh->setVertices([this](uint32_t i, Crane::Vertex &v){v.position = this->pbd.rigidbodies[2+i]->position;});
	renderables[1].mesh->recomputeNormals(renderables[1].mesh->data);
	size_t offset = 0;
	for (auto &v : renderables)
    {
        for (size_t i = 0; i < v.mesh->data.size(); i++)
            vertices[offset++] = v.mesh->data[i];
    }
	vertBuff.update(vertices.data());
}

void CLOTH::setImgui()
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

void CLOTH::createAssetApp()
{
	LOGI("创建应用资产");

	SDL2_IMGUI_BASE::createAssetApp();

	// create texture
	assets::AssetFile textureYume;
	if (!load_binaryfile("assets/anime.tx", textureYume))
		throw runtime_error("load asset failed");

	assets::TextureInfo textureInfo = assets::read_texture_info(&textureYume);
	VkDeviceSize imageSize = textureInfo.textureSize;
	vector<uint8_t> pixels(imageSize);
	assets::unpack_texture(&textureInfo, textureYume.binaryBlob.data(), textureYume.binaryBlob.size(), (char *)pixels.data());

	int texWidth, texHeight, texChannels;
	texChannels = 4;
	texWidth = textureInfo.pages[0].width;
	texHeight = textureInfo.pages[0].height;

	tie(textureImage, textureImageView) = createTextureImage(texWidth, texHeight, texChannels, pixels.data());


	// floor [-50, 50]x[-50, 50]x[0]
	floorEnv.setVertices([](uint32_t i, Crane::Vertex &v)
					   { v.position *= 50; v.color = Vector3f{1.0f, 1.0, 1.0f};});
	floorEnv.recomputeNormals(floorEnv.data);

	// cloak
	cloak.recomputeNormals(cloak.data);

	for(uint32_t i = 0; i <	cloak.data.size(); ++i)
	{
		Crane::Cube particle(2);
		particle.setVertices([](uint32_t i, Crane::Vertex &v)
					   { v.position *= 0.05f;});
		particle.recomputeNormals(particle.data);
		particles.push_back(particle);
	}

	// add
	renderables.emplace_back(&floorEnv, imageViewLilac.get());
	renderables.emplace_back(&cloak, textureImageView.get());
	renderables[1].transformMatrix.block<3, 1>(0, 3) = model1;

	for(uint32_t i = 0; i <	cloak.data.size(); ++i)
	{
		renderables.emplace_back(&particles[i], imageViewBlank.get());
		Vector3f modeln = cloak.data[i].position+model1;
		renderables.back().transformMatrix.block<3, 1>(0, 3) = modeln;
	}

	// shader
	auto vertShaderCode = Loader::readFile("shaders/phong.vert.spv");
	auto fragShaderCode = Loader::readFile("shaders/phong.frag.spv");
	vertexShader = createShaderModule(vertShaderCode);
	fragmentShader = createShaderModule(fragShaderCode);
	vk::PipelineShaderStageCreateInfo vertShaderCreateInfo{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = vertexShader.get(),
		.pName = "main"};
	vk::PipelineShaderStageCreateInfo fragShaderCreateInfo{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = fragmentShader.get(),
		.pName = "main"};
	pipelineShaderStageCreateInfos.push_back(vertShaderCreateInfo);
	pipelineShaderStageCreateInfos.push_back(fragShaderCreateInfo);


	// physics
	auto cube0 = std::make_shared<CranePhysics::Cube>();
	cube0->invMass = 0.f;
	cube0->position = model0;
	cube0->width = 100.f;
	cube0->depth = 100.f;
	cube0->height = 0.1f;
	pbd.rigidbodies.push_back(cube0);

	auto particle = std::make_shared<CranePhysics::Particle>();
	particle->position = model1;
	particle->invMass = 0;
	particle->radius = 0;
	pbd.rigidbodies.push_back(particle);

	for(uint32_t i = 0; i < cloak.data.size(); ++i)
	{
		auto particle = std::make_shared<CranePhysics::Particle>();
		particle->radius = 0.0005f;
		particle->position = cloak.data[i].position+model1;
		particle->positionPrime = particle->position;
		
		particle->invMass = (i - i%11)/11.f;
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
	for(uint32_t i = 0; i < cloak.indices.size(); i = i + 3)
	{
		size_t indexA = cloak.indices[i];
		size_t indexB = cloak.indices[i+1];
		size_t indexC = cloak.indices[i+2];

		CranePhysics::Rigidbody &a = *(pbd.rigidbodies[2+indexA]);
		CranePhysics::Rigidbody &b = *(pbd.rigidbodies[2+indexB]);
		CranePhysics::Rigidbody &c = *(pbd.rigidbodies[2+indexC]);
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
