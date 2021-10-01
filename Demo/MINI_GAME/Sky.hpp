#pragma once

#include <memory>
#include <string>
#include <fstream>

#include "Engine.hpp"

namespace Crane
{
	class Sky : public Actor
	{
	public:
		void init(Render *ctx);

		Crane::PipelinePassGraphics pipelinePass;
		MaterialBuilder materialBuilder;
	};
}