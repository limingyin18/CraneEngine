#include "POSE.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

POSE::POSE(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win), deviceTorch(torch::kCUDA)
{
	preferPresentMode = vk::PresentModeKHR::eFifo;
	camera.target = Vector3f{ 0.f, 10.f, 0.f };
	camera.cameraMoveSpeed = 1.f;

}

POSE::~POSE()
{
	device->waitIdle();
}

void POSE::createAssetApp()
{
	SDL2_IMGUI_BASE::createAssetApp();

    createChessboard();
	createBones();
	createHuman();

	initCV();
}

void POSE::updateApp()
{
    updateEngine();
	detectKeypoints();
	updateBones();

	updatePhysics();
	updateGraphics();
}

void POSE::updatePhysics()
{
	pbd.dt =dt;
	pbd.run();
}

void POSE::updateGraphics()
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

void POSE::updateBones()
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
	if(keypoints[0].second > detectPositive)
	{
		bones[0].setPosition(scale*Vector3f{nose.second, nose.first, 0.f});
		auto &b = bones[0];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[5].second > detectPositive)
	{
    	bones[1].setPosition(scale*Vector3f{lshoulder.second, lshoulder.first, 0.f});
		auto &b = bones[1];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[7].second > detectPositive)
	{
    	bones[2].setPosition(scale*Vector3f{lelbow.second, lelbow.first, 0.f});
		auto &b = bones[2];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[9].second > detectPositive)
	{
    	bones[3].setPosition(scale*Vector3f{lwrist.second, lwrist.first, 0.f});
		auto &b = bones[3];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}

    // right armsscale
	if(keypoints[6].second > detectPositive)
	{
    	bones[4].setPosition(scale*Vector3f{rshoulder.second, rshoulder.first, 0.f});
		auto &b = bones[4];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[8].second > detectPositive)
	{
    	bones[5].setPosition(scale*Vector3f{relbow.second, relbow.first, 0.f});
		auto &b = bones[5];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[10].second > detectPositive)
	{
    	bones[6].setPosition(scale*Vector3f{rwrist.second, rwrist.first, 0.f});
		auto &b = bones[6];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}

    // left legsscale
	if(keypoints[11].second > detectPositive)
	{
    	bones[7].setPosition(scale*Vector3f{lhip.second, lhip.first, 0.f});
		auto &b = bones[7];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[13].second > detectPositive)
	{
 	   bones[8].setPosition(scale*Vector3f{lknee.second, lknee.first, 0.f});
		auto &b = bones[8];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[15].second > detectPositive)
	{
    	bones[9].setPosition(scale*Vector3f{lankle.second, lankle.first, 0.f});
		auto &b = bones[9];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}

    // right legsscale
	if(keypoints[12].second > detectPositive)
	{
    	bones[10].setPosition(scale*Vector3f{rhip.second, rhip.first, 0.f});
		auto &b = bones[10];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[14].second > detectPositive)
	{
    	bones[11].setPosition(scale*Vector3f{rknee.second, rknee.first, 0.f});
		auto &b = bones[11];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
	}
	if(keypoints[16].second > detectPositive)
	{
 	    bones[12].setPosition(scale*Vector3f{rankle.second, rankle.first, 0.f});
		auto &b = bones[12];
		b.setPosition(b.position+ Vector3f{0.f, offset, 0.f});
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

	for(uint32_t i = 0; i < human.mesh->data.size(); ++i)
	{
		human.mesh->data[i].position = bones[bindCoffs[i].index].position + bindCoffs[i].diff;
	}
}