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

#include "spaint.h"
#include "RayTrace.h"

using namespace spaint;
using namespace cppmath;
using namespace raytrace;

#define KEY_ESCAPE 9
#define KEY_R      27

// bash c.sh "-lX11" example/raytrace


class scene : public component {
	
	void create() {
		get_paint().init_font();
		get_window().set_title("RayTrace example");
		
		updated = 1;
		rt.camera = Camera(250, 250);
		rt.set_background(Color::BLACK);
		
		Sphere* red_sphere = new Sphere(vec3(20, 20, 100), 5);
		red_sphere->material.color = Color::RED;
		red_sphere->material.luminosity = 0.0;
		rt.get_scene().addObject(red_sphere);
		
		Sphere* green_sphere = new Sphere(vec3(20, -20, 100), 5);
		green_sphere->material.color = Color::GREEN;
		green_sphere->material.luminosity = 0.0;
		rt.get_scene().addObject(green_sphere);
		
		Sphere* light_sphere = new Sphere(vec3(0, 20, 80), 5);
		light_sphere->material.color = Color::WHITE;
		light_sphere->material.luminosity = 1.0;
		rt.get_scene().addObject(light_sphere);
		
		Sphere* white_sphere = new Sphere(vec3(0, 0, 100), 10);
		white_sphere->material.color = Color::WHITE;
		white_sphere->material.reflect = 1.0;
		rt.get_scene().addObject(white_sphere);
	};
	
	void destroy() {
		
	};
	
	bool mouse_down = 0;
	bool resized = 0;
	bool updated = 0;
	
	RayTrace rt;
	
	void resize() {
		resized = 1;
	};
	
	void loop() {
		window& w = get_window();
		painter& p = w.get_paint();
		
		// Block untill event is reached
		if (!mouse_down) w.wait_event(1);
		
		if (w.has_key_event(0))
			if (w.get_key_down() == KEY_ESCAPE)
				w.stop();
			if (w.get_key_down() == KEY_R) 
				updated = 1;
		
		if (w.has_mouse_event(0)) 
			if (w.get_button_down() == Button1) 
				mouse_down = 1;
			else if (w.get_button_up() == Button1) 
				mouse_down = 0;
			
		w.clear_events();
			
		if (resized || updated || mouse_down) {
			
			resized = 0;
			updated = 0;
			
			for (int x = 0; x < rt.get_width(); ++x)
				for (int y = 0; y < rt.get_height(); ++y) {
					Color frag = rt.hitColorAt(x, y);
					if (frag != Color::BLACK) {
						p.color(frag);
						p.point(x, y);
					}
				}
				std::cout << "DONE\n";
			
		} else {
			// ...
		}
	};
};


int main() {
	scene s;
	window w(&s, 250, 250, 0);
	w.start();
	return 0;
};
