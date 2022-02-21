#pragma once

#include <iostream>

#include "numeric"
#include "Rigidbody.hpp"
#include "Intersect.hpp"


namespace CranePhysics
{
	class BVH
	{
		struct Node
		{
			uint32_t index = -1;
			Eigen::Vector3f aa;
			Eigen::Vector3f bb;

			Node* left = nullptr;
			Node* right = nullptr;

			~Node()
			{
				if(left != nullptr)
					delete left;
				if(right != nullptr)
					delete right;
			};
		};

	public:
		BVH(){};
		~BVH();
		BVH(const BVH &rhs) = delete;
		BVH(BVH &&rhs) = delete;
		BVH &operator=(const BVH &rhs) = delete;
		BVH &operator=(BVH &&rhs);

		BVH(const std::vector<std::shared_ptr<CranePhysics::Rigidbody>>& rbs);

		Node* build(const std::vector<std::shared_ptr<CranePhysics::Rigidbody>>& rbs, std::vector<uint32_t>& index, uint32_t start, uint32_t end);


		void check(CranePhysics::Rigidbody const & rb, Node *node, std::vector<uint32_t> &indices);

		Node* root = nullptr;
		const std::vector<std::shared_ptr<CranePhysics::Rigidbody>> *rgb = nullptr;
	};
}