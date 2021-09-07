#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "SDL2_IMGUI_BASE.hpp"

class BASIC : public SDL2_IMGUI_BASE
{
private:
	Crane::Image imageAnime;
	vk::UniqueImageView imageViewAnime;
	vk::DescriptorImageInfo descriptorImageInfoAnime;

	Crane::PipelinePassGraphics pipelinePassPhong;

	Crane::Chessboard chessboard{ 11, 11 };
	Crane::Material materialPhongChessboard; 

	Crane::Cube box{ 2 };
	Crane::Material materialPhongBox; 


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
