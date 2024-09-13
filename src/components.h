#pragma once
#include "raylib.h"

// Singleton components
struct Gravity {
	float value = -9.8f;
};

struct Damping {
	float coefficient;
};

struct Game {
	int windowWidth;
	int windowHeight;
	float halfExtentX;
	float halfExtentY;
	float halfExtentZ;
};

struct Position {
	float x;
	float y;
	float z;
};

struct Velocity {
	float x;
	float y;
	float z;
};

struct Radius {
	float size;
};

struct Size3D {
	float sizeX;
	float sizeY;
	float sizeZ;
};

struct ParticleSystemComponent {
	Mesh mesh;
	Shader shader;
	Material instanceMaterial;
	int NB_OF_PARTICLES;
};

