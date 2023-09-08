#pragma once
#include "components/simple_scene.h"
#include "lab_m1/lab3/object2D.h"
#include <vector>
#include <iostream>
#include <string>

using namespace std;

namespace m1 {
	class Player
	{
	public:
		Player();
		~Player();
		Player(glm::vec3 position);

		glm::vec3 position;
		float angle = 0;

		const float main_square_side = 50;
		const float secondary_square_side = 20;

		const float speed = 200;

		// depth value so that the player will be displayed 'above' the map
		const float z = 0.01f;

		// bottom left corners for the squares which form the player
		const glm::vec3 main_square_corner = glm::vec3(-main_square_side / 2, -main_square_side / 2, z);
		const glm::vec3 secondary_left_square_corner = glm::vec3(-main_square_side / 2, main_square_side / 2, z);
		const glm::vec3 secondary_right_square_corner = glm::vec3(main_square_side / 2 - secondary_square_side, main_square_side / 2, z);

	private:
	};
}

