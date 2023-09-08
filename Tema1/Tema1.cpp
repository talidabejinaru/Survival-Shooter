#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>
#include <math.h> 

#define PI 3.14159265

using namespace std;
using namespace m1;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema1::Tema1()
{
}


Tema1::~Tema1()
{
}


void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    //camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    logicSpace.x = 0;       // logic x
    logicSpace.y = 0;       // logic y
    logicSpace.width = 400;   // logic width
    logicSpace.height = 400;  // logic height

    srand(time(NULL));

    // player
    player = new Player(glm::vec3(logicSpace.width / 2, logicSpace.height / 2, 0));

    AddMeshToList(object2D_1::CreateSquare("player_main_square", player->main_square_corner, player->main_square_side, glm::vec3(0, 1, 1), true));
    AddMeshToList(object2D_1::CreateSquare("player_secondary_left_square", player->secondary_left_square_corner, player->secondary_square_side, glm::vec3(0.67, 0.02, 0.02), true));
    AddMeshToList(object2D_1::CreateSquare("player_secondary_right_square", player->secondary_right_square_corner, player->secondary_square_side, glm::vec3(0.67, 0.02, 0.02), true));

    // map
    AddMeshToList(object2D_1::CreateSquare("map", glm::vec3(-0.5, -0.5, 0), 1, glm::vec3(0.4, 0.4, 0.6), true));

    // create mesh for walls
    AddMeshToList(object2D_1::CreateSquare("wall", glm::vec3(-0.5, -0.5, 0.001), 1, glm::vec3(0, 1, 0), true));

    // store the walls into a vector
    walls.push_back(Wall(glm::vec3(-200, -200, 0), 400, 300));
    walls.push_back(Wall(glm::vec3(700, 0, 0), 100, 900));
    walls.push_back(Wall(glm::vec3(-100, 800, 0), 500, 300));
    walls.push_back(Wall(glm::vec3(700, 800, 0), 100, 300));
    walls.push_back(Wall(glm::vec3(200, 400, 0), 600, 100));

    // bullets
    AddMeshToList(object2D_1::CreateSquare("bullet", glm::vec3(-0.5, -0.5, 0.002), 1, glm::vec3(1, 0, 0), true));

    // enemy
    AddMeshToList(object2D_1::CreateSquare("enemy_main_square", glm::vec3(-0.5, -0.5, 0.002), 1, glm::vec3(1, 0, 0), true));
    AddMeshToList(object2D_1::CreateSquare("enemy_inner_square", glm::vec3(-0.5, -0.5, 0.003), 1, glm::vec3(0, 0, 1), true));
    AddMeshToList(object2D_1::CreateSquare("enemy_secondary_main_square", glm::vec3(-0.5, -0.5, 0.002), 1, glm::vec3(0, 0, 0), true));

    // healthbar
    AddMeshToList(object2D_1::CreateSquare("wireframe", glm::vec3(-0.5, -0.5, 0.1), 1, glm::vec3(1, 0, 0), false));
    AddMeshToList(object2D_1::CreateSquare("healthbar", glm::vec3(-0.5, -0.5, 0.1), 1, glm::vec3(1, 0, 0), true));

    // setting initial healthbar center (upper right corner)
    healthbar_center.x = 350;
    healthbar_center.y = 350;

    resolution = window->GetResolution();

    viewSpace = ViewportSpace(0, 0, resolution.x, resolution.y);
    SetViewportArea(viewSpace, glm::vec3(0), true);
}


glm::mat3 Tema1::VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
{
    float sx, sy, tx, ty, smin;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    if (sx < sy)
        smin = sx;
    else
        smin = sy;
    tx = viewSpace.x - smin * logicSpace.x + (viewSpace.width - smin * logicSpace.width) / 2;
    ty = viewSpace.y - smin * logicSpace.y + (viewSpace.height - smin * logicSpace.height) / 2;

    return glm::transpose(glm::mat3(
        smin, 0.0f, tx,
        0.0f, smin, ty,
        0.0f, 0.0f, 1.0f));
}

void Tema1::SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear)
{
    glViewport(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(colorColor.r, colorColor.g, colorColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    GetSceneCamera()->SetOrthographic((float)viewSpace.x, (float)(viewSpace.x + viewSpace.width), (float)viewSpace.y, (float)(viewSpace.y + viewSpace.height), 0.1f, 400);
    GetSceneCamera()->Update();
}


void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema1::Update(float deltaTimeSeconds)
{   
    if (deltaTimeSeconds > 0.3) {
        deltaTimeSeconds = 0.3;
    }

    timestamp += deltaTimeSeconds;

    // Compute the 2D visualization matrix
    visMatrix = glm::mat3(1);
    visMatrix *= VisualizationTransf2DUnif(logicSpace, viewSpace);

    // after dying, the last frame will remain on the screen for a few more seconds
    if (game_over) {
        DrawScene(visMatrix);
        if (timestamp - death_timestamp > 2) {
            exit(0);
        }
        return;
    }

    // create enemy
    if (timestamp > last_spawn_time + enemy_spawn_time) {
        create_enemy();
    }

    // update bullets
    for (int i = 0; i < bullets.size(); i++) {
        bullets[i].position += bullets[i].direction * bullet_speed * deltaTimeSeconds;

        glm::vec3 pos = bullets[i].position;
        if (bullet_wall_collision(bullets[i]) || bullet_map_collision(bullets[i]) || bullet_enemy_collision(bullets[i])) {
            bullets.erase(bullets.begin() + i);
        }
    }

    // update enemies
    for (int i = 0; i < enemies.size(); i++) {
        float y = player->position.y - enemies[i].position.y;
        float x = player->position.x - enemies[i].position.x;

        // update it's angle
        enemies[i].angle = atan2(y, x) - M_PI_2;

        // update it's position
        if (enemies[i].position.x - player->position.x < -1) {
            enemies[i].position.x += enemies[i].speed * deltaTimeSeconds;
        } else if (enemies[i].position.x - player->position.x > 1) {
            enemies[i].position.x -= enemies[i].speed * deltaTimeSeconds;
        }

        if (enemies[i].position.y - player->position.y < -1) {
            enemies[i].position.y += enemies[i].speed * deltaTimeSeconds;
        }
        else if (enemies[i].position.y - player->position.y > 1) {
            enemies[i].position.y -= enemies[i].speed * deltaTimeSeconds;
        }

        if (player_enemy_collision(enemies[i])) {
            enemies.erase(enemies.begin() + i);

            // make sure the health doesn't go negative
            if (current_health - damage < 0) {
                current_health = 0;
            }
            else {
                current_health -= damage;
            }

            // if current_health is 0, then it's game over
            if (current_health < 0.01f) {
                game_over = true;
                death_timestamp = timestamp;

                cout << "Game over!" << endl;
                cout << "Final score " << score << endl;
            }
        }
    }

    DrawScene(visMatrix);
}

void Tema1::DrawScene(glm::mat3 visMatrix) {
    // player main square
    modelMatrix = glm::mat3(1);
    modelMatrix = transform2D_1::Rotate(player->angle) * modelMatrix;
    modelMatrix = transform2D_1::Translate(player->position.x, player->position.y) * modelMatrix;
    modelMatrix = visMatrix * modelMatrix;
    RenderMesh2D(meshes["player_main_square"], shaders["VertexColor"], modelMatrix);

    // player secondary left square
    modelMatrix = glm::mat3(1);
    modelMatrix = transform2D_1::Rotate(player->angle) * modelMatrix;
    modelMatrix = transform2D_1::Translate(player->position.x, player->position.y) * modelMatrix;
    modelMatrix = visMatrix * modelMatrix;
    RenderMesh2D(meshes["player_secondary_left_square"], shaders["VertexColor"], modelMatrix);

    // player secondary right square
    modelMatrix = glm::mat3(1);
    modelMatrix = transform2D_1::Rotate(player->angle) * modelMatrix;
    modelMatrix = transform2D_1::Translate(player->position.x, player->position.y) * modelMatrix;
    modelMatrix = visMatrix * modelMatrix;
    RenderMesh2D(meshes["player_secondary_right_square"], shaders["VertexColor"], modelMatrix);

    // map
    modelMatrix = glm::mat3(1);
    modelMatrix = transform2D_1::Scale(map_width, map_length) * modelMatrix;
    modelMatrix = transform2D_1::Translate(logicSpace.width / 2, logicSpace.height / 2) * modelMatrix;
    modelMatrix = visMatrix * modelMatrix;
    RenderMesh2D(meshes["map"], shaders["VertexColor"], modelMatrix);

    // walls
    for (auto wall : walls) {
        modelMatrix = glm::mat3(1);
        modelMatrix = transform2D_1::Scale(wall.length, wall.width) * modelMatrix;
        modelMatrix = transform2D_1::Translate(wall.center.x, wall.center.y) * modelMatrix;
        modelMatrix = visMatrix * modelMatrix;
        RenderMesh2D(meshes["wall"], shaders["VertexColor"], modelMatrix);
    }

    // bullets
    for (auto bullet : bullets) {
        modelMatrix = glm::mat3(1);
        modelMatrix = transform2D_1::Scale(bullet_square_size, bullet_square_size) * modelMatrix;
        modelMatrix = transform2D_1::Rotate(bullet.angle) * modelMatrix; // might not need this; UPDATE MIGHT NEED IT
        modelMatrix = transform2D_1::Translate(bullet.position.x, bullet.position.y) * modelMatrix;
        modelMatrix = visMatrix * modelMatrix;
        RenderMesh2D(meshes["bullet"], shaders["VertexColor"], modelMatrix);
    }

    // enemies
    for (auto enemy : enemies) {
        // main square
        modelMatrix = glm::mat3(1);
        modelMatrix = transform2D_1::Scale(enemy_main_square_size, enemy_main_square_size) * modelMatrix;
        modelMatrix = transform2D_1::Rotate(enemy.angle) * modelMatrix;
        modelMatrix = transform2D_1::Translate(enemy.position.x, enemy.position.y) * modelMatrix;
        modelMatrix = visMatrix * modelMatrix;
        RenderMesh2D(meshes["enemy_main_square"], shaders["VertexColor"], modelMatrix);

        // secondary square
        float offset = enemy_main_square_size / 2 + enemy_secondary_square_size / 2;

        modelMatrix = glm::mat3(1);
        modelMatrix = transform2D_1::Scale(enemy_secondary_square_size, enemy_secondary_square_size) * modelMatrix;
        modelMatrix = transform2D_1::Translate(0, offset) * modelMatrix;
        modelMatrix = transform2D_1::Rotate(enemy.angle) * modelMatrix;
        modelMatrix = transform2D_1::Translate(enemy.position.x, enemy.position.y) * modelMatrix;
        modelMatrix = visMatrix * modelMatrix;
        RenderMesh2D(meshes["enemy_secondary_main_square"], shaders["VertexColor"], modelMatrix);

        // inner square
        float size = enemy_main_square_size * sqrt(2) / 2; // inner square length

        modelMatrix = glm::mat3(1);
        modelMatrix = transform2D_1::Scale(size, size) * modelMatrix;
        modelMatrix = transform2D_1::Rotate(M_PI_4) * modelMatrix;
        modelMatrix = transform2D_1::Rotate(enemy.angle) * modelMatrix;
        modelMatrix = transform2D_1::Translate(enemy.position.x, enemy.position.y) * modelMatrix;
        modelMatrix = visMatrix * modelMatrix;
        RenderMesh2D(meshes["enemy_inner_square"], shaders["VertexColor"], modelMatrix);
    }

    // healthbar

    // draw the wireframe
    modelMatrix = glm::mat3(1);
    modelMatrix = transform2D_1::Scale(healthbar_length, healthbar_width) * modelMatrix;
    modelMatrix = transform2D_1::Translate(healthbar_center.x, healthbar_center.y) * modelMatrix;
    modelMatrix = visMatrix * modelMatrix;
    RenderMesh2D(meshes["wireframe"], shaders["VertexColor"], modelMatrix);

    // draw the healthbar itself
    modelMatrix = glm::mat3(1);
    modelMatrix = transform2D_1::Scale(healthbar_length, healthbar_width) * modelMatrix;

    // translate it so that both lower/upper left corners are on the Oy axis
    modelMatrix = transform2D_1::Translate(healthbar_length / 2, 0) * modelMatrix;

    // now scale it according to the player's current health
    modelMatrix = transform2D_1::Scale(current_health / max_health, 1) * modelMatrix;

    // translate it back so that the center will be in (0, 0) again
    modelMatrix = transform2D_1::Translate(-healthbar_length / 2, 0) * modelMatrix;

    modelMatrix = transform2D_1::Translate(healthbar_center.x, healthbar_center.y) * modelMatrix;
    modelMatrix = visMatrix * modelMatrix;
    RenderMesh2D(meshes["healthbar"], shaders["VertexColor"], modelMatrix);
}


void Tema1::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    if (game_over) {
        return;
    }

    if (window->KeyHold(GLFW_KEY_W)) {
        // compute the player's position at the next frame
        glm::vec3 next_pos = player->position;
        next_pos.y += player->speed * deltaTime;

        // check if collision will occur; if it doesn't, then update the position
        if (!player_wall_collision(next_pos) && !player_map_edges_collision(next_pos)) {
            player->position = next_pos;
            logicSpace.y += player->speed * deltaTime;
            healthbar_center.y += player->speed * deltaTime;
        }
    }

    if (window->KeyHold(GLFW_KEY_S)) {
        glm::vec3 next_pos = player->position;
        next_pos.y -= player->speed * deltaTime;

        if (!player_wall_collision(next_pos) && !player_map_edges_collision(next_pos)) {
            player->position = next_pos;
            logicSpace.y -= player->speed * deltaTime;
            healthbar_center.y -= player->speed * deltaTime;
        }
    }
        
    if (window->KeyHold(GLFW_KEY_D)) {
        glm::vec3 next_pos = player->position;
        next_pos.x += player->speed * deltaTime;

        if (!player_wall_collision(next_pos) && !player_map_edges_collision(next_pos)) {
            player->position = next_pos;
            logicSpace.x += player->speed * deltaTime;
            healthbar_center.x += player->speed * deltaTime;
        }
    }
        
    if (window->KeyHold(GLFW_KEY_A)) {
        glm::vec3 next_pos = player->position;
        next_pos.x -= player->speed * deltaTime;

        if (!player_wall_collision(next_pos) && !player_map_edges_collision(next_pos)) {
            player->position = next_pos;
            logicSpace.x -= player->speed * deltaTime;
            healthbar_center.x -= player->speed * deltaTime;
        }
    }
}


void Tema1::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_F) {
        //cout << bullets[0].position << endl;
    }
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{   
    if (game_over) {
        return;
    }

    glm::ivec2 resolution = window->GetResolution();

    double x = (double)(mouseX - resolution.x / 2);
    double y = (double)(resolution.y / 2 - mouseY);

    player->angle = atan2(y, x) - M_PI_2;
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{   
    if (game_over) {
        return;
    }

    // Add mouse button press event
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        if (timestamp - fire_rate > last_fired) {
            last_fired = timestamp;
            create_bullet();
        }
    }
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema1::OnWindowResize(int width, int height)
{
}
