#include "POSE.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

void POSE::initCV()
{
	std::string filename = "assets/model.pt";
	// Deserialize the ScriptModule from a file using torch::jit::load()
	module = torch::jit::load(filename);
	module.to(deviceTorch);

	std::string fileNameImage = "assets/caixukun.mp4";
	cap = cv::VideoCapture(fileNameImage);
}

void POSE::detectKeypoints()
{
	cap.read(img);
	float weightImg = img.cols;
	float heightImg = img.rows;
	cv::Mat imgR;
	cv::resize(img, imgR, cv::Size(256, 256));

	// Create a vector of inputs
	torch::Tensor imgTensor = torch::from_blob(imgR.data, {imgR.rows, imgR.cols, 3}, torch::kByte);
	imgTensor = imgTensor.permute({2, 0, 1});
	imgTensor = imgTensor.toType(torch::kFloat);
	imgTensor = imgTensor.div(255);
	imgTensor = imgTensor.unsqueeze(0);
	imgTensor = imgTensor.to(deviceTorch);

	// Exectute the model
	at::Tensor output = module.forward({imgTensor}).toTensor();

	torch::Tensor outputCPU = output.to(torch::kCPU);
	std::vector<float> heatmap(outputCPU.data_ptr<float>(), outputCPU.data_ptr<float>() + outputCPU.numel());

	auto points = GetMaxPreds(heatmap);
	auto pointsPost = PostProcessingPoints(points, heatmap);
	keypoints = pointsPost;

	for (auto p : pointsPost)
	{
		cv::circle(img,
				   {static_cast<int>(4 * p.first.second * weightImg / 256.f),
					static_cast<int>(4 * p.first.first * heightImg / 256.f)},
				   2, {255, 0, 0}, 2);
	}

	cv::imshow("t", img);
	cv::waitKey(1);
}