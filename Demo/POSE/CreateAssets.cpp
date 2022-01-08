#include "POSE.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

void POSE::createBones()
{
    string name = "bone";
	loadMeshs[name] = make_shared<Crane::Cube>(2);
    loadMeshs[name]->setVertices([](uint32_t i, Vertex&v){v.position*=0.1f;});
	materials[name] = materialBuilderPhong.build();

	for (auto &b : bones)
	{
	    b.mesh = loadMeshs[name];
	    b.material = &materials[name];
		renderables.emplace_back(b.mesh.get(), b.material, &b.transform);
	}

	Vector3f position = { 0.f, 4.5f, -20.f };
    bones[0].setPosition(position);

	float scale = 2.5f;
    // left arms
    bones[1].setPosition(bones[0].position + scale*Vector3f{-0.30f, -0.30f, 0.f});
    bones[2].setPosition(bones[1].position + scale*Vector3f{-0.15f, -0.15f, 0.f});
    bones[3].setPosition(bones[2].position + scale*Vector3f{-0.15f, -0.15f, 0.f});

    // right arms
    bones[4].setPosition(bones[0].position + scale*Vector3f{0.30f, -0.30f, 0.f});
    bones[5].setPosition(bones[4].position + scale*Vector3f{0.15f, -0.15f, 0.f});
    bones[6].setPosition(bones[5].position + scale*Vector3f{0.15f, -0.15f, 0.f});

    // left legs
    bones[7].setPosition(bones[0].position + scale*Vector3f{-0.15f, -0.80f, 0.f});
    bones[8].setPosition(bones[7].position + scale*Vector3f{0.f, -0.40f, 0.f});
    bones[9].setPosition(bones[8].position + scale*Vector3f{0.f, -0.40f, 0.f});

    // right legs
    bones[10].setPosition(bones[0].position + scale*Vector3f{0.15f, -0.80f, 0.f});
    bones[11].setPosition(bones[10].position + scale*Vector3f{0.f, -0.40f, 0.f});
    bones[12].setPosition(bones[11].position + scale*Vector3f{0.f, -0.40f, 0.f});
}

void POSE::createChessboard()
{
	LOGI("create chessboard");

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

void POSE::createHuman()
{
	LOGI("create human");

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

	renderables.emplace_back(human.mesh.get(), human.material, &human.transform);

	bindCoffs.resize(human.mesh->data.size());
	for(uint32_t j = 0; j < human.mesh->data.size(); ++j)
	{
		float minDist = numeric_limits<float>::max();
		for(uint32_t i = 0; i < bones.size(); ++i)
		{
			Vector3f diff = human.mesh->data[j].position - bones[i].position;
			float dist = (diff).norm();
			if(dist < minDist)
			{
				minDist = dist;
				bindCoffs[j].index = i;
				bindCoffs[j].diff = diff;
			}
		}
	}

	for(uint32_t i = 0; i < human.mesh->data.size(); ++i)
	{
		human.mesh->data[i].position = bones[bindCoffs[i].index].position + bindCoffs[i].diff;
	}

	// voxelize

	/*
	human.computeAABB();
	Vector3f lengthHuman = human.extentMax - human.extentMin;

	vector<Vector3f> vertices(human.mesh->data.size());
	for (uint32_t i = 0; i < human.mesh->data.size(); ++i)
		vertices[i] = human.mesh->data[i].position;
	vector<unsigned> volume;
	vector<uint32_t> indices;

	float w = 0.25f, h = 0.25f, d = 0.25f; // ����
	uint32_t width = ceil(lengthHuman.x()/w), height = ceil(lengthHuman.y()/h), depth = ceil(lengthHuman.z()/d); // ����

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
	*/
}