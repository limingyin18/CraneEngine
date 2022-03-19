#include "POSE.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

POSE::POSE(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win), deviceTorch(torch::kCUDA)
{
	preferPresentMode = vk::PresentModeKHR::eFifo;
	camera.target = Vector3f{ 0.f, 10.f, 0.f };
	camera.cameraMoveSpeed = 10.f;
}

POSE::~POSE()
{
	device->waitIdle();
}

void POSE::createAssetApp()
{
	SDL2_IMGUI_BASE::createAssetApp();

	chessboardActor = make_shared<ChessboardActor>();
	chessboardActor->init(this, &pbd);
	scene.push_back(chessboardActor);

	skeletonActor = make_shared<SkeletonActor>();
	skeletonActor->init(this, &pbd);
	scene.push_back(skeletonActor);

	humanActor = make_shared<CharacterActor>();
	humanActor->init(this, &pbd, &skeletonActor->bones);
	scene.push_back(humanActor);

	initCV();
}

void POSE::updateApp()
{
    updateEngine();
	detectKeypoints();
	skeletonActor->update(keypoints);
	humanActor->update();

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
}
