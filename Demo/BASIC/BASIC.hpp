#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

class BASIC : public SDL2_IMGUI_BASE
{

private:
	Eigen::Vector3f model1{0.f, 5.f, -5.f};
	Crane::Plane cloak{11, 11};

	Crane::Plane floorEnv{2, 2};
	Eigen::Vector3f model0{0.f, 0.f, 0.f};
	Crane::MaterialPhong materialPhong; 

	Crane::Cube cube{ 2 };

	CranePhysics::PositionBasedDynamics pbd;

	std::vector<Crane::Cube> particles;

	Crane::Image textureImage;
	vk::UniqueImageView textureImageView;


	void createAssetApp() override;
	void updateApp() override;

	void setImgui() override;

public:
	explicit BASIC(std::shared_ptr<SDL_Window> win);
	~BASIC();
	BASIC(const BASIC &rhs) = delete;
	BASIC(BASIC &&rhs) = delete;
	BASIC &operator=(const BASIC &rhs) = delete;
	BASIC &operator=(BASIC &&rhs) = delete;
};
