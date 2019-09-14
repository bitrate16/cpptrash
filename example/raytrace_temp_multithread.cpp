/*
    Example shows use of spaint and cppmath::math::right_turn
	
	cpp math utilities
    Copyright (C) 2019-3041  bitrate16

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <string>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <thread>
#include <condition_variable>
#include <mutex>

#include "RayTrace.h"
#include "lodepng.h"

using namespace spaint;
using namespace cppmath;
using namespace raytrace;

#define FILENAME "output/aframe.png"

#define THREAD_COUNT 4

#define WIDTH 10000
#define HEIGHT 10000
#define SCALE 40

// bash c.sh "-lpthread" example/raytrace_temp_multithread

class tracer;

void worker_function(tracer* t, int thread_id);

class tracer {
	
public:
	
	// 1. All threads waits for main to pause on start
	// 2. Main waits all threads to stop after work
	std::mutex waiter;
	std::condition_variable cv;
	
	// Protect coords
	std::mutex index_access;
	int x = 0;
	int y = 0;
	
	bool written = 0;
	
	// Array of threads
	std::thread* threads[THREAD_COUNT];
	
	// Output array
	unsigned int* frame;
	
	// Number of finished threads
	int finshed_threads = 0;
	
	RayTrace rt;
	
	tracer() {
		
		// INIT THREADS
		
		for (int i = 0; i < THREAD_COUNT; ++i)
			threads[i] = new std::thread(worker_function, this, i);
		
		create_scene();
	};
	
	void create_scene() {
		// INIT SCENE
		
		rt.camera = Camera(WIDTH, HEIGHT);
		rt.set_background(Color::BLACK);
		rt.get_scene().soft_shadows = 1;
		//rt.get_scene().diffuse_light = 1;
		rt.get_scene().soft_shadows_scale = 0.5;
		rt.get_scene().random_diffuse_ray = 1;
		rt.get_scene().random_diffuse_count = 8;
		rt.get_scene().MAX_RAY_DEPTH = 4;
		
		// Surrounding
		Plane* red_plane = new Plane(vec3(0, -50, 0) * SCALE, vec3(0, 1, 0));
		red_plane->material.color = Color::WHITE;
		red_plane->material.diffuse = 1.0;
		rt.get_scene().addObject(red_plane);
		
		Plane* green_plane = new Plane(vec3(-50, 0, 0) * SCALE, vec3(1, 0, 0));
		green_plane->material.color = Color::BLUE;
		green_plane->material.diffuse = 1.0;
		rt.get_scene().addObject(green_plane);
		
		Plane* blue_plane = new Plane(vec3(50, 0, 0) * SCALE, vec3(-1, 0, 0));
		blue_plane->material.color = Color::RED;
		blue_plane->material.diffuse = 1.0;
		rt.get_scene().addObject(blue_plane);
		
		Plane* cyan_plane = new Plane(vec3(0, 0, 150) * SCALE, vec3(0, 0, -1));
		cyan_plane->material.color = Color::WHITE;
		cyan_plane->material.diffuse = 1.0;
		rt.get_scene().addObject(cyan_plane);
		
		Plane* yellow_plane = new Plane(vec3(0, 50, 0) * SCALE, vec3(0, -1, 0));
		yellow_plane->material.color = Color::WHITE;
		yellow_plane->material.diffuse = 1.0;
		rt.get_scene().addObject(yellow_plane);
		
		Plane* magenta_plane = new Plane(vec3(0, 0, -50) * SCALE, vec3(0, 0, 1));
		magenta_plane->material.color = Color::WHITE;
		magenta_plane->material.diffuse = 1.0;
		rt.get_scene().addObject(magenta_plane);
		
		// Spheres
		Sphere* red_sphere = new Sphere(vec3(20, 20, 120) * SCALE, 5 * SCALE);
		red_sphere->material.color = Color::RED;
		red_sphere->material.luminosity = 0.0;
		rt.get_scene().addObject(red_sphere);
		
		Sphere* green_sphere = new Sphere(vec3(15, -15, 100) * SCALE, 5 * SCALE);
		green_sphere->material.color = Color::GREEN;
		green_sphere->material.luminosity = 0.0;
		rt.get_scene().addObject(green_sphere);
		
		Sphere* light_sphere = new Sphere(vec3(0, 20, 80) * SCALE, 5 * SCALE);
		light_sphere->material.color = Color::WHITE;
		light_sphere->material.luminosity = 1.0;
		light_sphere->material.surface_visible = 0;
		rt.get_scene().addObject(light_sphere);
		
		Sphere* white_sphere = new Sphere(vec3(10, 0, 100) * SCALE, 10 * SCALE);
		white_sphere->material.color = Color::WHITE;
		white_sphere->material.reflect = 1.0;
		rt.get_scene().addObject(white_sphere);
		
		Sphere* glass_sphere = new Sphere(vec3(-5, -5, 50) * SCALE, 10 * SCALE);
		glass_sphere->material.color = Color::WHITE;
		glass_sphere->material.refract = 0.9;
		glass_sphere->material.refract_val = 3.3;
		glass_sphere->material.reflect = 0.1;
		rt.get_scene().addObject(glass_sphere);
	
		UVSphere* uv_sphere = new UVSphere(vec3(30, 20, 90) * SCALE, 10 * SCALE);
		uv_sphere->material.color = Color::WHITE;
		uv_sphere->material.luminosity = 0.0;
		uv_sphere->material.diffuse = 1.0;
		uv_sphere->uv_map = [](double u, double v) -> Color {
			u += 1;
			int iu = u / 3.14159265358979323846 * 2;
			int iv = v / 3.14159265358979323846 * 2;
			
			if (!(iu % 2 == 0) && (iv % 2 == 0) || (iu % 2 == 0) && !(iv % 2 == 0))
				return Color(255, 0, 255);
			return Color();
		};
		rt.get_scene().addObject(uv_sphere);
	};
	
	~tracer() {
		for (int i = 0; i < THREAD_COUNT; ++i)
			delete threads[i];
	};
	
	void encodeOneStep(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
		/*Encode the image*/
		unsigned error = lodepng_encode32_file(filename, image, width, height);

		/*if there's an error, display it*/
		if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
	};
	
	void render() {
		std::cout << "RENDERING\n";
		
		frame = (unsigned int*) malloc(WIDTH * HEIGHT * 4);
		
		/*std::unique_lock<std::mutex> locker(waiter);
		cv.notify_all();
		cv.wait(locker);*/
		
		cv.notify_all();
		
		for (int i = 0; i < THREAD_COUNT; ++i)
			threads[i]->join();
	};
};

void worker_function(tracer* t, int thread_id) {
	std::cout << "Thread " << thread_id << " created" << std::endl;
	
	// Wait for main thread to start tracer
	{ 
		std::unique_lock<std::mutex> locker(t->waiter);
		t->cv.wait(locker);
		std::cout << "Thread " << thread_id << " started" << std::endl;
	}
	
	// Do rendering
	while (1) {
		int x, y;
		{
			std::unique_lock<std::mutex> locker(t->index_access);
			if (t->x == t->rt.get_width() - 1) {
				t->x = 0;
				++t->y;
				
				std::cout << (t->y + 1) << " / " << t->rt.get_width() << std::endl;
				// std::cout << "Thread " << thread_id << std::endl;
			}
			if (t->y >= t->rt.get_height()) {
				++t->finshed_threads;
				
				if (!t->written) {
					t->written = 1;
					std::cout << "DONE\n";
					
					t->encodeOneStep(FILENAME, (unsigned char*) t->frame, WIDTH, HEIGHT);
								
					std::cout << "WRITTEN\n";
					
					exit(0);
				}
				
				// Wake main after all threads finished
				/*if (t->finshed_threads == THREAD_COUNT) {
		
					std::cout << "DONE\n";
					
					t->encodeOneStep(FILENAME, (unsigned char*) t->frame, WIDTH, HEIGHT);
								
					std::cout << "WRITTEN\n";
					
					t->cv.notify_all();
				}*/
				
				std::cout << "Thread " << thread_id << " stopped" << std::endl;
				return;
			}
			
			x = t->x;
			y = t->y;
			++t->x;
			
			locker.unlock();
		}
		
		// std::cout << "Thread " << thread_id << " --> (" << x << ", " << y << ")" << std::endl;
		
		Color frag = t->rt.hitColorAt(x, y);
		frag.a = 255;
		t->frame[x + y * WIDTH] = frag.abgr();
	}
	
	
};

int main() {
	
	tracer t;
	t.render();
	
	return 0;
};

