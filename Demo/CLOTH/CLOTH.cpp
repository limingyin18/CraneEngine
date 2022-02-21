#include "CLOTH.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

CLOTH::CLOTH(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
	preferPresentMode = vk::PresentModeKHR::eFifo;
	camera.target = Vector3f{0.f, 1.8f, 10.f};
	camera.rotation[0] = -0.0f;
	camera.cameraMoveSpeed = 1.f;
}

CLOTH::~CLOTH()
{
	vkDeviceWaitIdle(device.get());
}

void CLOTH::updateApp()
{
	updateEngine();

	// physics update
	// pbd.dt = 0.001f;
	pbd.dt = dt;

	/*
		Vector3f u{ 2.f, 0.f, 0.f };
		float drag = 1.f;
		float lift = 1.f;
		for(size_t i = 0; i < cloak.mesh->indices.size(); i = i + 3)
		{
			size_t indexA = cloak.mesh->indices[i];
			size_t indexB = cloak.mesh->indices[i+1];
			size_t indexC = cloak.mesh->indices[i+2];
			CranePhysics::Particle &a = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexA].get());
			CranePhysics::Particle &b = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexB].get());
			CranePhysics::Particle &c = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexC].get());

			Vector3f AB = a.positionPrime - b.positionPrime;
			Vector3f AC = a.positionPrime - c.positionPrime;
			Vector3f ABC = AB.cross(AC);
			float area = ABC.norm() / 2;

			Vector3f vRel = (a.velocity + b.velocity + c.velocity)/3 - u;
			Vector3f n = ABC.normalized();
			n = vRel.dot(n) > 0 ? n : -n;

			Vector3f fd = drag * vRel.squaredNorm() * area * vRel.dot(n) * -vRel;
			float cosq = vRel.normalized().cross(n).norm();
			Vector3f fl = lift * vRel.squaredNorm() * area * cosq * (n.cross(vRel).cross(vRel));

			a.velocity += dt * a.invMass *(fd+fl);

			b.velocity += dt * b.invMass *(fd+fl);

			c.velocity += dt * c.invMass *(fd+fl);
		}
		*/

	// pbd.run();

	// cloak.mesh->setVertices([this](uint32_t i, Crane::Vertex &v){v.position = this->pbd.rigidbodies[offsetCloak+i]->position - this->cloak.position;});
	// cloak.mesh->recomputeNormals();

	// flagCloth.mesh->setVertices([this](uint32_t i, Crane::Vertex &v){v.position = this->pbd.rigidbodies[offsetPhyFlagCloth+i]->position - this->flagCloth.position;});
	// flagCloth.mesh->recomputeNormals();

	// sphereTest.setPosition(pbd.rigidbodies[sphereTestPhyIndex]->position);
	// sphereA.setPosition(pbd.rigidbodies[sphereAPhyIndex]->position);
	// LOGI("λ�� {}, {}, {}", sphereTest.position.x(), sphereTest.position.y(), sphereTest.position.z());
	// LOGI("�ٶ� {}, {}, {}", pbd.rigidbodies.back()->velocity.x(), pbd.rigidbodies.back()->velocity.y(), pbd.rigidbodies.back()->velocity.z());

	/*
	for(size_t i = 0; i < renderables.size(); ++i)
	{
		Matrix4f t = (Translation3f(pbd.rigidbodies[i]->position) * pbd.rigidbodies[i]->rotation).matrix();
		renderables[i].transformMatrix = t;
	}*/

	/*
		size_t offset = chessboard.mesh->data.size();
		for (size_t i = 0; i < cloak.mesh->data.size(); i++)
			vertices[offset++] = cloak.mesh->data[i];

		for (size_t i = 0; i < flagCloth.mesh->data.size(); i++)
			vertices[offset++] = flagCloth.mesh->data[i];
			*/

	// vertBuff.update(vertices.data());
}

void CLOTH::setImgui()
{
	static size_t count = 0;

	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowPos(ImVec2{0.f, 0.f});
	ImGui::Text("count:\t%d", count++);
	ImGui::Text("FPS:\t%.3f ms/frame (%.1f FPS)", dt, 1.f / dt);
	ImGui::Text("camera position: %5.1f %5.1f %5.1f", camera.getCameraPos().x(), camera.getCameraPos().y(), camera.getCameraPos().z());
	ImGui::Text("camera rotation: %5.1f %5.1f %5.1f", camera.rotation.x(), camera.rotation.y(), camera.rotation.z());
	ImGui::End();
}

void CLOTH::createAssetApp()
{
	LOGI("create app asset");

	SDL2_IMGUI_BASE::createAssetApp();

	clothActor = make_shared<ClothActor>();
	clothActor->init(this);
	scene.push_back(clothActor);
	// createChessboard();
	// createCloak();
	// createFlagCloth();
	// createCubeTest();
	// createDragon();
	// createSoldiers();
	// createSphereTest();

	/*
		for (auto& rb : pbd.rigidbodies)
			rb->computeAABB();

		for (auto& rb : rgbDragon)
			rb->computeAABB();

		pbd.bvh = BVH(rgbDragon);
		*/
}
