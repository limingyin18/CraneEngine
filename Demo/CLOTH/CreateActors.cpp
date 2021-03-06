#include "CLOTH.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;
using namespace CranePhysics;

void CLOTH::createChessboard()
{
	LOGI("create chessboard");

	chessboard = make_shared<Actor>();

    chessboardGP = make_shared<GraphicsPrimitive>();
	{
		string name = "chessboard";
        if (meshRepository.find(name) == meshRepository.end())
			meshRepository[name] = make_shared<Crane::Chessboard>(11, 11);
        shared_ptr<Crane::Chessboard> mesh = dynamic_pointer_cast<Crane::Chessboard>(meshRepository[name]);
		mesh->setVertices([](uint32_t, Vertex& v) {v.position *= 100; });
		chessboardGP->mesh = mesh;
	}
	{
		string name = "phongBlank";
        if (materialRepository.find(name) == materialRepository.end())
            materialRepository[name] = materialBuilderPhong.build();
        chessboardGP->material = materialRepository[name];
	}
	{
        Eigen::Vector3f pos{0.f, 0.f, 0.f};
        chessboardGP->transformer.setPosition(pos);
	}
	chessboardGP->transformer.setTransformParent(&chessboard->transformer.getTransformWorld());
	chessboard->primitives.push_back(chessboardGP);
	chessboard->transformer.setTransformParent(&transformWorld);
	scene.push_back(chessboard);

	// physics
	auto physicsCubeChessboard = std::make_shared<CranePhysics::Cube>();
	physicsCubeChessboard->invMass = 0.f;
	Vector3f pos = chessboard->primitives[0]->transformer.getPosition();
	Matrix4f trans = *(chessboard->primitives[0]->transformer.getTransformParent());
	Vector4f posT =  trans* Vector4f{pos[0], pos[1], pos[2], 1.0f};

	physicsCubeChessboard->position = Vector3f{posT[0], posT[1], posT[2]};
	physicsCubeChessboard->width = 100.f;
	physicsCubeChessboard->depth = 100.f;
	physicsCubeChessboard->height = 0.1f;
	pbd.rigidbodies.push_back(physicsCubeChessboard);
}

/*
void CLOTH::createCloak()
{
	LOGI("create cloak");

	string nameImage = "BannerHolyRoman";
	{
		string imageName = "Banner_of_the_Holy_Roman_Emperor_with_haloes.tx";
		assets::AssetFile textureFile;
		if (!load_binaryfile((string("assets/") + imageName).c_str(), textureFile))
			throw runtime_error("load asset failed");

		assets::TextureInfo textureInfo = assets::read_texture_info(&textureFile);
		VkDeviceSize imageSize = textureInfo.textureSize;
		vector<uint8_t> pixels(imageSize);
		assets::unpack_texture(&textureInfo, textureFile.binaryBlob.data(), textureFile.binaryBlob.size(), (char*)pixels.data());

		int texWidth, texHeight, texChannels;
		texChannels = 4;
		texWidth = textureInfo.pages[0].width;
		texHeight = textureInfo.pages[0].height;

		tie(loadImages[nameImage], loadImageViews[nameImage]) = createTextureImage(texWidth, texHeight, texChannels, pixels.data());
		descriptorImageInfos[nameImage].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		descriptorImageInfos[nameImage].imageView = loadImageViews[nameImage].get();
		descriptorImageInfos[nameImage].sampler = textureSampler.get();
	}

	Eigen::Vector3f modelCloak{ 0.f, 6.0f, 5.f };

	string name = "cloak";
	uint32_t n = 3;
	loadMeshs[name] = make_shared<Crane::Plane>(n, n);
	cloak.mesh = loadMeshs[name];

	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfos[nameImage];
	materials[name] = materialBuilderPhong.build();
	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoBlank;
	cloak.material = &materials[name];

	cloak.setPosition(modelCloak);
	renderables.emplace_back(cloak.mesh.get(), cloak.material, &cloak.transform);

	// physics
	offsetCloak = pbd.rigidbodies.size();
	for (uint32_t i = 0; i < cloak.mesh->data.size(); ++i)
	{
		auto particle = std::make_shared<CranePhysics::Particle>();
		particle->radius = 1.0f / 10.f - 0.01f;
		particle->position = cloak.mesh->data[i].position + modelCloak;
		particle->positionPrime = particle->position;

		particle->invMass = (i - i % n) / float(n);
		pbd.rigidbodies.push_back(particle);
	}

	// physics constraint
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
}

void CLOTH::createCubeTest()
{
	Eigen::Vector3f modelCube{ 0.f, 3.0f, -5.f };

	string name = "cubeTest";
	loadMeshs[name] = make_shared<Crane::Cube>(2);
	cubeTest.mesh = loadMeshs[name];

	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoLilac;
	materialBuilderPhong.pipelinePass = &pipelinePassLinePhong;
	materials[name] = materialBuilderPhong.build();
	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoBlank;
	materialBuilderPhong.pipelinePass = &pipelinePassPhong;
	cubeTest.material = &materials[name];

	cubeTest.setPosition(modelCube);
	renderables.emplace_back(cubeTest.mesh.get(), cubeTest.material, &cubeTest.transform);

	// physics
	auto physicsCubeTest = std::make_shared<CranePhysics::Cube>();
	physicsCubeTest->invMass = 0.f;
	physicsCubeTest->position = cubeTest.position;
	physicsCubeTest->positionPrime = cubeTest.position;
	physicsCubeTest->width = 2.f;
	physicsCubeTest->depth = 2.f;
	physicsCubeTest->height =2.f;
	pbd.rigidbodies.push_back(physicsCubeTest);
}

void CLOTH::createFlagCloth()
{
	LOGI("create flag cloth");

	Eigen::Vector3f modelFlagCloth{ 0.f, 3.0f, 5.f };

	string name = "flagCloth";
	loadMeshs[name] = make_shared<Crane::Plane>(3, 3);
	flagCloth.mesh = loadMeshs[name];
	flagCloth.material = &materials["cloak"];
	flagCloth.setPosition(modelFlagCloth);
	renderables.emplace_back(flagCloth.mesh.get(), flagCloth.material, &flagCloth.transform);

	// physics
	offsetPhyFlagCloth = pbd.rigidbodies.size();
	for (uint32_t i = 0; i < flagCloth.mesh->data.size(); ++i)
	{
		auto particle = std::make_shared<CranePhysics::Particle>();
		particle->radius = 1.0f / 10.f - 0.01f;
		particle->position = flagCloth.mesh->data[i].position + modelFlagCloth;
		particle->positionPrime = particle->position;

		//particle->invMass = (i - i % 11) / 11.f;
		particle->invMass = 1.f;
		pbd.rigidbodies.push_back(particle);
	}

	// physics constraint
	float compress = 1.0f, stretch = 1.0f;
	for (uint32_t i = 0; i < flagCloth.mesh->indices.size(); i = i + 3)
	{
		size_t indexA = flagCloth.mesh->indices[i];
		size_t indexB = flagCloth.mesh->indices[i + 1];
		size_t indexC = flagCloth.mesh->indices[i + 2];

		CranePhysics::Rigidbody& a = *(pbd.rigidbodies[offsetPhyFlagCloth + indexA]);
		CranePhysics::Rigidbody& b = *(pbd.rigidbodies[offsetPhyFlagCloth + indexB]);
		CranePhysics::Rigidbody& c = *(pbd.rigidbodies[offsetPhyFlagCloth + indexC]);
		float distAB = (a.position - b.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(a, b, distAB, compress, stretch));

		float distAC = (a.position - c.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(a, c, distAC, compress, stretch));

		float distBC = (b.position - c.position).norm();
		pbd.constraints.emplace_back(std::make_shared<Stretching>(b, c, distBC, compress, stretch));
	}
}

void CLOTH::createSphereTest()
{
	float radius = 1.f;
	Eigen::Vector3f modelSphere{ 0.f, 10.0f, -10.f };

	string name = "sphereTest";
	loadMeshs[name] = make_shared<Crane::Sphere>(20, 20, radius);
	loadMeshs[name]->setVertices([](uint32_t i, Vertex& v) {v.color = {1.0f, 0.f, 0.f}; });
	sphereTest.mesh = loadMeshs[name];

	materialBuilderPhong.descriptorInfos[1][0].second = &descriptorImageInfoBlank;
	materials[name] = materialBuilderPhong.build();
	sphereTest.material = &materials[name];

	sphereTest.setPosition(modelSphere);
	renderables.emplace_back(sphereTest.mesh.get(), sphereTest.material, &sphereTest.transform);

	LOGI("create sphere A")
	{
		sphereA.mesh = loadMeshs[name];
		sphereA.material = &materials[name];
		sphereA.setPosition(modelSphere+Vector3f{0.f, 0.f, 5.0f});
		renderables.emplace_back(sphereA.mesh.get(), sphereA.material, &sphereA.transform);
	}

	// physics
	auto physicsSphereTest = std::make_shared<CranePhysics::Sphere>();
	physicsSphereTest->invMass = 0.1f;
	physicsSphereTest->position = sphereTest.position;
	physicsSphereTest->positionPrime = sphereTest.position;
	physicsSphereTest->radius = radius;
	sphereTestPhyIndex = pbd.rigidbodies.size();
	pbd.rigidbodies.push_back(physicsSphereTest);

	auto sphereAPhy = std::make_shared<CranePhysics::Sphere>();
	sphereAPhy->invMass = 0.1f;
	sphereAPhy->position = sphereA.position;
	sphereAPhy->positionPrime = sphereA.position;
	sphereAPhy->radius = radius;
	sphereAPhyIndex = pbd.rigidbodies.size();
	pbd.rigidbodies.push_back(sphereAPhy);

	CranePhysics::Rigidbody& a = *(pbd.rigidbodies[sphereTestPhyIndex]);
	CranePhysics::Rigidbody& b = *(pbd.rigidbodies[sphereAPhyIndex]);
	float distAB = (a.position -b.position).norm();
	float compress = 1.0f, stretch = 1.0f;
	pbd.constraints.emplace_back(std::make_shared<Stretching>(a, b, distAB, compress, stretch));
}

void CLOTH::createDragon()
{
	LOGI("create dragon");

	Eigen::Vector3f rotationDragon{ 45.f / 180.f * 3.14f, 0.f, 0.f };
	//Eigen::Vector3f rotationDragon{ 0.f, 0.f, 0.f };
	Eigen::Vector3f modelDragon{ 0.f, 1.f, 5.f };

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
		loadMeshs[name].get()->setVertices([](uint32_t i, Vertex& v) {v.color = { 1.f, 1.f, 1.f }; 
		v.position += Vector3f{0.f, 0.0f, 0.05f}; 
		v.position *= 10.f; 
		AngleAxis<float> aa(90.f / 180.f * 3.14f, Vector3f(1.f,0.f,0.f));
		v.position= aa*v.position;
		});
		loadMeshs[name].get()->recomputeNormals();
	}
	dragon.mesh = loadMeshs[name];


	if (materials.find(name) == materials.end())
	{
		materials[name] = materialBuilderPhong.build();
	}
	dragon.material = &materials[name];

	//dragon.setRotation(rotationDragon);
	dragon.setPosition(modelDragon);
	dragon.computeAABB();
	renderables.emplace_back(dragon.mesh.get(), dragon.material, &dragon.transform);


	vector<Vector3f> box1Vertex(dragon.mesh->data.size());
	for (uint32_t i = 0; i < dragon.mesh->data.size(); ++i)
		box1Vertex[i] = dragon.mesh->data[i].position;
	vector<unsigned> volume;
	vector<uint32_t> indices;
	uint32_t width = 100, height = 40, depth = 20;
	//uint32_t width = 2, height = 2, depth = 2;
	Voxelize(box1Vertex.data(), box1Vertex.size(), dragon.mesh->indices.data(), dragon.mesh->indices.size() / 3,
			 width, height, depth, volume, dragon.extentMin, dragon.extentMax);

	Vector3f len = dragon.extentMax - dragon.extentMin;
	for (uint32_t i = 0, offset = 2; i < volume.size(); ++i)
	{
		if (volume[i] == 1)
		{
			uint32_t z = i / (width * height);
			uint32_t y = i % (width * height) / width;
			uint32_t x = i % (width * height) % width;
			Vector3f pos = dragon.position + dragon.extentMin +
												 Vector3f(len.x() / width * (x+0.5f),
														  len.y() / height * (y+0.5f),
														  len.z() / depth * (z+0.5f));
			Actor box;
			box.mesh = loadMeshs["cubeTest"];
			box.material = &materials["cubeTest"];
			box.setPosition(pos);
			box.setRotation(dragon.rotation);
			box.setScale(Vector3f{len.x() / width, len.y() / height, len.z() / depth}/ 2.f);
			boxs.push_back(box);

			auto physicsCubeTest = std::make_shared<CranePhysics::Cube>();
			physicsCubeTest->invMass = 0.f;
			physicsCubeTest->position = pos;
			physicsCubeTest->positionPrime = pos;
			physicsCubeTest->width = len.x() / width;
			physicsCubeTest->height = len.y() / height;
			physicsCubeTest->depth = len.z() / depth;

			rgbDragon.push_back(physicsCubeTest);
			rgbDragon.back()->velocity = Vector3f(0.f, 0.f, 0.f);
			//pbd.rigidbodies.push_back(physicsCubeTest);
			//pbd.rigidbodies.back()->velocity = Vector3f(0.f, 0.f, 0.f);
			//indices.emplace_back(offset++);
		}
	}
	//pbd.mConstraints.emplace_back(std::make_shared<ShapeMatchingConstraint>(pbd, indices.size(), indices));
	for(auto &b:boxs)
		renderables.emplace_back(b.mesh.get(), b.material, &b.transform);
}

void CLOTH::createSoldiers()
{
	LOGI("craete soldier");

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
		materials[name] = materialBuilderPhong.build();
	}

	soldier.mesh = loadMeshs[name];
	soldier.material = &materials[name];
	soldier.setPosition(Vector3f{ 0.f, 0.f, -5.0f });
	renderables.emplace_back(soldier.mesh.get(), soldier.material, &soldier.transform);
}

*/
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
