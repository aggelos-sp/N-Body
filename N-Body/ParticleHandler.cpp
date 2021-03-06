#include <random>
#include "Settings.h"
#include "ParticleHandler.h"
#include <tbb/concurrent_vector.h>
#include "lodepng.h"
#include "TreeParticle.h"
#include "QuadParticleTree.h"

// Generate random particles
void ParticleHandler::allocate_random_particles(size_t particle_count, std::vector<Particle>& particles, size_t size_x, size_t size_y) {
	if (particle_count > 0) {
		std::random_device random_device_;
		std::mt19937 mt_engine(random_device_());
		std::uniform_real_distribution<> real_position_x(0, static_cast<float>(size_x));
		std::uniform_real_distribution<> real_position_y(0, static_cast<float>(size_y));
		std::uniform_real_distribution<> real_mass(0, MAX_MASS);

		for (size_t i = 0; i < particle_count; ++i)
			particles.push_back(Particle(static_cast<float>(real_position_x(mt_engine)), static_cast<float>(real_position_y(mt_engine)),
				0.0f, 0.0f, static_cast<float>(real_mass(mt_engine)), 0.0f, 0.0f));
	}
}

std::vector<Particle> ParticleHandler::get_random_particles_Barns_Hut_sample() {
	// Example of 8 fixed points
		
	std::vector<Particle> particles; // ensure that the vector is clear

	Particle a = Particle(20.0f, 78.0f, 1.0f);
	Particle b = Particle(70.0f, 90.0f, 1.0f);
	Particle c = Particle(60.0f, 80.0f, 1.0f);
	Particle d = Particle(80.0f, 72.0f, 1.0f);
	Particle e = Particle(45.0f, 45.0f, 1.0f);
	Particle f = Particle(12.0f, 12.0f, 1.0f);
	Particle g = Particle(30.0f, 20.0f, 1.0f);
	Particle h = Particle(68.0f, 15.0f, 1.0f);

	particles.push_back(a);
	particles.push_back(b);
	particles.push_back(c);
	particles.push_back(d);
	particles.push_back(e);
	particles.push_back(f);
	particles.push_back(g);
	particles.push_back(h);	

	return particles;
}

// Generate an image from the given particle collection
void ParticleHandler::universe_to_png(const std::vector<Particle>& universe, size_t universe_size_x, size_t universe_size_y, const char* filename) {
	uint32_t width = static_cast<uint32_t>(universe_size_y);
	uint32_t height = static_cast<uint32_t>(universe_size_x);
	std::vector<uint8_t> image;
	image.resize(width * height * 4);

	// Paint all black
	for (uint32_t x = 0; x < height; x++) {
		for (uint32_t y = 0; y < width; y++) {
			image[4 * width * x + 4 * y + 0] = 0;
			image[4 * width * x + 4 * y + 1] = 0;
			image[4 * width * x + 4 * y + 2] = 0;
			image[4 * width * x + 4 * y + 3] = 255;
		}
	}

	// Paint red all the particles
	for (const Particle& current_particle : universe) {
		image[4 * width * static_cast<uint32_t>(current_particle.x_) + 4 * static_cast<uint32_t>(current_particle.y_) + 0] = 255; // red
	}

	// Do save to png
	lodepng::encode(filename, image, width, height);
}

// Convert a vector particle collection into a concurrent vector collection
tbb::concurrent_vector<Particle> ParticleHandler::to_concurrent_vector(const std::vector<Particle>& input_particles) {
	tbb::concurrent_vector<Particle, tbb::cache_aligned_allocator<Particle>> returning_concurrent_vector;

	for (const Particle& current_particle : input_particles)
		returning_concurrent_vector.push_back(current_particle);
	return returning_concurrent_vector;
}

// Convert a concurrent vector particle collection into a vector collection
std::vector<Particle> ParticleHandler::to_vector(const tbb::concurrent_vector<Particle>& input_particles) {
	std::vector<Particle> returning_vector;

	for (const Particle& current_particle : input_particles)
		returning_vector.push_back(current_particle);

	return returning_vector;
}

// Check if two particle collections contain exactly the same particles in terms of location, velocity, mass and acceleration
bool ParticleHandler::are_equal(const std::vector<Particle>& first_particles, const std::vector<Particle>& second_particles) {
	
	bool are_equal = false;

	if (first_particles.size() == second_particles.size()) {
		are_equal = true;
		for (size_t i = 0; i < first_particles.size(); ++i) {
			// TODO: comparision should be within a floating point limit, not exact...
			if (first_particles[i].velocity_x_ != second_particles[i].velocity_x_ || first_particles[i].velocity_y_ != second_particles[i].velocity_y_ ||
				first_particles[i].x_ != second_particles[i].x_ || first_particles[i].y_ != second_particles[i].y_ ||
				first_particles[i].acceleration_x_ != second_particles[i].acceleration_x_ ||
				first_particles[i].acceleration_y_ != second_particles[i].acceleration_y_ || first_particles[i].mass_ != second_particles[i].mass_) {

				are_equal = false;
				break;
			}
		}
	}	
	return are_equal;
}

QuadParticleTree* ParticleHandler::to_quad_tree(const std::vector<Particle>& input_particles, size_t size_x, size_t size_y) {
	
	// Crate a new quad tree with limits from zero, up to grid size x and y
	QuadParticleTree *quad_particle_tree = new QuadParticleTree(Particle(0.0f, 0.0f, 0.0f),
		Particle(static_cast<float>(size_x), static_cast<float>(size_y), 0.0f));

	// Insert the points in the quad tree
	TreeParticle *quad_tree_particles = new TreeParticle[input_particles.size()];
	for (size_t i = 0; i < input_particles.size(); ++i) {
		quad_tree_particles[i].set_particle(input_particles[i]);
		quad_particle_tree->insert(quad_tree_particles + i);
	}

	return quad_particle_tree;
}