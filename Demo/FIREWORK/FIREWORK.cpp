#include "FIREWORK.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;
using namespace Crane;

FIREWORK::FIREWORK(shared_ptr<SDL_Window> win):SDL2_IMGUI_BASE(win)
{
    
}

void FIREWORK::updateApp()
{
   updateEngine();
   updateGraphics(); 
}
