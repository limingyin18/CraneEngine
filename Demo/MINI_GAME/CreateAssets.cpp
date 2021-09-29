#include "MINI_GAME.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

void MINI_GAME::createChessboard()
{
	LOGI("创建chessboard");

	string name = "chessboard";
	loadMeshs[name] = make_shared<Crane::Chessboard>(11, 11);
	loadMeshs[name]->setVertices([](uint32_t, Vertex& v) {v.position *= 100; });
	chessboard.mesh = loadMeshs[name];

	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoBlank;
	materials[name] = materialBuilderPhong.build();
	chessboard.material = &materials[name];

	renderables.emplace_back(chessboard.mesh.get(), chessboard.material, &chessboard.transform);
}

void MINI_GAME::createCloak()
{
	LOGI("创建cloak");

	Eigen::Vector3f modelCloak{ 0.f, 5.f, -5.f };

	string name = "cloak";
	loadMeshs[name] = make_shared<Crane::Plane>(11, 11);
	cloak.mesh = loadMeshs[name];

	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoLilac;
	materials[name] = materialBuilderPhong.build();
	cloak.material = &materials[name];

	cloak.setPosition(modelCloak);
	renderables.emplace_back(cloak.mesh.get(), cloak.material, &cloak.transform);
}

void MINI_GAME::createDragon()
{
	LOGI("读取龙模型");
	Eigen::Vector3f rotationDragon{45.f/180.f*3.14f, 0.f, 0.f };
	Eigen::Vector3f modelDragon{ 0.f, 5.f, -5.f };

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
		loadMeshs[name].get()->setVertices([](uint32_t i, Vertex& v) {v.color = { 1.f, 1.f, 1.f }; });
		loadMeshs[name].get()->recomputeNormals();
	}
	dragon.mesh = loadMeshs[name];


	if (materials.find(name) == materials.end())
	{
		materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoBlank;
		materials[name] =  materialBuilderPhong.build();
	}
	dragon.material = &materials[name];

	dragon.setRotation(rotationDragon);
	dragon.setPosition(modelDragon);

	renderables.emplace_back(dragon.mesh.get(), dragon.material, &dragon.transform);
}

void MINI_GAME::createSoldiers()
{
	LOGI("读取罗马士兵模型");

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
		loadMeshs[name].get()->setVertices([](uint32_t i, Vertex& v) {v.color = { 1.f, 1.f, 1.f }; });
		loadMeshs[name].get()->recomputeNormals();
	}

	if (materials.find(name) == materials.end())
	{
		materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoBlank;
		materials[name] = materialBuilderPhong.build();
	}

	soldiers.resize(soldierCount*soldierCount);
	for (uint32_t i = 0; i < soldierCount; ++i)
	{
		for (uint32_t j = 0; j < soldierCount; ++j)
		{
			soldiers[i*soldierCount+j].mesh = loadMeshs[name];
			soldiers[i*soldierCount+j].material = &materials[name];

			float x = 100.f * (2.f * normal_dist(rand_generator) - 1.f);
			float y = 0.f;
			float z = 100.f * (2.f * normal_dist(rand_generator) - 1.f);
			soldiers[i*soldierCount+j].setPosition(Vector3f{ x, y, z });

			renderables.emplace_back(soldiers[i*soldierCount+j].mesh.get(), soldiers[i*soldierCount+j].material, &soldiers[i*soldierCount+j].transform);
		}
	}
}