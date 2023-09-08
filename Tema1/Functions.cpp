#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>
#include <math.h>

using namespace m1;

/*
* computes the upper left corner of a rectangle
* @params:
*	glm::vec3 center -> center of the rectangle
*	float length, width -> dimensions
*	float angle -> angle at which the rectangle is rotated
*/
glm::vec3 compute_upper_left_corner(glm::vec3 center, float length, float width, float angle) {
	glm::vec3 upper_left = glm::vec3(0, 0, 0);
	upper_left.x = center.x - length / 2;
	upper_left.y = center.y + width / 2;

	// rotate around the center
	upper_left -= center;
	upper_left = transform2D_1::Rotate(-angle) * upper_left;
	upper_left += center;

	return upper_left;
}

/*
* computes the upper right corner of a rectangle
* @params:
*	glm::vec3 center -> center of the rectangle
*	float length, width -> dimensions
*	float angle -> angle at which the rectangle is rotated
*/
glm::vec3 compute_upper_right_corner(glm::vec3 center, float length, float width, float angle) {
	glm::vec3 upper_right = glm::vec3(0, 0, 0);
	upper_right.x = center.x + length / 2;
	upper_right.y = center.y + width / 2;

	// rotate around the center
	upper_right -= center;
	upper_right = transform2D_1::Rotate(-angle) * upper_right;
	upper_right += center;

	return upper_right;
}

/*
* computes the lower left corner of a rectangle
* @params:
*	glm::vec3 center -> center of the rectangle
*	float length, width -> dimensions
*	float angle -> angle at which the rectangle is rotated
*/
glm::vec3 compute_lower_left_corner(glm::vec3 center, float length, float width, float angle) {
	glm::vec3 lower_left = glm::vec3(0, 0, 0);
	lower_left.x = center.x - length / 2;
	lower_left.y = center.y - width / 2;

	// rotate around the center
	lower_left -= center;
	lower_left = transform2D_1::Rotate(-angle) * lower_left;
	lower_left += center;

	return lower_left;
}

/*
* computes the lower right corner of a rectangle
* @params:
*	glm::vec3 center -> center of the rectangle
*	float length, width -> dimensions
*	float angle -> angle at which the rectangle is rotated
*/
glm::vec3 compute_lower_right_corner(glm::vec3 center, float length, float width, float angle) {
	glm::vec3 lower_right = glm::vec3(0, 0, 0);
	lower_right.x = center.x + length / 2;
	lower_right.y = center.y - width / 2;

	// rotate around the center
	lower_right -= center;
	lower_right = transform2D_1::Rotate(-angle) * lower_right;
	lower_right += center;

	return lower_right;
}

// INIT OBJECTS

void Tema1::create_bullet() {
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 direction = glm::vec3(cos(player->angle + M_PI_2), sin(player->angle + M_PI_2), 0);

	// compute the start position of the bullet
	position.y = player->main_square_side + player->secondary_square_side; // bullet will be spawned at the player's head (the 2 red squares)
	position = transform2D_1::Rotate(player->angle) * position; // player might be rotated, so the bullet must be spawned at the right place

	// translate the bullet to it's position
	position[0] += player->position.x;
	position.y += player->position.y;

	struct Bullet b = struct Bullet(position, direction, player->angle);

	bullets.push_back(b);
}

void Tema1::create_enemy() {
	// TODO: randomly generate the starting position
	struct Enemy e = Enemy(glm::vec3(0, 0, 0));

	// generate coordinates
	glm::vec3 map_center = glm::vec3(logicSpace.height / 2, logicSpace.width / 2, 0);

	float min_x = map_center.x - map_length / 2;
	float max_x = map_center.x + map_length / 2;

	float min_y = map_center.y - map_width / 2;
	float max_y = map_center.y + map_width / 2;

	e.position.x = min_x + (((float)rand()) / (float)RAND_MAX) * (max_x - min_x);
	e.position.y = min_y + (((float)rand()) / (float)RAND_MAX) * (max_y - min_y);

	// compute speed -> a random number between 0.8*avg_speed and 1.2*avg_speed
	float min_speed = 0.8 * enemy_average_speed;
	float max_speed = 1.2 * enemy_average_speed;
	e.speed = min_speed + (((float)rand()) / (float)RAND_MAX) * (max_speed - min_speed);

	// check if the enemy is outside the logic window
	// if it is, then it will be generated, else it won't
	float min_x_window = player->position.x - logicSpace.height / 2;
	float max_x_window = player->position.x + logicSpace.height / 2;
	float min_y_window = player->position.y - logicSpace.width / 2;
	float max_y_window = player->position.y + logicSpace.width / 2;

	if (!(e.position.x > min_x_window && e.position.x < max_x_window &&
		e.position.y > min_y_window && e.position.y < max_y_window)) {
		last_spawn_time = timestamp;
		enemies.push_back(e);
	}
}

// COLLISIONS

/*
* checks if a point is contained inside an axis-aligned rectangle
* glm::vec3 point - point coordinates
* glm::vec3 center - rectangle center
* float width, length - rectangle sizes
*/
bool point_inside_rectangle(glm::vec3 point, glm::vec3 center, float length, float width) {
	return point.x > center.x - length / 2 && point.x < center.x + length / 2
		&& point.y > center.y - width / 2 && point.y < center.y + width / 2;
}

/*
* checks if a point is contained inside a rectangle
* the rectangle might be rotated (not axis aligned)
*/
bool point_inside_rotated_rectangle(glm::vec3 point, glm::vec3 center, float length, float width, float angle) {
	// first we need to rotate the point around the rectangle's center
	// for that, we translate it with '-center', then we apply rotation, then we translate it back with 'center'

	// translation
	point.x -= center.x;
	point.y -= center.y;

	// rotation
	point = transform2D_1::Rotate(-angle) * point;

	// translation
	point.x += center.x;
	point.y += center.y;

	// now we can consider that the rectangle is axis aligned and apply the previous function
	return point_inside_rectangle(point, center, length, width);
}

bool Tema1::bullet_wall_collision(struct Bullet bullet) {
	// for each wall
	for (auto wall : walls) {
		// check if any corner of the bullet is inside the wall
		glm::vec3 upper_left = compute_upper_left_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
		glm::vec3 upper_right = compute_upper_right_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
		glm::vec3 lower_left = compute_lower_left_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
		glm::vec3 lower_right = compute_lower_right_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);

		bool inside = 
			point_inside_rectangle(upper_left, wall.center, wall.length, wall.width) ||
			point_inside_rectangle(upper_right, wall.center, wall.length, wall.width) ||
			point_inside_rectangle(lower_left, wall.center, wall.length, wall.width) ||
			point_inside_rectangle(lower_right, wall.center, wall.length, wall.width);

		if (inside) {
			return true;
		}
	}

	return false;
}

bool Tema1::bullet_map_collision(struct Bullet bullet) {
	glm::vec3 upper_left = compute_upper_left_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
	glm::vec3 upper_right = compute_upper_right_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
	glm::vec3 lower_left = compute_lower_left_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
	glm::vec3 lower_right = compute_lower_right_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);

	glm::vec3 map_center = glm::vec3(logicSpace.width / 2, logicSpace.height / 2, 0);

	// check if any corner of the bullet is outside the map
	bool outside =
		(!point_inside_rectangle(upper_left, map_center, map_length, map_width)) ||
		(!point_inside_rectangle(upper_right, map_center, map_length, map_width)) ||
		(!point_inside_rectangle(lower_left, map_center, map_length, map_width)) ||
		(!point_inside_rectangle(lower_right, map_center, map_length, map_width));

	return outside;
}

bool Tema1::bullet_enemy_collision(struct Bullet bullet) {
	// for each enemy
	for (int i = 0; i < enemies.size(); i++) {
		glm::vec3 upper_left = compute_upper_left_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
		glm::vec3 upper_right = compute_upper_right_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
		glm::vec3 lower_left = compute_lower_left_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);
		glm::vec3 lower_right = compute_lower_right_corner(bullet.position, bullet_square_size, bullet_square_size, bullet.angle);

		bool collision =
			point_inside_rotated_rectangle(upper_left, enemies[i].position, enemy_main_square_size, enemy_main_square_size, enemies[i].angle) ||
			point_inside_rotated_rectangle(upper_right, enemies[i].position, enemy_main_square_size, enemy_main_square_size, enemies[i].angle) ||
			point_inside_rotated_rectangle(lower_left, enemies[i].position, enemy_main_square_size, enemy_main_square_size, enemies[i].angle) ||
			point_inside_rotated_rectangle(lower_right, enemies[i].position, enemy_main_square_size, enemy_main_square_size, enemies[i].angle);

		if (collision) {
			enemies.erase(enemies.begin() + i);

			score++;
			if (score % 5 == 0) {
				cout << "Current score " << score << endl;
			}
			
			return true;
		}
	}

	return false;
}

bool Tema1::player_wall_collision(glm::vec3 next_pos) {
	glm::vec3 upper_left = compute_upper_left_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 upper_right = compute_upper_right_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 lower_left = compute_lower_left_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 lower_right = compute_lower_right_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);

	for (auto wall : walls) {
		// check if any of the player's corners is inside the obstacle
		bool collision =
			point_inside_rectangle(upper_left, wall.center, wall.length, wall.width) ||
			point_inside_rectangle(upper_right, wall.center, wall.length, wall.width) ||
			point_inside_rectangle(lower_left, wall.center, wall.length, wall.width) ||
			point_inside_rectangle(lower_right, wall.center, wall.length, wall.width);

		if (collision) {
			return true;
		}

		// also check if any of the obstacle's corners is inside the player
		glm::vec3 wall_upper_left = compute_upper_left_corner(wall.center, wall.length, wall.width, 0);
		glm::vec3 wall_upper_right = compute_upper_right_corner(wall.center, wall.length, wall.width, 0);
		glm::vec3 wall_lower_left = compute_lower_left_corner(wall.center, wall.length, wall.width, 0);
		glm::vec3 wall_lower_right = compute_lower_right_corner(wall.center, wall.length, wall.width, 0);

		collision =
			point_inside_rotated_rectangle(wall_upper_right, next_pos, player->main_square_side, player->main_square_side, player->angle) ||
			point_inside_rotated_rectangle(wall_upper_left, next_pos, player->main_square_side, player->main_square_side, player->angle) ||
			point_inside_rotated_rectangle(wall_lower_left, next_pos, player->main_square_side, player->main_square_side, player->angle) ||
			point_inside_rotated_rectangle(wall_lower_right, next_pos, player->main_square_side, player->main_square_side, player->angle);

		if (collision) {
			return true;
		}
	}

	return false;
}

bool Tema1::player_map_edges_collision(glm::vec3 next_pos) {
	glm::vec3 upper_left = compute_upper_left_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 upper_right = compute_upper_right_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 lower_left = compute_lower_left_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 lower_right = compute_lower_right_corner(next_pos, player->main_square_side, player->main_square_side, player->angle);

	glm::vec3 map_center = glm::vec3(logicSpace.height / 2, logicSpace.width / 2, 0);

	// check if any corner of the player's main square is outside the map
	bool outside =
		(!point_inside_rectangle(upper_left, map_center, map_length, map_width)) ||
		(!point_inside_rectangle(upper_right, map_center, map_length, map_width)) ||
		(!point_inside_rectangle(lower_left, map_center, map_length, map_width)) ||
		(!point_inside_rectangle(lower_right, map_center, map_length, map_width));

	return outside;
}

/*
* checks collisions between player and any enemy
*/
bool Tema1::player_enemy_collision(struct Enemy e) {
	// rectangle - rectangle collision
	// check if any of the player's corners is inside the enemy's rectangle

	glm::vec3 upper_left = compute_upper_left_corner(player->position, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 upper_right = compute_upper_right_corner(player->position, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 lower_left = compute_lower_left_corner(player->position, player->main_square_side, player->main_square_side, player->angle);
	glm::vec3 lower_right = compute_lower_right_corner(player->position, player->main_square_side, player->main_square_side, player->angle);

	bool collision =
		point_inside_rotated_rectangle(upper_left, e.position, enemy_main_square_size, enemy_main_square_size, e.angle) ||
		point_inside_rotated_rectangle(upper_right, e.position, enemy_main_square_size, enemy_main_square_size, e.angle) ||
		point_inside_rotated_rectangle(lower_left, e.position, enemy_main_square_size, enemy_main_square_size, e.angle) ||
		point_inside_rotated_rectangle(lower_right, e.position, enemy_main_square_size, enemy_main_square_size, e.angle);

	return collision;
}