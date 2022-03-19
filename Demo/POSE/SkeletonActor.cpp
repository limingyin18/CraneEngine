#include "SkeletonActor.hpp"

using namespace std;
using namespace Crane;

void SkeletonActor::init(Crane::Engine *c, CranePhysics::PositionBasedDynamics *p)
{
	LOGI("create skeleton")

	ctx = c;
	pbd = p;

	bones.resize(13);
	for (auto &bone : bones)
	{
		bone = make_shared<GraphicsPrimitive>();
		bone->transformer.setTransformParent(&transformer.getTransformWorld());
		primitives.push_back(bone);
	}

	{
		string name = "bone";
		if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
		{
			ctx->meshRepository[name] = make_shared<Crane::Cube>(2);
			ctx->meshRepository[name]->setVertices([](uint32_t i, Vertex &v)
												   { v.position *= 0.1f; });
		}

		for (auto &bone : bones)
			bone->mesh = ctx->meshRepository[name];
	}
	{
		string name = "phongBlank";
		if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
			ctx->materialRepository[name] = ctx->materialBuilderPhong.build();

		for (auto &bone : bones)
			bone->material = ctx->materialRepository[name];
	}
	{
		Vector3f position = {0.f, 4.5f, -20.f};
		bones[0]->transformer.setPosition(position);

		float scale = 2.5f;
		// left arms
		bones[1]->transformer.setPosition(bones[0]->transformer.getPosition() +
										  scale * Vector3f{-0.30f, -0.30f, 0.f});
		bones[2]->transformer.setPosition(bones[1]->transformer.getPosition() +
										  scale * Vector3f{-0.15f, -0.15f, 0.f});
		bones[3]->transformer.setPosition(bones[2]->transformer.getPosition() +
										  scale * Vector3f{-0.15f, -0.15f, 0.f});
		// right arms->transformer.getPosition()
		bones[4]->transformer.setPosition(bones[0]->transformer.getPosition() +
										  scale * Vector3f{0.30f, -0.30f, 0.f});
		bones[5]->transformer.setPosition(bones[4]->transformer.getPosition() +
										  scale * Vector3f{0.15f, -0.15f, 0.f});
		bones[6]->transformer.setPosition(bones[5]->transformer.getPosition() +
										  scale * Vector3f{0.15f, -0.15f, 0.f});
		// left legs->transformer.getPosition()
		bones[7]->transformer.setPosition(bones[0]->transformer.getPosition() +
										  scale * Vector3f{-0.15f, -0.80f, 0.f});
		bones[8]->transformer.setPosition(bones[7]->transformer.getPosition() +
										  scale * Vector3f{0.f, -0.40f, 0.f});
		bones[9]->transformer.setPosition(bones[8]->transformer.getPosition() +
										  scale * Vector3f{0.f, -0.40f, 0.f});
		// right legs
		bones[10]->transformer.setPosition(bones[0]->transformer.getPosition() +
										   scale * Vector3f{0.15f, -0.80f, 0.f});
		bones[11]->transformer.setPosition(bones[10]->transformer.getPosition() +
										   scale * Vector3f{0.f, -0.40f, 0.f});
		bones[12]->transformer.setPosition(bones[11]->transformer.getPosition() +
										   scale * Vector3f{0.f, -0.40f, 0.f});
	}

	{
		// load skeleton
		string fileName = "kachujin_g_rosales_GLTF/MESH_0_Kachujin.bones";
		assets::AssetFile skeletonFile;
		if (!load_binaryfile((string("assets/") + fileName).c_str(), skeletonFile))
			throw runtime_error("load asset failed");

		assets::SkeletonInfo skeletonInfo = assets::read_skeleton_info(&skeletonFile);

		numBoneVertices = skeletonInfo.numBoneVertices;
		uint32_t num = accumulate(numBoneVertices.cbegin(), numBoneVertices.cend(), 0);
		uint32_t fullsize = num * sizeof(assets::VertexBoneData);

		vector<uint8_t> buffer(fullsize);
		assets::unpack_skeleton(&skeletonInfo, skeletonFile.binaryBlob.data(), skeletonFile.binaryBlob.size(), (char *)buffer.data());

		vertexBoneData.resize(num);
		memcpy(vertexBoneData.data(), buffer.data(), buffer.size());
	}

	{
		// mesh
		soldierGP = make_shared<GraphicsPrimitive>();
		{
			string name = "kachujin";
			assets::AssetFile file;
			if (!assets::load_binaryfile((string("assets/kachujin_g_rosales_GLTF/") + "MESH_0_Kachujin.mesh").c_str(), file))
				throw std::runtime_error(string("Error when loading mesh "));
			assets::MeshInfo meshInfo = assets::read_mesh_info(&file);

			shared_ptr<Crane::MeshBase> mesh;
			if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
			{
				ctx->meshRepository[name] = make_shared<MeshBase>();
				ctx->meshRepository[name]->data.resize(meshInfo.vertexBuferSize / sizeof(Vertex));
				ctx->meshRepository[name]->indices.resize(meshInfo.indexBuferSize / sizeof(uint32_t));
				assets::unpack_mesh(&meshInfo, file.binaryBlob.data(), file.binaryBlob.size(),
									reinterpret_cast<char *>(ctx->meshRepository[name]->data.data()),
									reinterpret_cast<char *>(ctx->meshRepository[name]->indices.data()));
				ctx->meshRepository[name].get()->setVertices([](uint32_t i, Vertex &v)
															 { v.color = {1.f, 1.f, 1.f}; });
				ctx->meshRepository[name].get()->recomputeNormals();
			}
			mesh = ctx->meshRepository[name];
			soldierGP->mesh = mesh;
		}

		{
			string name = "phongBlank";
			if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
				ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
			soldierGP->material = ctx->materialRepository[name];
		}

		{
			Vector3f posSoldier{0.f, 0.f, -5.0f};
			soldierGP->transformer.setPosition(posSoldier);
		}
		primitives.push_back(soldierGP);
	}

	{
		vertexWeights.resize(soldierGP->mesh->data.size());
		uint32_t index = 0;
		for(uint32_t i = 0; i < numBoneVertices.size(); ++i)
		{
			for(uint32_t j = 0; j < numBoneVertices[i]; ++j)
			{
				vertexWeights[vertexBoneData[index].index].indices.push_back(i);
				vertexWeights[vertexBoneData[index].index].weights.push_back(vertexBoneData[index].weight);
				++index;
			}
		}
	}
}

void SkeletonActor::update(const std::array<std::pair<std::pair<float, float>, float>, 17> &keypoints)
{
	pair<float, float> nose = keypoints[0].first;
	pair<float, float> lshoulder = keypoints[5].first;
	pair<float, float> lelbow = keypoints[7].first;
	pair<float, float> lwrist = keypoints[9].first;
	pair<float, float> rshoulder = keypoints[6].first;
	pair<float, float> relbow = keypoints[8].first;
	pair<float, float> rwrist = keypoints[10].first;
	pair<float, float> lhip = keypoints[11].first;
	pair<float, float> lknee = keypoints[13].first;
	pair<float, float> lankle = keypoints[15].first;
	pair<float, float> rhip = keypoints[12].first;
	pair<float, float> rknee = keypoints[14].first;
	pair<float, float> rankle = keypoints[16].first;

	float scale = -0.1f;
	float offset = 10.f;
	float detectPositive = 0.5f;
	// left arms
	if (keypoints[0].second > detectPositive)
	{
		bones[0]->transformer.setPosition(scale * Vector3f{nose.second, nose.first, 0.f});
		auto &b = bones[0];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[5].second > detectPositive)
	{
		bones[1]->transformer.setPosition(scale * Vector3f{lshoulder.second, lshoulder.first, 0.f});
		auto &b = bones[1];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[7].second > detectPositive)
	{
		bones[2]->transformer.setPosition(scale * Vector3f{lelbow.second, lelbow.first, 0.f});
		auto &b = bones[2];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[9].second > detectPositive)
	{
		bones[3]->transformer.setPosition(scale * Vector3f{lwrist.second, lwrist.first, 0.f});
		auto &b = bones[3];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}

	// right armsscale
	if (keypoints[6].second > detectPositive)
	{
		bones[4]->transformer.setPosition(scale * Vector3f{rshoulder.second, rshoulder.first, 0.f});
		auto &b = bones[4];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[8].second > detectPositive)
	{
		bones[5]->transformer.setPosition(scale * Vector3f{relbow.second, relbow.first, 0.f});
		auto &b = bones[5];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[10].second > detectPositive)
	{
		bones[6]->transformer.setPosition(scale * Vector3f{rwrist.second, rwrist.first, 0.f});
		auto &b = bones[6];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}

	// left legsscale
	if (keypoints[11].second > detectPositive)
	{
		bones[7]->transformer.setPosition(scale * Vector3f{lhip.second, lhip.first, 0.f});
		auto &b = bones[7];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[13].second > detectPositive)
	{
		bones[8]->transformer.setPosition(scale * Vector3f{lknee.second, lknee.first, 0.f});
		auto &b = bones[8];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[15].second > detectPositive)
	{
		bones[9]->transformer.setPosition(scale * Vector3f{lankle.second, lankle.first, 0.f});
		auto &b = bones[9];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}

	// right legsscale
	if (keypoints[12].second > detectPositive)
	{
		bones[10]->transformer.setPosition(scale * Vector3f{rhip.second, rhip.first, 0.f});
		auto &b = bones[10];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[14].second > detectPositive)
	{
		bones[11]->transformer.setPosition(scale * Vector3f{rknee.second, rknee.first, 0.f});
		auto &b = bones[11];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}
	if (keypoints[16].second > detectPositive)
	{
		bones[12]->transformer.setPosition(scale * Vector3f{rankle.second, rankle.first, 0.f});
		auto &b = bones[12];
		b->transformer.setPosition(b->transformer.getPosition() + Vector3f{0.f, offset, 0.f});
	}

	/*
	// left arms
	bones[1].setPosition(bones[0].position + -scale*Vector3f{lshoulder.second-nose.second, lshoulder.first-nose.first, 0.f});
	bones[2].setPosition(bones[1].position + -scale*Vector3f{lelbow.second-lshoulder.second, lelbow.first-lshoulder.first, 0.f});
	bones[3].setPosition(bones[2].position + -scale*Vector3f{lwrist.second-lelbow.second, lwrist.first-lelbow.first, 0.f});

	// right armsscale
	bones[4].setPosition(bones[0].position + -scale*Vector3f{rshoulder.second-nose.second, rshoulder.first-nose.first, 0.f});
	bones[5].setPosition(bones[4].position + -scale*Vector3f{relbow.second-rshoulder.second, relbow.first-rshoulder.first, 0.f});
	bones[6].setPosition(bones[5].position + -scale*Vector3f{rwrist.second-relbow.second, rwrist.first-relbow.first, 0.f});

	// left legsscale
	bones[7].setPosition(bones[0].position + -scale*Vector3f{lhip.second-nose.second, lhip.first-nose.first, 0.f});
	bones[8].setPosition(bones[7].position + -scale*Vector3f{lknee.second-lhip.second, lknee.first-lhip.first, 0.f});
	bones[9].setPosition(bones[8].position + -scale*Vector3f{lankle.second-lknee.second, lankle.first-lknee.first, 0.f});

	// right legsscale
	bones[10].setPosition(bones[0].position + -scale*Vector3f{rhip.second-nose.second, rhip.first-nose.first, 0.f});
	bones[11].setPosition(bones[10].position +-scale*Vector3f{rknee.second-rhip.second, rknee.first-rhip.first, 0.f});
	bones[12].setPosition(bones[11].position +-scale*Vector3f{rankle.second-rknee.second, rankle.first-rknee.first, 0.f});
	*/

	/*
	for(uint32_t i = 0; i < human.mesh->data.size(); ++i)
	{
		human.mesh->data[i].position = bones[bindCoffs[i].index].position + bindCoffs[i].diff;
	}
	*/
}