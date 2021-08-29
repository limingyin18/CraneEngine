#pragma once

#include <vector>
#include <string>
#include <fstream>

namespace Crane
{
	class Loader
	{
	public:
		static std::vector<char> readFile(const std::string& filename);
	};
}
