#pragma once

#include <numbers>
#include <chrono>
#include <iostream>
#include <fstream>
#include <Eigen/Eigen>
/**
 * @brief 天空类
*/
class Atmosphere
{
public:
	Eigen::Vector3f sunDirection; // 太阳光方向

	float earthRadius = 6360.e3; // 地球半径
	float atmosphereRadius = 6420e3; // 大气层半径
	float Hr = 7994.f; // 瑞利散射尺度
	float Hm = 1200.f; // 米氏散射尺度
	Eigen::Vector3f betaR{3.8e-6f, 13.5f, 33.1e-6f}; // 海平面瑞利散射系数
	Eigen::Vector3f betaM{21.0e-6f, 21.0e-6f, 21.0e-6f}; // 海平面米氏散射系数

	/**
	 * @brief 计算单个方向的天空颜色
	 * @param orig 视线起点位置
	 * @param dir 视线方向
	 * @param tmin 起始
	 * @param tmax 结束
	 * @return 天空颜色
	*/
	Eigen::Vector3f computeIncidentLight(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir,
		float tmin, float tmax) const;
};

void renderSkydome(const Eigen::Vector3f& sunDir, const char* filename);
