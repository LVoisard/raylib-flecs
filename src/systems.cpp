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
	world.system<Position, Size3D, Matrix, const Velocity>("UpdatePosition")
		.cached()
		.multi_threaded()
		.each([](flecs::iter& it, size_t, Position& p, Size3D& s, Matrix& m, const Velocity& v) {
		float dx = v.x * it.delta_time();
		float dy = v.y * it.delta_time();
		float dz = v.z * it.delta_time();
		p.x += dx;
		p.y += dy;
		p.z += dz;
		m.m12 = p.x;
		m.m13 = p.y;
		m.m14 = p.z;
			});
}

void Systems::InitApplyGravitySystem()
{
	world.system<Velocity, const Gravity>("ApplyGravity")
		.cached()
		.multi_threaded()
		.term_at(1).singleton()
		.each([](flecs::iter& it, size_t, Velocity& v, const Gravity& g) {		
			v.y = v.y - (g.value * g.value * it.delta_time());
			});
}

void Systems::InitApplyDampingSystem()
{
	world.system<Velocity, const Damping>("ApplyDamping")
		.cached()
		.multi_threaded()
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
		.cached()
		.multi_threaded()
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
	particle_transforms_query = world.query_builder<Matrix>()
		.with(flecs::ChildOf, "ParticleSystemEntity")
		.build();

	draw_particles_system = world.system<ParticleSystemComponent>("DrawParticles")
		.kind(0)
		.multi_threaded()
		.each([this](ParticleSystemComponent& s) {
			
			DrawMeshInstanced(s.mesh, s.instanceMaterial, particle_transforms_query.first().get<Matrix>(), s.NB_OF_PARTICLES);
			});
}
