#include "systems.h"
#include "components.h"
#include "raylib.h"
#include "raymath.h"

#include <iostream>
#include <string>
#include <algorithm>

Systems::Systems(flecs::world world)
{
	this->world = world;
	this->InitSystems();

}

void Systems::InitSystems()
{
	InitUpdatePositionSystem();
	InitApplyGravitySystem();
	InitApplyDampingSystem();
	InitApplyBoundingBoxSystem();
	InitDrawParticlesSystem();
}

void Systems::InitUpdatePositionSystem()
{
	world.system<Position, Matrix, const Velocity>("UpdatePosition")
		.each([](flecs::iter& it, size_t, Position& p, Matrix& m, const Velocity& v) {
		p.x += v.x * it.delta_time();
		p.y += v.y * it.delta_time();
		p.z += v.z * it.delta_time();
		m = MatrixTranslate(p.x, p.y, p.z);		
			});
}

void Systems::InitApplyGravitySystem()
{
	world.system<Velocity, const Gravity>("ApplyGravity")
		.term_at(1).singleton()
		.each([](flecs::iter& it, size_t, Velocity& v, const Gravity& g) {		
			v.y = v.y - (g.value * g.value * it.delta_time());
			});
}

void Systems::InitApplyDampingSystem()
{
	world.system<Velocity, const Damping>("ApplyDamping")
		.term_at(1).singleton()
		.each([](flecs::iter& it, size_t, Velocity& v, const Damping& d) {
			v.x *= d.coefficient;
			v.y *= d.coefficient;
			v.z *= d.coefficient;
		});
}

void Systems::InitApplyBoundingBoxSystem()
{
	world.system<Position, Velocity, Damping, Size3D, const Game>("ApplyBoundingBox")
		.term_at(4).singleton()
		.each([](Position& p, Velocity& v, const Damping& d, const Size3D& s, const Game& g) {
		if (p.x + s.sizeX > g.halfExtentX || p.x - s.sizeX < -g.halfExtentX) {
			v.x = -v.x * d.coefficient;
			p.x = std::clamp(p.x, -g.halfExtentX + 0.01f + s.sizeX, (float)g.halfExtentX - 0.01f - s.sizeX);
		}
		if (p.y + s.sizeY > g.halfExtentY || p.y - s.sizeY < -g.halfExtentY) {
			v.y = -v.y * d.coefficient;
			p.y = std::clamp(p.y, -g.halfExtentY + 0.01f + s.sizeY, (float)g.halfExtentY - 0.01f - s.sizeY);
		}
		if (p.z + s.sizeZ > g.halfExtentZ || p.z - s.sizeZ < -g.halfExtentZ) {
			v.z = -v.z * d.coefficient;
			p.z = std::clamp(p.z, -g.halfExtentZ + 0.01f + s.sizeZ, (float)g.halfExtentZ - 0.01f - s.sizeZ);
		}
			});
}

// need to manually call this when we draw
void Systems::InitDrawParticlesSystem()
{
	draw_particles_system = world.system<Position, Color>("DrawParticles")
		.kind(0)
		.each([](Position& p, Color c) {
			DrawCircle(p.x, p.y, 5, c);
			});
}
