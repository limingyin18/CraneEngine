#include "MINI_GAME.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

MINI_GAME::MINI_GAME(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
	//preferPresentMode = vk::PresentModeKHR::eFifo;
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

	//ocean.update(dtAll);

	//updateAI();

	updatePhysics();

	updateGraphics();

}

void MINI_GAME::updateGraphics()
{
	auto modelMatrixPtr = modelMatrix.data();
	uint32_t offset = 0;
	for (auto& d : draws)
	{
		auto& v = renderables[d.first];
		for (size_t i = 0; i < v.mesh->data.size(); i++)
			vertices[offset+i] = v.mesh->data[i];
		offset += v.mesh->data.size();

		for (uint32_t i = 0; i < d.count; ++i)
		{
			*(reinterpret_cast<Eigen::Matrix4f*>(modelMatrixPtr)) = *renderables[d.first + i].transformMatrix;
			modelMatrixPtr = modelMatrixPtr + modelMatrixOffset;
		}
	}
	modelMatrixBuffer.update(modelMatrix.data());
	vertBuff.update(vertices.data());
	

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

	createChessboard();

	sky.init(this);
	renderables.emplace_back(sky.mesh.get(), sky.material, &sky.transform);
	renderables.back().cullFlag = false;

	//createBall();

	createCloak();
	createHuman();

	//createDragon();

	//createSoldiers();

	//ocean.init(this);

	//oceans[0].mesh = ocean.mesh;
	//oceans[0].material = ocean.material;
	//oceans[0].setPosition(Vector3f{ 100.f, 0.f, 0.f });
	//renderables.emplace_back(oceans[0].mesh.get(), oceans[0].material, &oceans[0].transform);

	//oceans[1].mesh = ocean.mesh;
	//oceans[1].material = ocean.material;
	//oceans[1].setPosition(Vector3f{ -100.f, 0.f, 0.f });
	//renderables.emplace_back(oceans[1].mesh.get(), oceans[1].material, &oceans[1].transform);

	//oceans[2].mesh = ocean.mesh;
	//oceans[2].material = ocean.material;
	//oceans[2].setPosition(Vector3f{ 100.f, 0.f, 200.f });
	//renderables.emplace_back(oceans[2].mesh.get(), oceans[2].material, &oceans[2].transform);

	//oceans[3].mesh = ocean.mesh;
	//oceans[3].material = ocean.material;
	//oceans[3].setPosition(Vector3f{ -100.f, 0.f, 200.f });
	//renderables.emplace_back(oceans[3].mesh.get(), oceans[3].material, &oceans[3].transform);

	// physics


	//vector<Vector3f> box1Vertex(box1.data.size());
	//for (uint32_t i = 0; i < box1.data.size(); ++i)
	//	box1Vertex[i] = box1.data[i].position;
	//vector<unsigned> volume;
	//vector<uint32_t> indices;
	//uint32_t width = 2, height = 2, depth = 2;
	//Voxelize(box1Vertex.data(), box1Vertex.size(), box1.indices.data(), box1.indices.size() / 3,
	//		 width, height, depth, volume, phyBox1.getAabb().first, phyBox1.getAabb().second);

	//for (uint32_t i = 0, offset = 2; i < volume.size(); ++i)
	//{
	//	if (volume[i] == 1)
	//	{
	//		uint32_t z = i / (width * height);
	//		uint32_t y = i % (width * height) / width;
	//		uint32_t x = i % (width * height) % width;
	//		pbd.mRigiBodies.emplace_back(1.0f);
	//		pbd.mRigiBodies.back().mBaryCenter = phyBox1.getAabb().first +
	//											 Vector3f(phyBox1.mLength.x() / width * x,
	//													  phyBox1.mLength.y() / height * y,
	//													  phyBox1.mLength.z() / depth * z);
	//		pbd.mRigiBodies.back().mVelocity = Vector3f(0.f, 0.f, 0.f);
	//		indices.emplace_back(offset++);
	//	}
	//}
	//pbd.mConstraints.emplace_back(std::make_shared<ShapeMatchingConstraint>(pbd, indices.size(), indices));

	for (auto& rb : pbd.rigidbodies)
		rb->computeAABB();

	pbd.bvh = BVH(pbd.rigidbodies);
}
