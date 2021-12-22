#include "OCEAN.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

constexpr float LX = 100.0f;
constexpr float LZ = 100.0f;
constexpr float AMPLITUDE = 1000.f;
constexpr float WIND_SPEED = 1.0f;
constexpr complex<float> WIND_DIRECTION = { 1.f, 1.f };
constexpr float CHOPPY_FACTOR = 1.0f;


OCEAN::OCEAN(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
	preferPresentMode = vk::PresentModeKHR::eFifo;
	camera.target = Vector3f{ 0.f, 10.f, 0.f };
	camera.cameraMoveSpeed = 1.f;
}

OCEAN::~OCEAN()
{
	device->waitIdle();

}

void OCEAN::createAssetApp()
{
	SDL2_IMGUI_BASE::createAssetApp();
	

	//sky.init(this);
	//renderables.emplace_back(sky.mesh.get(), sky.material, &sky.transform);
	//renderables.back().cullFlag = false;

	createOceans();

	createBox();
	createBoat();

	//createCloak();
	//createHuman();

	//createDragon();

	//createSoldiers();
}
void OCEAN::updateApp()
{
	updateEngine();

	ocean.update(dtAll);
	updateBox();

	//updateAI();

	updatePhysics();

	updateGraphics();
}

void OCEAN::updatePhysics()
{
	for(auto &b : boats)
		b.updatePhysics();
}

void OCEAN::updateGraphics()
{
	auto modelMatrixPtr = modelMatrix.data();
	uint32_t offset = 0;
	for (auto& d : draws)
	{
		auto& v = renderables[d.first];
		for (size_t i = 0; i < v.mesh->data.size(); i++)
			vertices[offset + i] = v.mesh->data[i];
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

void OCEAN::createOceans()
{
	ocean.init(this);
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			oceans[i*10+j].mesh = ocean.mesh;
			oceans[i*10+j].material = ocean.material;
			oceans[i*10+j].setPosition(Vector3f{ -900.f + j*200.f, 0.f, -900.f + i * 200.f });
			renderables.emplace_back(oceans[i*10+j].mesh.get(), oceans[i*10+j].material, &oceans[i*10+j].transform);
		}
	}
}

void OCEAN::createBox()
{
	LOGI("´´½¨Ïä×Ó");

	string name = "box";
	Vector3f position = { 0.f, 100.f, -10.f };
	loadMeshs[name] = make_shared<Crane::Cube>(2);
	box.mesh = loadMeshs[name];

	materials[name] = materialBuilderPhong.build();
	box.material = &materials[name];
	box.setPosition(position);

	for (int i = 0; i < boxs.size(); i++)
	{
		boxs[i].mesh = box.mesh;
		boxs[i].material = &materials[name];

		float x = -1000.f + rand() / float(RAND_MAX)*2000.f;
		float z = -1000.f + rand() / float(RAND_MAX)*2000.f;
		Vector3f position = { x, 0.f, z };
		boxs[i].setPosition(position);
		renderables.emplace_back(boxs[i].mesh.get(), boxs[i].material, &boxs[i].transform);
	}
}

void OCEAN::createBoat()
{
	for (auto& b : boats)
	{
		b.init(this);
		float x = -1000.f + rand() / float(RAND_MAX) * 2000.f;
		float z = -1000.f + rand() / float(RAND_MAX) * 2000.f;
		Vector3f position = { x, 0.f, z };
		b.setPosition(position);
	}
}

