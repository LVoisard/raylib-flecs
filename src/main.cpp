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
    const int NB_OF_PARTICLES = 250000;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(100);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    flecs::world world;

    Mesh instancedMesh = GenMeshCube(3, 3, 3);

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

    world.import<flecs::stats>(); 
    world.set<flecs::Rest>({});

    Systems systems(world);

    auto particleSystemEntity = world.entity("ParticleSystemEntity");

    for (int i = 0; i < NB_OF_PARTICLES; i++) {
        auto cube = world.entity();
        float radius = GetRandomValue(5, 25);
        cube.set<Position>({ (float)GetRandomValue(-halfExtentX, halfExtentX), (float)GetRandomValue(-halfExtentY, halfExtentY), (float)GetRandomValue(-halfExtentZ, halfExtentZ) });
        cube.set<Velocity>({ (float)GetRandomValue(-250, 250), (float)GetRandomValue(-250, 250), (float)GetRandomValue(-250, 250) });
        float size = GetRandomValue(10, 50) / 10.0f;
        cube.set<Size3D>({ size, size, size });
        cube.set<Matrix>({ MatrixTranslate(cube.get<Position>()->x, cube.get<Position>()->y, cube.get<Position>()->z)});
        cube.set<Damping>({ GetRandomValue(30, 70) / 100.f });
        cube.set<Color>(ColorFromNormalized({ GetRandomValue(0,100)/100.f,GetRandomValue(0,100) / 100.f,GetRandomValue(0,100) / 100.f,1}));
        //cube.child_of(particleSystemEntity);
    }

    
    Camera camera = { 0 };
    camera.position = { 0, 100, -500.0f };    // Camera position
    camera.target = { 0.0f, 0.0f, 1.0f };              // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };                  // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                        // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                     // Camera projection type


    Shader shader = LoadShader("../resources/lighting_instancing.vert", "../resources/lighting.frag");
    // Get shader locations
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "instanceTransform");
    shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "matEmission");


    int ambientLoc = GetShaderLocation(shader, "ambient");
    float lightingVal[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    SetShaderValue(shader, ambientLoc, lightingVal, SHADER_UNIFORM_VEC4);

    CreateLight(LIGHT_DIRECTIONAL, { 0.0f , 150.0f, -150.0f }, Vector3Zero(), WHITE, shader);

    Material matInstances = LoadMaterialDefault();
    matInstances.shader = shader;
    matInstances.maps[MATERIAL_MAP_DIFFUSE].color = RED;

    // Iterate over the Position component column
    Material matDefault = LoadMaterialDefault();
    matDefault.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;

    int textureWidthLoc = GetShaderLocation(shader, "textureWidth");
    int textureHeightLoc = GetShaderLocation(shader, "textureHeight");
    
   
    

    int textureHeight = std::sqrt(NB_OF_PARTICLES);
    int textureWidth = NB_OF_PARTICLES / textureHeight;

    Image image;
    image.height = textureHeight;
    image.width = NB_OF_PARTICLES / textureHeight;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image.mipmaps = 1; 
    image.data = new Color[NB_OF_PARTICLES * sizeof(Color)];

    auto q1 = world.query_builder<Color>().build();    
    const Color* c = q1.first().get<Color>();
    memcpy(image.data, c, q1.count() * sizeof(Color));


    Texture2D tex = LoadTextureFromImage(image);
    UnloadImage(image);

    SetShaderValue(shader, textureWidthLoc, &textureWidth, SHADER_UNIFORM_INT);
    SetShaderValue(shader, textureWidthLoc, &textureHeight, SHADER_UNIFORM_INT);
    matInstances.maps[MATERIAL_MAP_EMISSION].texture = tex;


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

        

        

        BeginDrawing();

            ClearBackground(RAYWHITE);

            // systems.GetDrawParticlesSystem().run();
            BeginMode3D(camera);
                
                auto q = world.query_builder<Matrix, Color>()
                    .build();

                const Matrix* pos = q.first().get<Matrix>();
                memcpy(transforms, pos, q.count() * sizeof(Matrix));

                 DrawMeshInstanced(instancedMesh, matInstances, transforms, q.count());
            EndMode3D();


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