/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "gpsdraw.h"

#include <cmath>
#include "gpstrans/gpsproj.h"


namespace mapgeneration_gui
{
	
	GPSDraw::GPSDraw()
	: _dc(0), _x_offset(0), _y_offset(0), _zoom_factor(1)
	{
	}
	
	
	GPSDraw::GPSDraw(wxDC* dc, double x_offset, double y_offset, double zoom_factor)
	: _dc(dc), _x_offset(x_offset), _y_offset(y_offset), _zoom_factor(zoom_factor)
	{
	}
	
	
	void
	GPSDraw::arrow(double latitude, double longitude, double direction, double length)
	{
		const double PI = 3.141592;
		double x, y;
		gps_project(latitude, longitude, x, y);
		
		double latitude_end = latitude + cos(direction) * length;
		double longitude_end = longitude + sin(direction) * length;
		double x_end, y_end;
		gps_project(latitude_end, longitude_end, x_end, y_end);
		
		double latitude_left = latitude + cos(direction - PI/8) * length * 0.8;
		double longitude_left = longitude + sin(direction - PI/8) * length * 0.8;
		double x_left, y_left;
		gps_project(latitude_left, longitude_left, x_left, y_left);
		
		double latitude_right = latitude + cos(direction + PI/8) * length * 0.8;
		double longitude_right = longitude + sin(direction + PI/8) * length * 0.8;
		double x_right, y_right;
		gps_project(latitude_right, longitude_right, x_right, y_right);
		
		_dc->DrawLine(zox(x), zoy(y), zox(x_end), zoy(y_end));
		_dc->DrawLine(zox(x_end), zoy(y_end), zox(x_left), zoy(y_left));
		_dc->DrawLine(zox(x_end), zoy(y_end), zox(x_right), zoy(y_right));
	}
	
	
	void
	GPSDraw::arrow(double latitude1, double longitude1,
		double latitude2, double longitude2, double arrow_length)
	{
		const double PI = 3.141592;
		double x1, y1, x2, y2;
		gps_project(latitude1, longitude1, x1, y1);
		gps_project(latitude2, longitude2, x2, y2);
		
		double slopy , cosy , siny;
		slopy = atan2( y1-y2, x1-x2 );
		cosy = cos( slopy );
		siny = sin( slopy ); //need math.h for these functions
 
		_dc->DrawLine(zox(x1), -zoy(y1), zox(x2), -zoy(y2));
	
		_dc->DrawLine(zox(x2), -zoy(y2), zox(x2 + arrow_length * cosy - ( arrow_length / 2.0 * siny )),
			-zoy(y2 + arrow_length * siny + ( arrow_length / 2.0 * cosy )));
		_dc->DrawLine(zox(x2), -zoy(y2), zox(x2 + arrow_length * cosy + arrow_length / 2.0 * siny ),
			-zoy(y2 - (arrow_length / 2.0 * cosy - arrow_length * siny )));
	}
	
	
	void
	GPSDraw::circle(double latitude, double longitude, double radius)
	{
		const double PI = 3.141592;
		double x, y;
		gps_project(latitude, longitude, x, y);
		
		double rad = 0;
		double previous_x = x + cos(rad) * radius;
		double previous_y = y + sin(rad) * radius;
		
		double current_x, current_y;
		for (; rad < 2*PI; rad += (2*PI) / 36)
		{
			current_x = x + cos(rad) * radius;
			current_y = y + sin(rad) * radius;
			_dc->DrawLine(zox(previous_x), zoy(previous_y), zox(current_x), zoy(current_y));
			previous_x = current_x;
			previous_y = current_y;
		}
	}

	
	void
	GPSDraw::cross(double latitude, double longitude, int length)
	{
		double x;
		double y;
		gps_project(latitude, longitude, x, y);

		_dc->DrawLine(zox(x) - length, zoy(y) - length, zox(x) + length, zoy(y) + length);
		_dc->DrawLine(zox(x) - length, zoy(y) + length, zox(x) + length, zoy(y) - length);
	}
	
	
	void
	GPSDraw::gps_project(double latitude, double longitude, double& x, double& y)
	{
		GPS_Math_Mercator_LatLon_To_EN(latitude, longitude, &x,
			&y, 0, 0,
			0, 0, 6378137.0, 6356752.314);
	}
	
	
	void
	GPSDraw::gps_reproject(double x, double y, double& latitude, double& longitude)
	{
		GPS_Math_Mercator_EN_To_LatLon(x, y, &latitude,
			&longitude, 0,
			0, 0, 0,
			6378137.0, 6356752.314);
	}


	void
	GPSDraw::line(double latitude1, double longitude1, double latitude2, double longitude2)
	{
		int x1, y1, x2, y2;
		project(latitude1, longitude1, x1, y1);
		project(latitude2, longitude2, x2, y2);

		_dc->DrawLine(x1, y1, x2, y2);
	}


	void
	GPSDraw::point(double latitude, double longitude)
	{
		double x;
		double y;		
		gps_project(latitude, longitude, x, y);

		_dc->DrawPoint((int)((x + _x_offset) * _zoom_factor), (int)((y + _y_offset) * _zoom_factor));
	}
	
	
	void
	GPSDraw::project(double latitude, double longitude, int& x, int& y)
	{
		double new_x, new_y;
		GPS_Math_Mercator_LatLon_To_EN(latitude, longitude, &new_x,
			&new_y, 0, 0,
			0, 0, 6378137.0, 6356752.314);

		x = zox(new_x);
		y = -zoy(new_y);
	}
	
	
	void
	GPSDraw::reproject(int x, int y, double& latitude, double& longitude)
	{
		GPS_Math_Mercator_EN_To_LatLon(rezox(x), rezoy(-y),
			&latitude, &longitude, 0,
			0, 0, 0,
			6378137.0, 6356752.314);
	}
	
	
	void
	GPSDraw::rectangle(double latitude1, double longitude1, double latitude2, double longitude2)
	{
		int x1, y1, x2, y2;
		project(latitude1, longitude1, x1, y1);
		project(latitude2, longitude2, x2, y2);

		_dc->DrawLine(x1, y1, x2, y1);
		_dc->DrawLine(x2, y1, x2, y2);
		_dc->DrawLine(x2, y2, x1, y2);
		_dc->DrawLine(x1, y2, x1, y1);
	}
	
	
	void
	GPSDraw::set(wxDC* dc, double x_offset, double y_offset, double zoom_factor)
	{
		_dc = dc;
		_x_offset = x_offset;
		_y_offset = y_offset;
		_zoom_factor = zoom_factor;
	}
	

	void
	GPSDraw::text(double latitude, double longitude, const wxString& text)
	{
		double x, y;
		gps_project(latitude, longitude, x, y);
		
		int font_size = (int)(2 * _zoom_factor);
		if (font_size < 6) font_size = 0;

		_dc->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxNORMAL));
		
		_dc->DrawText(text, zox(x), zoy(y));
	}
	

}
