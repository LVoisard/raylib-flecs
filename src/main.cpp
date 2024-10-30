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

const int screenWidth = 1920;
const int screenHeight = 1080;
const int halfExtentX = 100;
const int halfExtentY = 50;
const int halfExtentZ = 100;
const int NB_OF_PARTICLES = 600000;

flecs::entity createParticleSystem(flecs::world& world)
{
    auto particleSystemEntity = world.entity("ParticleSystemEntity");

    for (int i = 0; i < NB_OF_PARTICLES; i++) {
        auto cube = world.entity();
        float radius = GetRandomValue(5, 25);
        cube.set<Position>({ (float)GetRandomValue(-halfExtentX, halfExtentX), (float)GetRandomValue(-halfExtentY, halfExtentY), (float)GetRandomValue(-halfExtentZ, halfExtentZ) });
        cube.set<Velocity>({ (float)GetRandomValue(-250, 250), (float)GetRandomValue(-250, 250), (float)GetRandomValue(-250, 250) });
        float size = GetRandomValue(10, 50) / 10.0f;
        cube.set<Size3D>({ size, size, size });
        cube.set<Matrix>({ MatrixMultiply(MatrixTranslate(cube.get<Position>()->x, cube.get<Position>()->y, cube.get<Position>()->z), MatrixScale(size, size, size)) });
        cube.set<Damping>({ GetRandomValue(30, 70) / 100.f });
        cube.set<Color>(ColorFromNormalized({ GetRandomValue(0,100) / 100.f,GetRandomValue(0,100) / 100.f,GetRandomValue(0,100) / 100.f,1 }));
        cube.child_of(particleSystemEntity);
    }

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

    Shader shader = LoadShader("../resources/lighting_instancing.vert", "../resources/lighting.frag");
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "instanceTransform");
    shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "matEmission");

    int textureWidthLoc = GetShaderLocation(shader, "textureWidth");
    int textureHeightLoc = GetShaderLocation(shader, "textureHeight");

    SetShaderValue(shader, textureWidthLoc, &textureWidth, SHADER_UNIFORM_INT);
    SetShaderValue(shader, textureWidthLoc, &textureHeight, SHADER_UNIFORM_INT);

    Material mat = LoadMaterialDefault();
    mat.shader = shader;
    mat.maps[MATERIAL_MAP_EMISSION].texture = tex;

    
    particleSystemEntity.set<ParticleSystemComponent>({ GenMeshCube(1, 1, 1), shader, mat, NB_OF_PARTICLES });

    int ambientLoc = GetShaderLocation(shader, "ambient");
    float lightingVal[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    SetShaderValue(shader, ambientLoc, lightingVal, SHADER_UNIFORM_VEC4);

    CreateLight(LIGHT_DIRECTIONAL, { 0.0f , 150.0f, -150.0f }, Vector3Zero(), WHITE, shader);

    return particleSystemEntity;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
   

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(100);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    flecs::world world;

    world.component<Position>();
    world.component<Matrix>();
    world.component<Velocity>();
    world.component<Damping>();
    world.component<Radius>();
    world.component<Color>();
    world.component<Size3D>();

    world.set<Game>({ screenWidth, screenHeight, halfExtentX, halfExtentY, halfExtentZ });
    world.set<Gravity>({ 9.8f });
    world.set<Damping>({ 0.98f });

    world.import<flecs::stats>(); 
    world.set<flecs::Rest>({});
    
    
    flecs::entity particleSystem = createParticleSystem(world);
    Systems systems(world);

    // Get shader locations    
    Camera camera = { 0 };
    camera.position = { 0, 25, -250.0f };    // Camera position
    camera.target = { 0.0f, 0.0f, 1.0f };              // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };                  // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                        // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                     // Camera projection type

    


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

            BeginMode3D(camera);
                
            systems.GetDrawParticlesSystem().run();

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

    return 0;
}
