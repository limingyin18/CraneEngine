#pragma once

#include <vector>
#include <string>
#include <fstream>

namespace CraneVision
{
	class Loader
	{
	public:
		static std::vector<char> readFile(const std::string& filename);
	};
}
