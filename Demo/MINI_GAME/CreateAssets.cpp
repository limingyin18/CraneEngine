#include "MINI_GAME.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;
using namespace CranePhysics;

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

	// physics
	auto cube0 = std::make_shared<CranePhysics::Cube>();
	cube0->invMass = 0.f;
	cube0->position = chessboard.position;
	cube0->width = 100.f;
	cube0->depth = 100.f;
	cube0->height = 0.1f;
	pbd.rigidbodies.push_back(cube0);
}

void MINI_GAME::createBall()
{
	LOGI("创建ball");

	string name = "ball";
	float radius = 1.0f;
	Vector3f modelBall = { 0.f, 10.f, -10.f };
	loadMeshs[name] = make_shared<Crane::Sphere>(11, 11, radius);
	ball.mesh = loadMeshs[name];

	materials[name] = materialBuilderPhong.build();
	ball.material = &materials[name];
	ball.setPosition(modelBall);

	renderables.emplace_back(ball.mesh.get(), ball.material, &ball.transform);

	// physics
	auto rb = std::make_shared<CranePhysics::Sphere>();
	rb->invMass = 1.f;
	rb->radius = radius;
	rb->position = ball.position;
	rb->positionPrime = ball.position;

	offsetBall = pbd.rigidbodies.size();
	pbd.rigidbodies.push_back(rb);
}

void MINI_GAME::createCloak()
{
	LOGI("创建cloak");

	Eigen::Vector3f modelCloak{ 0.f, 4.f, -1.17f };
	Eigen::Vector3f rotationCloak{ 0.f, numbers::pi / 2.f, 0.f };

	string name = "cloak";
	loadMeshs[name] = make_shared<Crane::Plane>(11, 11);
	cloak.mesh = loadMeshs[name];
	cloak.mesh->setVertices([](uint32_t, Vertex& v) {v.position.z() *= 0.5f; });

	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoLilac;
	materials[name] = materialBuilderPhong.build();
	cloak.material = &materials[name];

	cloak.setPosition(modelCloak);
	cloak.setRotation(rotationCloak);
	renderables.emplace_back(cloak.mesh.get(), cloak.material, &cloak.transform);

	//physics
	offsetCloak = pbd.rigidbodies.size();
	for (uint32_t i = 0; i < cloak.mesh->data.size(); ++i)
	{
		auto homogeneous = cloak.transform * Eigen::Vector4f{ cloak.mesh->data[i].position[0],
											cloak.mesh->data[i].position[1],
											cloak.mesh->data[i].position[2],
											1.f };
		auto particle = std::make_shared<CranePhysics::Particle>();
		particle->radius = 0.0005f;
		particle->position = Vector3f{ homogeneous[0], homogeneous[1], homogeneous[2] };
		particle->positionPrime = particle->position;

		//particle->invMass = (i - i % 11) / 11.f;
		if (i == 0 || i == 10)
			particle->invMass = 0.f;
		else
			particle->invMass = 1.f;
		pbd.rigidbodies.push_back(particle);
	}

	//
	//for (uint32_t i = 0; i < 11; ++i)
	//{
	//	for (uint32_t j = 1; j < 11; ++j)
	//	{
	//		float d = (pbd.rigidbodies[offsetCloak+i]->position - pbd.rigidbodies[offsetCloak+j * 11 + i]->position).norm();
	//		pbd.constraints.emplace_back(std::make_shared<LongRangeAttachmentConstraint>(*pbd.rigidbodies[offsetCloak+i+(j-1)*11], *pbd.rigidbodies[offsetCloak+j*11+i], d, 1.0f));
	//	}
	//}

	float compress = 1.0f, stretch = 1.0f;
	for (uint32_t i = 0; i < cloak.mesh->indices.size(); i = i + 3)
	{
		size_t indexA = cloak.mesh->indices[i];
		size_t indexB = cloak.mesh->indices[i + 1];
		size_t indexC = cloak.mesh->indices[i + 2];

		CranePhysics::Rigidbody& a = *(pbd.rigidbodies[offsetCloak + indexA]);
		CranePhysics::Rigidbody& b = *(pbd.rigidbodies[offsetCloak + indexB]);
		CranePhysics::Rigidbody& c = *(pbd.rigidbodies[offsetCloak + indexC]);
		float distAB = (a.position - b.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(a, b, distAB, compress, stretch));

		float distAC = (a.position - c.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(a, c, distAC, compress, stretch));

		float distBC = (b.position - c.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(b, c, distBC, compress, stretch));
	}


	CranePhysics::Rigidbody& a = *(pbd.rigidbodies[offsetCloak + 0]);
	a.position.z() = -0.6f;
	//a.position.x() = -0.f;
	a.positionPrime = a.position;

	CranePhysics::Rigidbody& b = *(pbd.rigidbodies[offsetCloak + 10]);
	b.position.z() = -0.6f;
	b.positionPrime = b.position;

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

void MINI_GAME::createHuman()
{
	LOGI("创建人模型");

	string name = "human";

	assets::AssetFile file;
	if (!assets::load_binaryfile((string("assets/") + "human.mesh").c_str(), file))
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
	human.mesh = loadMeshs[name];


	if (materials.find(name) == materials.end())
	{
		materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoBlank;
		materials[name] =  materialBuilderPhong.build();
	}
	human.material = &materials[name];

	Eigen::Vector3f rotationHuman{0.f, 0.f, 0.f };
	Eigen::Vector3f modelHuman{ 0.f, 0.f, 0.f };
	human.setRotation(rotationHuman);
	human.setPosition(modelHuman);

	//renderables.emplace_back(human.mesh.get(), human.material, &human.transform);


	// voxelize

	human.computeAABB();
	Vector3f lengthHuman = human.extentMax - human.extentMin;

	vector<Vector3f> vertices(human.mesh->data.size());
	for (uint32_t i = 0; i < human.mesh->data.size(); ++i)
		vertices[i] = human.mesh->data[i].position;
	vector<unsigned> volume;
	vector<uint32_t> indices;

	float w = 0.25f, h = 0.25f, d = 0.25f; // 精度
	uint32_t width = ceil(lengthHuman.x()/w), height = ceil(lengthHuman.y()/h), depth = ceil(lengthHuman.z()/d); // 个数

	Voxelize(vertices.data(), vertices.size(), human.mesh->indices.data(), human.mesh->indices.size() / 3,
			 width, height, depth, volume, human.extentMin, human.extentMax);

	IDHuman = pbd.rigidbodies.size();
	Actor cube;
	name = "cubeVox";
	loadMeshs[name] = make_shared<Crane::Cube>(2);
	cube.mesh = loadMeshs[name];
	cube.mesh->setVertices([&lengthHuman, &width, &height, &depth](uint32_t i, Vertex &v) 
	{
		v.position.x() *= lengthHuman.x() / width/2.f;
		v.position.y() *= lengthHuman.y() / height/2.f;
		v.position.z() *= lengthHuman.z() / depth/2.f;
	}
	);
	materialBuilderPhong.pipelinePass = &pipelinePassLinePhong;
	materials[name] = materialBuilderPhong.build();
	materialBuilderPhong.pipelinePass = &pipelinePassPhong;
	cube.material = &materials[name];

	for (uint32_t i = 0; i < volume.size(); ++i)
	{
		if (volume[i] == 1)
		{
			uint32_t z = i / (width * height);
			uint32_t y = i % (width * height) / width;
			uint32_t x = i % (width * height) % width;


			auto cubePhy = std::make_shared<CranePhysics::Cube>(0.f);
			cubePhy->width = lengthHuman.x() / width;
			cubePhy->height = lengthHuman.y() / height;
			cubePhy->depth = lengthHuman.z() / depth;
			cubePhy->position = human.extentMin +
												 Vector3f(lengthHuman.x() / width * x,
														  lengthHuman.y() / height * y,
														  lengthHuman.z() / depth * z);

			cubePhy->positionPrime = pbd.rigidbodies.back()->position;

			pbd.rigidbodies.emplace_back(cubePhy);

			cube.setPosition(cubePhy->position);
			cubesHuman.push_back(cube);
		}
	}

	for(auto &c : cubesHuman)
		renderables.emplace_back(c.mesh.get(), c.material, &c.transform);
}

