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

#include <cmath>
#include <vector>

#include "spaint.h"
#include "spaint_util.h"
#include "cppmath.h"
#include "vec2.h"
#include "ivec2.h"
#include "delaunay.h"

using namespace std;
using namespace spaint;
using namespace cppmath;

#define KEY_ESCAPE  9
#define KEY_R      27
#define KEY_L      46
#define KEY_C      54
#define KEY_I      31

// Bezier by points visualisation using spaint tool.

// bash c.sh "-lX11" example/delaunay_manual

// rendering

class scene : public component {
	
	void create() {
		get_paint().init_font();
		get_window().set_title("Delaunay example");
	};
	
	vector<vec2> points;
	
	delaunay::delaunay dln;
	
	ivec2 last_pointer;
	
	// Points drag
	bool mouse1_down = 0;
	bool mouse2_down = 0;
	bool drag1_mode = 0;
	int  drag1_id = 0;
	
	bool resized = 1;
	bool updated = 0;
	
	// Render options
	bool render_lines = 1;   // L
	bool render_circles = 1; // C
	bool render_indices = 1; // I
	
	void resize() {
		resized = 1;
	};
	
	// Returns triangle cointaining passed point.
	int get_triangle(const ivec2& v) {
		if (points.size() < 3)
			return -1;
		
		for (int i = 0; i < dln.triangles.size(); ++i) 
			if (math::in_triangle(v, dln.triangles[i].p0, dln.triangles[i].p1, dln.triangles[i].p2)) 
				return i;
					
		return -1;
	};
	
	void loop() {
		window& w = get_window();
		painter& p = w.get_paint();
		
		// Block untill event is reached
		if (!mouse1_down && !mouse2_down) w.wait_event(1);
		
		if (w.has_key_event(0))
			if (w.get_key_down() == KEY_ESCAPE)
				w.stop();
			else if (w.get_key_down() == KEY_R) {
				points.clear();
				updated = 1;
			} else if (w.get_key_down() == KEY_L) {
				render_lines = !render_lines;
				updated = 1;
			} else if (w.get_key_down() == KEY_C) {
				render_circles = !render_circles;
				updated = 1;
			} else if (w.get_key_down() == KEY_I) {
				render_indices = !render_indices;
				updated = 1;
			}
			
		if (w.has_mouse_event(0)) {
			if (w.get_button_down() == Button1) 
				mouse1_down = 1;
			else if (w.get_button_up() == Button1) {
				mouse1_down = 0;
				drag1_mode = 0;
			}
			if (w.get_button_down() == Button3) 
				mouse2_down = 1;
			else if (w.get_button_up() == Button3) 
				mouse2_down = 0;
		}
			
		w.clear_events();
		
		// Left button operations
		if (!drag1_mode && mouse1_down) {

			window::pointer point = w.get_pointer();
			vec2 mp(point.x, point.y);
			
			// Match dragged point
			for (int i = 0; i < points.size(); ++i) 
				if ((points[i] - mp).len2() < 100) {
					drag1_mode = 1;
					drag1_id = i;
					return;
				}
			
			// Else add point
			points.push_back(vec2(point.x, point.y));
			dln = delaunay::triangulate(points);
			
			mouse1_down = 0;
			updated = 1;
		} else if (drag1_mode) {
			// Move selected point
			window::pointer point = w.get_pointer();
			points[drag1_id].x = point.x;
			points[drag1_id].y = point.y;
			dln = delaunay::triangulate(points);
			
			updated = 1;
		}
		
		if (mouse2_down)
			updated = 1;
			
		if (resized || updated) {
			window::pointer point = w.get_pointer();			
			p.clear();
			
			last_pointer.x = point.x;
			last_pointer.y = point.y;
			
			p.color(0, 255, 0);
			p.arc(point.x-10, point.y-10, 20, 20);
			
			if (points.size() > 1) {
				if (mouse2_down) {
					int matched_triangle = get_triangle(last_pointer);
					if (matched_triangle != -1) {
						
						p.color(255, 0, 255);
						delaunay::triangle& tr = dln.triangles[matched_triangle];
						
						XPoint verts[4] = {
											Point(tr.p0.x, tr.p0.y), 
											Point(tr.p1.x, tr.p1.y), 
											Point(tr.p2.x, tr.p2.y),
											Point(tr.p0.x, tr.p0.y)
										};
						
						p.fill_poly(verts, 4);
						
						string ind = to_string(matched_triangle);
						p.text(16, 16, ind.c_str());
					}
				}
			
				p.color(255, 255, 255);
				
				// Render frame lines
				if (render_lines) {
					for (int i = 0; i < dln.edges.size(); ++i) 
						p.line(dln.edges[i].p0.x, dln.edges[i].p0.y, dln.edges[i].p1.x, dln.edges[i].p1.y);
				}
				
				p.color(255, 0, 255);
				
				// Render anchor points
				if (render_circles)
					for (int i = 0; i < points.size(); ++i)
						p.arc(points[i].x-10, points[i].y-10, 20, 20);
				
			}
			
			p.color(0, 255, 255);
			
			// Render frame lines
			if (render_indices)
				for (int i = 0; i < points.size(); ++i) {
					p.color(0, 255, 255);
					p.point(points[i].x, points[i].y);
					p.color(255, 255, 0);
					string ind = to_string(i);
					p.text(points[i].x, points[i].y, ind.c_str());
				}
			
			resized = 0;
			updated = 0;
		}
	};
};


int main() {
	scene s;
	window w(&s, 200, 200, 0);
	w.start();
	return 0;
};

