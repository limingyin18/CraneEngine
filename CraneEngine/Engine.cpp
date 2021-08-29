#include "Engine.hpp"

using namespace std;
using namespace Crane;

Engine::Engine() : Render()
{
	camera.cameraMoveSpeed = 0.025f;
	camera.cameraRotateSpeed = 0.5f;
}

void Engine::updateEngine()
{
	auto timeNow = chrono::system_clock::now();
	std::chrono::duration<float> elapsed = timeNow - mTime;
	mTime = timeNow;
	dt = elapsed.count();
    dt = 0.001f;
	dtAll += dt;

    updateInput();
}

void Engine::updateInput()
{
    if (input.keys["lctrl"])
    {
        float x = (input.mousePos.first - input.mousePosPrev.first) / width * 10;
        float y = (input.mousePos.second - input.mousePosPrev.second) / height * 10;

        camera.rotate(x, y);
        camera.focus(input.scrollOffset.first);

        input.mousePosPrev = input.mousePos;
        input.scrollOffset = { 0.f, 0.f };
    }

    if (input.keys["a"])
    {
        camera.target = camera.target - camera.right * camera.cameraMoveSpeed;
    }

    if (input.keys["w"])
    {
        camera.target = camera.target + camera.front * camera.cameraMoveSpeed;
    }

    if (input.keys["s"])
    {
        camera.target = camera.target - camera.front * camera.cameraMoveSpeed;
    }

    if (input.keys["d"])
    {
        camera.target = camera.target + camera.right * camera.cameraMoveSpeed;
    }

    if (input.keys["q"])
    {
        camera.target = camera.target + camera.up * camera.cameraMoveSpeed;
    }

    if (input.keys["e"])
    {
        camera.target = camera.target - camera.up * camera.cameraMoveSpeed;
    }

    camera.computeTransformation();
}