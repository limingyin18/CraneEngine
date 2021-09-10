#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

class MINI_GAME : public SDL2_IMGUI_BASE
{
private:
	Crane::PipelinePassGraphics pipelinePassPhong;

	Crane::Chessboard chessboard{ 11, 11 };
	Crane::Material materialPhongChessboard; 
	Eigen::Vector3f modelChessboard{0.f, 0.f, 0.f};

	Crane::Plane cloak{11, 11};
	Crane::Material materialPhongCloak; 
	Eigen::Vector3f modelCloak{0.f, 5.f, -5.f};

	CranePhysics::PositionBasedDynamics pbd;


	void createAssetApp() override;
	void updateApp() override;

	void setImgui() override;

public:
	explicit MINI_GAME(std::shared_ptr<SDL_Window> win);
	~MINI_GAME();
	MINI_GAME(const MINI_GAME &rhs) = delete;
	MINI_GAME(MINI_GAME &&rhs) = delete;
	MINI_GAME &operator=(const MINI_GAME &rhs) = delete;
	MINI_GAME &operator=(MINI_GAME &&rhs) = delete;
};
