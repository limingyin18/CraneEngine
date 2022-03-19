#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"
#include "ChessboardActor.hpp"
#include "SkeletonActor.hpp"
#include "CharacterActor.hpp"

#include <torch/script.h>
#include <opencv2/opencv.hpp>

#include "CVUtils.hpp"



class POSE : public SDL2_IMGUI_BASE
{
public:
	explicit POSE(std::shared_ptr<SDL_Window> win);
	~POSE();
	POSE(const POSE &rhs) = delete;
	POSE(POSE &&rhs) = delete;
	POSE &operator=(const POSE &rhs) = delete;
	POSE &operator=(POSE &&rhs) = delete;

private:
	void createAssetApp() override;

	void updateApp() override;
	void updatePhysics();
	void updateGraphics();

private:
	CranePhysics::PositionBasedDynamics pbd;

	std::shared_ptr<Crane::ChessboardActor> chessboardActor;
	std::shared_ptr<SkeletonActor> skeletonActor;

	Crane::Actor human;
	std::vector<BindCoff> bindCoffs;
	void createHuman();
	std::shared_ptr<CharacterActor> humanActor;

	void initCV();
	void detectKeypoints();
	std::array<std::pair<std::pair<float, float>, float>, 17> keypoints;
	torch::Device deviceTorch;
	torch::jit::script::Module module;
	cv::VideoCapture cap;
	cv::Mat img;
};