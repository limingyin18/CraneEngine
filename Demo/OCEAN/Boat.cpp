#include "Boat.hpp"

using namespace std;
using namespace Crane;

void Boat::init(Render* ctx)
{
	Actor::init(ctx);

	LOGI("¶ÁÈ¡´¬Ä£ÐÍ");
	Eigen::Vector3f rotationBoat{ 0.f, 0.f, 0.f };
	Eigen::Vector3f positionBoat{ 0.f, 5.f, 0.f };

	string name = "boat";
	assets::AssetFile file;
	if (!assets::load_binaryfile((string("assets/") + "kapal.mesh").c_str(), file))
		throw std::runtime_error(string("Error when loading mesh "));
	assets::MeshInfo meshInfo = assets::read_mesh_info(&file);

	if (context->loadMeshs.find(name) == context->loadMeshs.end())
	{
		context->loadMeshs[name] = make_shared<MeshBase>();
		context->loadMeshs[name]->data.resize(meshInfo.vertexBuferSize / sizeof(Vertex));
		context->loadMeshs[name]->indices.resize(meshInfo.indexBuferSize / sizeof(uint32_t));
		assets::unpack_mesh(&meshInfo, file.binaryBlob.data(), file.binaryBlob.size(),
			reinterpret_cast<char*>(context->loadMeshs[name]->data.data()),
			reinterpret_cast<char*>(context->loadMeshs[name]->indices.data()));
		context->loadMeshs[name].get()->setVertices([](uint32_t i, Vertex& v) {v.position *=10.f; v.color = { 1.f, 1.f, 1.f }; });
		context->loadMeshs[name].get()->recomputeNormals();
	}
	mesh = context->loadMeshs[name];


	if (context->materials.find(name) == context->materials.end())
	{
		context->materialBuilderPhong.descriptorInfos[1][0].second = &context->descriptorImageInfoBlank;
		context->materials[name] = context->materialBuilderPhong.build();
	}
	material = &context->materials[name];

	setRotation(rotationBoat);
	setPosition(positionBoat);

	context->renderables.emplace_back(mesh.get(), material, &transform);

	bottomHeight = yMin();
	area = Area();
}

void Crane::Boat::updatePhysics()
{
	float y = seaHeight - m / rho / area - bottomHeight;
	setPosition({ position.x(), y, position.z() });
}

float Crane::Boat::yMin()
{
	return min_element(mesh->data.cbegin(), mesh->data.cend(), [](Vertex const& a, Vertex const& b)->bool {return a.position.y() < b.position.y(); })->position.y();
}

float Crane::Boat::Area()
{
	computeAABB();
	float x = extentMax.x() - extentMin.x();
	float z = extentMax.z() - extentMin.z();
	return x*z;
}
