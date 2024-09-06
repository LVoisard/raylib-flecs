#include <iostream>
#include "raylib.h"
#include "raymath.h"
#include "flecs.h"
#include "components.h"
#include "systems.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <string>

#include <filesystem>

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    const int halfExtentX = 100;
    const int halfExtentY = 50;
    const int halfExtentZ = 100;
    const int NB_OF_PARTICLES = 50000;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(100);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    flecs::world world;

    Mesh instancedMesh = GenMeshSphere(1, 4, 4);

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Matrix);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Damping);
    ECS_COMPONENT(world, Radius);
    ECS_COMPONENT(world, Color);
    ECS_COMPONENT(world, Size3D);

    world.set<Game>({ screenWidth, screenHeight, halfExtentX, halfExtentY, halfExtentZ });
    world.set<Gravity>({ 9.8f });
    world.set<Damping>({ 0.98f });


    Systems systems(world);


    for (int i = 0; i < NB_OF_PARTICLES; i++) {
        auto cube = world.entity();
        float radius = GetRandomValue(5, 25);
        cube.set<Position>({ (float)GetRandomValue(-halfExtentX, halfExtentX), (float)GetRandomValue(-halfExtentY, halfExtentY), (float)GetRandomValue(-halfExtentZ, halfExtentZ) });
        cube.set<Matrix>({ MatrixTranslate(cube.get<Position>()->x, cube.get<Position>()->y, cube.get<Position>()->z)});
        cube.set<Velocity>({ (float)GetRandomValue(-250, 250), (float)GetRandomValue(-250, 250), (float)GetRandomValue(-250, 250) });
        cube.set<Damping>({ GetRandomValue(30, 70) / 100.f });
        cube.set<Size3D>({ 1,1,1 });
        cube.set<Color>(ColorFromNormalized({ GetRandomValue(0,100)/100.f,GetRandomValue(0,100) / 100.f,GetRandomValue(0,100) / 100.f,1}));
    }

    
    Camera camera = { 0 };
    camera.position = { 0, 0, -250.0f };    // Camera position
    camera.target = { 0.0f, 0.0f, 1.0f };              // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };                  // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                        // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                     // Camera projection type


    Shader shader = LoadShader("../resources/lighting_instancing.vert", "../resources/lighting.frag");
    // Get shader locations
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "instanceTransform");

    int ambientLoc = GetShaderLocation(shader, "ambient");
    float lightingVal[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    SetShaderValue(shader, ambientLoc, lightingVal, SHADER_UNIFORM_VEC4);

    CreateLight(LIGHT_DIRECTIONAL, { 50.0f, 50.0f, 0.0f }, Vector3Zero(), WHITE, shader);

    Material matInstances = LoadMaterialDefault();
    matInstances.shader = shader;
    matInstances.maps[MATERIAL_MAP_DIFFUSE].color = RED;

    // Iterate over the Position component column
    Material matDefault = LoadMaterialDefault();
    matDefault.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;

    int colorLoc = GetShaderLocation(shader, "cols");
    std::cout << "location " << GetShaderLocation(shader, "cols") << std::endl;
    float f[4] = { 1,0,0,1 };
    
    
    Color* colors = new Color[NB_OF_PARTICLES * sizeof(Color)];
    int i = 0;
    world.each([&i, &colors](Color& c) {
        i++;
        colors[i] = c;
        });
    Image image;
    image.height = 1;
    image.width = NB_OF_PARTICLES;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image.mipmaps = 1; 
    image.data = colors;

    Texture2D tex = LoadTextureFromImage(image);
    
    SetShaderValueTexture(shader, colorLoc, tex);
    SetShaderValue(shader, GetShaderLocation(shader, "tests"), f, SHADER_UNIFORM_VEC4);
    UnloadImage(image);


    Matrix* transforms = new Matrix[sizeof(Matrix) * NB_OF_PARTICLES];

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------

        auto q = world.query_builder<Matrix, Color>()
            .build();

        

        BeginDrawing();

            ClearBackground(RAYWHITE);

            // systems.GetDrawParticlesSystem().run();
            BeginMode3D(camera);
                

                const Matrix* pos = q.iter().first().get<Matrix>();
                memcpy(transforms, pos, q.count() * sizeof(Matrix));




                DrawMeshInstanced(instancedMesh, matInstances, transforms, q.count());
            EndMode3D();
            DrawTexture(tex, 0, 300, WHITE);
            DrawText(std::to_string(GetFPS()).c_str(), 20, 20, 20, BLACK);
            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
        
        // update systems
        world.progress(0.0167f);
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    delete[] transforms;
    return 0;
}