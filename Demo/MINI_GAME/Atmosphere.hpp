#pragma once

#include <numbers>
#include <chrono>
#include <iostream>
#include <fstream>
#include <Eigen/Eigen>
/**
 * @brief �����
*/
class Atmosphere
{
public:
	Eigen::Vector3f sunDirection; // ̫���ⷽ��

	float earthRadius = 6360.e3; // ����뾶
	float atmosphereRadius = 6420e3; // ������뾶
	float Hr = 7994.f; // ����ɢ��߶�
	float Hm = 1200.f; // ����ɢ��߶�
	Eigen::Vector3f betaR{3.8e-6f, 13.5f, 33.1e-6f}; // ��ƽ������ɢ��ϵ��
	Eigen::Vector3f betaM{21.0e-6f, 21.0e-6f, 21.0e-6f}; // ��ƽ������ɢ��ϵ��

	/**
	 * @brief ���㵥������������ɫ
	 * @param orig �������λ��
	 * @param dir ���߷���
	 * @param tmin ��ʼ
	 * @param tmax ����
	 * @return �����ɫ
	*/
	Eigen::Vector3f computeIncidentLight(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir,
		float tmin, float tmax) const;
};

void renderSkydome(const Eigen::Vector3f& sunDir, const char* filename);
