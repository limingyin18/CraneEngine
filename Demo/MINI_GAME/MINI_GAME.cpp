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
	camera.cameraMoveSpeed = 1.1f;
}

MINI_GAME::~MINI_GAME()
{
	vkDeviceWaitIdle(device.get());
}

void MINI_GAME::updateApp()
{
	updateEngine();

	ocean.update(dtAll);

	updateAI();

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
		*(reinterpret_cast<Eigen::Matrix4f*>(modelMatrixPtr)) = *v.transformMatrix;
		modelMatrixPtr += modelMatrixOffset;
	}
	modelMatrixBuffer.update(modelMatrix.data());

	for (auto& d : draws)
	{
		for (uint32_t i = d.first; i < d.count; ++i)
		{
			cullObjCandidates[i].model = *renderables[i].transformMatrix;
		}
	}
	bufferCullObjCandidate.update(cullObjCandidates.data());
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
	LOGI("create application specific assets");

	SDL2_IMGUI_BASE::createAssetApp();

	//createChessboard();
	sky.init(this);
	renderables.emplace_back(sky.mesh.get(), sky.material, &sky.transform);
	renderables.back().cullFlag = false;

	createCloak();

	createDragon();

	createSoldiers();

	ocean.init(this);

	oceans[0].mesh = ocean.mesh;
	oceans[0].material = ocean.material;
	oceans[0].setPosition(Vector3f{ 100.f, 0.f, 0.f });
	renderables.emplace_back(oceans[0].mesh.get(), oceans[0].material, &oceans[0].transform);

	oceans[1].mesh = ocean.mesh;
	oceans[1].material = ocean.material;
	oceans[1].setPosition(Vector3f{ -100.f, 0.f, 0.f });
	renderables.emplace_back(oceans[1].mesh.get(), oceans[1].material, &oceans[1].transform);

	oceans[2].mesh = ocean.mesh;
	oceans[2].material = ocean.material;
	oceans[2].setPosition(Vector3f{ 100.f, 0.f, 200.f });
	renderables.emplace_back(oceans[2].mesh.get(), oceans[2].material, &oceans[2].transform);

	oceans[3].mesh = ocean.mesh;
	oceans[3].material = ocean.material;
	oceans[3].setPosition(Vector3f{ -100.f, 0.f, 200.f });
	renderables.emplace_back(oceans[3].mesh.get(), oceans[3].material, &oceans[3].transform);
	// physics
	/*
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

	for (uint32_t i = 0; i < 11; ++i)
	{
		for (uint32_t j = 1; j < 11; ++j)
		{
			float d = (pbd.rigidbodies[2+i]->position - pbd.rigidbodies[2+j * 11 + i]->position).norm();
			pbd.constraints.emplace_back(std::make_shared<LongRangeAttachmentConstraint>(*pbd.rigidbodies[2+i+(j-1)*11], *pbd.rigidbodies[2+j*11+i], d, 1.0f));
		}
	}

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

	/*
	// Render a sequence of images (sunrise to sunset)
	unsigned nangles = 128;
	for (unsigned i = 0; i < nangles; ++i) {
		char filename[1024];
		sprintf(filename, "./skydome.%04d.ppm", i);
		float angle = i / float(nangles - 1) * numbers::pi * 0.6;
		fprintf(stderr, "Rendering image %d, angle = %0.2f\n", i, angle * 180 / numbers::pi);
		renderSkydome(Vector3f(0, cos(angle), -sin(angle)), filename);
	}*/
}
