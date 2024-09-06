#pragma once
#include "flecs.h"

class Systems {
public:
	Systems(flecs::world world);
	void InitSystems();

	void InitUpdatePositionSystem();
	void InitApplyGravitySystem();
	void InitApplyDampingSystem();
	void InitApplyBoundingBoxSystem();
	void InitDrawParticlesSystem();

	const flecs::system& GetDrawParticlesSystem() {
		return draw_particles_system;
	}

private:
	flecs::world world;
	flecs::system draw_particles_system;
};