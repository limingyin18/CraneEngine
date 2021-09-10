#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

class CLOTH : public SDL2_IMGUI_BASE
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
	explicit CLOTH(std::shared_ptr<SDL_Window> win);
	~CLOTH();
	CLOTH(const CLOTH &rhs) = delete;
	CLOTH(CLOTH &&rhs) = delete;
	CLOTH &operator=(const CLOTH &rhs) = delete;
	CLOTH &operator=(CLOTH &&rhs) = delete;
};
