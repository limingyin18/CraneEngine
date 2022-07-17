#include "CLOTH.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

CLOTH::CLOTH(shared_ptr<SDL_Window> win) : SDL2_IMGUI_BASE(win)
{
    preferPresentMode = vk::PresentModeKHR::eFifo;
    camera.target = Vector3f{ 0.f, 1.8f, 10.f };
    camera.rotation[0] = -0.0f;
    camera.cameraMoveSpeed = 10.f;
}

CLOTH::~CLOTH()
{
    vkDeviceWaitIdle(device.get());
}

void CLOTH::updateApp()
{
    updateEngine();

    // physics update
    pbd.dt = 0.001f;
    //pbd.dt = dt;

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

    pbd.run();

    clothActor->update();

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

    /*
    ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::SetWindowPos(ImVec2{0.f, 0.f});
    ImGui::Text("count:\t%d", count++);
    ImGui::Text("FPS:\t%.3f ms/frame (%.1f FPS)", dt, 1.f / dt);
    ImGui::Text("camera position: %5.1f %5.1f %5.1f", camera.getCameraPos().x(), camera.getCameraPos().y(), camera.getCameraPos().z());
    ImGui::Text("camera rotation: %5.1f %5.1f %5.1f", camera.rotation.x(), camera.rotation.y(), camera.rotation.z());
    ImGui::End();
    */

    static bool opt_padding = false;
    //static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoSplit 
        //|ImGuiDockNodeFlags_NoResize ;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode
        ;


    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    window_flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        //ShowDockingDisabledMessage();
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {

            if (ImGui::MenuItem("Exit"))
            {
                SDL_Event event;
                event.type = SDL_QUIT;
                SDL_PushEvent(&event);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            //ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            //if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
            //if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            //if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
            //if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            //if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
    ImGui::SetNextWindowBgAlpha(0.001f);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    if (ImGui::TreeNode("Basic"))
    {
        ImGui::TreePop();
    }
    ImGui::End();

    ImGui::SetNextWindowBgAlpha(0.001f);
    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::End();

    ImGui::SetNextWindowBgAlpha(0.001f);
    ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::End();

    ImGui::SetNextWindowBgAlpha(0.001f);
    ImGui::Begin("Asset", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::End();

    ImGui::SetNextWindowBgAlpha(0.001f);
    ImGui::Begin("Model", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::End();
}

void CLOTH::createAssetApp()
{
    LOGI("create app asset");

    SDL2_IMGUI_BASE::createAssetApp();

    clothActor = make_shared<ClothActor>();
    clothActor->init(this, &pbd);
    scene.push_back(clothActor);

    soldierActor = make_shared<SoldierActor>();
    soldierActor->init(this, &pbd);
    scene.push_back(soldierActor);

    chessboardActor = make_shared<ChessboardActor>();
    chessboardActor->init(this, &pbd);
    createChessboard();
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
