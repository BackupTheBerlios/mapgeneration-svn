/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef GPSDRAW_H
#define GPSDRAW_H

#include <wx/wx.h>


namespace mapgeneration_gui
{

	class GPSDraw
	{		
		public:
		
			GPSDraw();


			GPSDraw(wxDC* dc, double x_offset, double y_offset, double zoom_factor);
			
			
			void
			arrow(double latitude, double longitude, double direction, double length);
			
			
			void
			arrow(double latitude1, double longitude1, 
				double latitude2, double longitude2, double arrow_length);


			void
			circle(double latitude, double longitude, double radius);
			

			void 
			cross(double latitude, double longitude, int length);
			
			
			void
			gps_project(double latitude, double longitude, double& x, double& y);
			
			
			void
			gps_reproject(double x, double y, double& latitude, double& longitude);


			void
			line(double latitude1, double longitude1, double latitude2, double longitude2);
		
		
			void
			point(double latitude, double longitude);
			

			void
			project(double latitde, double longitude, int& x, int& y);
			
			
			void
			rectangle(double latitude1, double longitude1, double latitude2, double longitude2);
			
			
			void
			reproject(int x, int y, double& latitude, double& longitude);
			
			
			void
			set(wxDC* dc, double x_offset, double y_offset, double zoom_factor);
			
			
			void 
			text(double latitude, double longitude, const wxString& text);
			
			
			void
			xy_to_gps(int x, int y, double latitude, double longitude);
			
			
		private:
		
			wxDC* _dc;
			
			double _x_offset;
			double _y_offset;
			
			double _zoom_factor;
			
			
			inline double
			rezox(int x);
			
			
			inline double
			rezoy(int y);
			
			
			/**
			 * Short helper function to apply zoom and offset to the
			 * x value and return it as an integer.
			 */
			inline int
			zox(double x);
			
			
			/**
			 * Short helper function to apply zoom and offset to the
			 * y value and return it as an integer.
			 */
			inline int
			zoy(double y);
			
	};
	
	
	
	inline double
	GPSDraw::rezox(int x)
	{
		double new_x = ((double)x / _zoom_factor) - _x_offset;
		return (int)new_x;
	}
	
	
	inline double
	GPSDraw::rezoy(int y)
	{
		double new_y = ((double)y / _zoom_factor) - _y_offset;
		return (int)new_y;
	}
	
	
	inline int
	GPSDraw::zox(double x)
	{
		double new_x = (x + _x_offset) * _zoom_factor;
		return (int)new_x;
	}
	
	
	inline int
	GPSDraw::zoy(double y)
	{
		double new_y = (y + _y_offset) * _zoom_factor;
		return (int)new_y;
	}

} // namespace mapgeneration_gui


#endif //GPSDRAW_H
