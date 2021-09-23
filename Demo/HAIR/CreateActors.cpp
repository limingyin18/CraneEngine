#include "HAIR.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;
using namespace CranePhysics;

void HAIR::createChessboard()
{
	LOGI("创建chessboard");

	string name = "chessboard";
	loadMeshs[name] = make_shared<Crane::Chessboard>(11, 11);
	loadMeshs[name]->setVertices([](uint32_t, Vertex& v) {v.position *= 10; });
	chessboard.mesh = loadMeshs[name];

	materials[name] = materialBuilderPhong.build();
	chessboard.material = &materials[name];

	renderables.emplace_back(chessboard.mesh.get(), chessboard.material, &chessboard.transform);

	// physics
	auto physicsCubeChessboard = std::make_shared<CranePhysics::Cube>();
	physicsCubeChessboard->invMass = 0.f;
	physicsCubeChessboard->position = chessboard.position;
	physicsCubeChessboard->width = 100.f;
	physicsCubeChessboard->depth = 100.f;
	physicsCubeChessboard->height = 0.1f;
	pbd.rigidbodies.push_back(physicsCubeChessboard);
}

void HAIR::createHair()
{
	LOGI("构建头发模型");


	verticesHair.reserve(3);
	verticesHair.emplace_back(0.f, 5.f, 0.f);
	verticesHair.emplace_back(-0.1f, 4.8f, 0.f);
	verticesHair.emplace_back(-0.15f, 4.5f, 0.f);
	verticesHair.emplace_back(-0.4f, 4.0f, 0.f);

	// tangent
	tangentsHair.reserve(verticesHair.size());
	Vector3f tangentFirst = verticesHair[0] - verticesHair[1];
	tangentFirst.normalize();
	tangentsHair.emplace_back(tangentFirst);
	for (uint32_t i = 1; i < verticesHair.size()-1; ++i)
	{
		Vector3f tangent = (verticesHair[i-1] - verticesHair[i + 1])/2.f;
		tangent.normalize();
		tangentsHair.emplace_back(tangent);
	}
	Vector3f tangentLast = verticesHair[verticesHair.size()-2] - verticesHair[verticesHair.size()-1];
	tangentLast.normalize();
	tangentsHair.emplace_back(tangentLast);

	// normal
	normalsHair.reserve(verticesHair.size());
	Vector3f normalFirst = tangentsHair[0] - tangentsHair[1];
	normalFirst.normalize();
	normalsHair.emplace_back(normalFirst);
	for (uint32_t i = 1; i < verticesHair.size()-1; ++i)
	{
		Vector3f normal = (tangentsHair[i-1] - tangentsHair[i + 1])/ 2.f;
		normal.normalize();
		normalsHair.emplace_back(normal);
	}
	Vector3f normalLast = tangentsHair[verticesHair.size()-2] - tangentsHair[verticesHair.size()-1];
	normalLast.normalize();
	normalsHair.emplace_back(normalLast);

	string name = "hair";
	if (loadMeshs.find(name) == loadMeshs.end())
	{
		loadMeshs[name] = make_shared<MeshBase>();
		loadMeshs[name]->data.reserve(verticesHair.size()*2);
		loadMeshs[name]->indices.reserve((verticesHair.size()-1)*6);

		for (uint32_t i = 0; i < verticesHair.size(); ++i)
		{
			loadMeshs[name]->data.emplace_back(verticesHair[i] - radius * normalsHair[i]);
			loadMeshs[name]->data.emplace_back(verticesHair[i] + radius * normalsHair[i]);
		}

		for (uint32_t i = 0; i < verticesHair.size() - 1; ++i)
		{
			uint32_t offset = i * 2;
			loadMeshs[name]->indices.emplace_back(offset+0); 
			loadMeshs[name]->indices.emplace_back(offset+1); 
			loadMeshs[name]->indices.emplace_back(offset+2); 

			loadMeshs[name]->indices.emplace_back(offset+2); 
			loadMeshs[name]->indices.emplace_back(offset+1); 
			loadMeshs[name]->indices.emplace_back(offset+3); 
		}

		loadMeshs[name].get()->recomputeNormals();
	}

	if (materials.find(name) == materials.end())
	{
		materials[name] = materialBuilderPhong.build();
	}

	hair.mesh = loadMeshs[name];
	hair.material = &materials[name];

	for (const auto& obj : renderables)
		hairRenderableOffset += obj.mesh->data.size();
	renderables.emplace_back(hair.mesh.get(), hair.material, &hair.transform);

	hairPhysicsIndexOffset = pbd.rigidbodies.size();
	for (uint32_t i = 0; i < verticesHair.size(); ++i)
	{
		auto particleHair = std::make_shared<CranePhysics::Particle>();

		particleHair->invMass = 1.f;
		particleHair->position = verticesHair[i];
		particleHair->positionPrime = verticesHair[i];
		particleHair->radius = radius;
		pbd.rigidbodies.push_back(particleHair);
	}
	pbd.rigidbodies[hairPhysicsIndexOffset]->invMass = 0.f;

	float stretch = 1.0f;
	for (uint32_t i = 0; i < verticesHair.size() - 1; ++i)
	{
		CranePhysics::Rigidbody& at = *(pbd.rigidbodies[hairPhysicsIndexOffset + i]);
		CranePhysics::Rigidbody& rb = *(pbd.rigidbodies[hairPhysicsIndexOffset + i + 1]);
		float restDistance = (at.position - rb.position).norm();
		pbd.constraints.emplace_back(std::make_shared<LongRangeAttachmentConstraint>(at, rb, restDistance, stretch));
	}
}
