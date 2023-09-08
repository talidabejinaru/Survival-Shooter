#pragma once

#include "components/simple_scene.h"
#include "Player.h"

#include "lab_m1/Tema1/transform2D_1.h"
#include "lab_m1/Tema1/object2D_1.h"

namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        struct ViewportSpace
        {
            ViewportSpace() : x(0), y(0), width(1), height(1) {}
            ViewportSpace(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        struct LogicSpace
        {
            LogicSpace() : x(0), y(0), width(1), height(1) {}
            LogicSpace(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {}
            float x;
            float y;
            float width;
            float height;
        };

     public:
        Tema1();
        ~Tema1();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void DrawScene(glm::mat3(visMatrix));

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        glm::mat3 VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);
        glm::mat3 VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);

        void SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor = glm::vec3(0), bool clear = true);

     protected:
        ViewportSpace viewSpace;
        LogicSpace logicSpace;
        glm::mat3 modelMatrix, visMatrix;
        
        // player
        Player* player;
   
        // game variables
        float timestamp = 0;
        float death_timestamp;
        float max_health = 100;
        float current_health = max_health;
        float damage = 20; // health loss after a collision with an enemy
        bool game_over = false;
        int score = 0;

        // map info
        float map_width = 1500;
        float map_length = 1500;

        // WALLS
        struct Wall
        {
            Wall(glm::vec3 center, float length, float width)
                : center(center), length(length), width(width) {}
            glm::vec3 center;
            float width;
            float length;
        };
        vector<struct Wall> walls;

        // BULLETS
        void create_bullet();

        float bullet_square_size = 10;
        float bullet_speed = 300.0f;

        float fire_rate = 0.5f; // minimum interval between firing two consecutive bullets
        float last_fired = 0; // point in time at which the last bullet was fired

        struct Bullet
        {
            Bullet(glm::vec3 position, glm::vec3 direction, float angle)
                : position(position), direction(direction), angle(angle) {}
            glm::vec3 position;
            glm::vec3 direction;
            float angle;
            //float speed;
        };

        vector<struct Bullet> bullets;

        // ENEMIES
        void create_enemy();

        float enemy_main_square_size = 50;
        float enemy_secondary_square_size = 15;
        float enemy_average_speed = 180;

        float enemy_spawn_time = 3;
        float last_spawn_time = 0;

        struct Enemy
        {
            Enemy(glm::vec3 position)
                : position(position), direction(glm::vec3(0, 0, 0)), angle(0), speed(0) {
            }
            glm::vec3 position;
            glm::vec3 direction;
            float angle;
            float speed;
        };

        vector<struct Enemy> enemies;

        // HEALTHBAR
        float healthbar_length = 250;
        float healthbar_width = 60;
        glm::vec3 healthbar_center;


        // COLLISIONS
        bool bullet_wall_collision(struct Bullet bullet);
        bool bullet_map_collision(struct Bullet bullet);
        bool bullet_enemy_collision(struct Bullet bullet);

        bool player_wall_collision(glm::vec3 next_pos);
        bool player_map_edges_collision(glm::vec3 next_pos);
        bool player_enemy_collision(struct Enemy e);
    };
}   
