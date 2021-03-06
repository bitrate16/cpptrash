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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "RayTrace.h"
#include "lodepng.h"

using namespace spaint;
using namespace cppmath;
using namespace raytrace;

#define WIDTH 1000
#define HEIGHT 1000
#define SCALE 4

// bash c.sh "-lX11" example/raytrace_render

class tracer {
	
public:
	
	RayTrace rt;
	
	tracer() {
		rt.camera = Camera(WIDTH, HEIGHT);
		rt.set_background(Color::BLACK);
		//rt.get_scene().soft_shadows = 1;
		rt.get_scene().diffuse_light = 1;
		rt.get_scene().soft_shadows_scale = 0.5;
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
		glass_sphere->material.refract_val = 0.5;
		glass_sphere->material.reflect = 0.1;
		rt.get_scene().addObject(glass_sphere);
	};
	
	void encodeOneStep(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
		/*Encode the image*/
		unsigned error = lodepng_encode32_file(filename, image, width, height);

		/*if there's an error, display it*/
		if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
	};
	
	void render() {
		std::cout << "RENDERING\n";
		unsigned width = WIDTH, height = HEIGHT;
		unsigned int* frame = (unsigned int*) malloc(width * height * 4);
		
		for (int x = 0; x < rt.get_width(); ++x) {
			std::cout << (x + 1) << " / " << rt.get_width() << std::endl;
			for (int y = 0; y < rt.get_height(); ++y) {
				Color frag = rt.hitColorAt(x, y);
				frag.a = 255;
				frame[x + y * WIDTH] = frag.abgr();
			}
		}
			
		std::string outname = "output/aframe.png";
		
		struct stat st = {0};
		if (stat("output", &st) == -1) 
			mkdir("output", 0700);
		
		encodeOneStep(outname.c_str(), (unsigned char*)frame, width, height);
					
		std::cout << "DONE\n";
	};
};

int main() {
	
	tracer t;
	t.render();
	
	return 0;
};

