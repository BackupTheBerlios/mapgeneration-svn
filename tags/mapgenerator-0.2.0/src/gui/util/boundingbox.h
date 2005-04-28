/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <cassert>
#include <wx/wx.h>


namespace mapgeneration_gui
{

	template <typename T_>
	class BoundingBox
	{
		public:

			inline
			BoundingBox();


			inline
			BoundingBox(const T_& x_min, const T_& y_min, const T_& x_max, const T_& y_max);
			
			
			inline void
			extend(const BoundingBox<T_>& bb);
			
			
			inline void
			extend(const T_& value);


			inline void
			extend(const T_& x, const T_& y);
			

			inline void
			extend(const T_& x_min, const T_& y_min, const T_& x_max, const T_& y_max);
			
			
			inline void
			get(T_& x_min, T_& y_min, T_& x_max, T_& y_max);
			
			
			inline bool
			initialized();
			
			
			inline void
			set(const T_& x, const T_& y);
			
			
			inline void
			set(const T_& x_min, const T_& y_min, const T_& x_max, const T_& y_max);
			
			
			inline T_&
			x_min()
			{
				return _x_min;
			}
			
			
			inline const T_&
			x_min() const
			{
				return _x_min;
			}


			inline T_&
			y_min()
			{
				return _y_min;
			}
			
			
			inline const T_&
			y_min() const
			{
				return _y_min;
			}
			
			
			inline T_&
			x_max()
			{
				return _x_max;
			}
			
			
			inline const T_&
			x_max() const
			{
				return _x_max;
			}
			
			
			inline T_&
			y_max()
			{
				return _y_max;
			}
			
			
			inline const T_&
			y_max() const
			{
				return _y_max;
			}


		private:
		
			T_ _x_min;
			T_ _y_min;
			T_ _x_max;
			T_ _y_max;
			
			bool _initialized;


	};
	
	
	template <typename T_>
	inline
	BoundingBox<T_>::BoundingBox()
	: _initialized(false)
	{
	}
	
	
	template <typename T_>
	inline 
	BoundingBox<T_>::BoundingBox(const T_& x_min, const T_& y_min, const T_& x_max, const T_& y_max)
	: _x_min(x_min), _y_min(y_min), _x_max(x_max), _y_max(y_max), _initialized(true)
	{
		assert(_x_min <= _x_max);
		assert(_y_min <= _y_max);
	}
	
	
	template <typename T_>
	inline void
	BoundingBox<T_>::extend(const BoundingBox<T_>& bb)
	{
		extend(bb.x_min(), bb.y_min(), bb.x_max(), bb.y_max());
	}
	
	
	template <typename T_>	
	inline void
	BoundingBox<T_>::extend(const T_& value)
	{
		if (initialized() && value >= 0)
		{
			_x_min -= value;
			_y_min -= value;
			_x_max += value;
			_y_max += value;
		}
	}
	
	
	template <typename T_>
	inline void
	BoundingBox<T_>::extend(const T_& x, const T_& y)
	{
		if (!initialized())
		{
			set(x, y);
			return;
		}
	
		if (_x_min > x)
			_x_min = x;
		else if (_x_max < x)
			_x_max = x;
				
		if (_y_min > y)
			_y_min = y;
		else if (_y_max < y)
			_y_max = y;
	}
	
	
	template <typename T_>
	inline void
	BoundingBox<T_>::extend(const T_& x_min, const T_& y_min, const T_& x_max, const T_& y_max)
	{
		assert(x_min <= x_max);
		assert(y_min <= y_max);
		
		if (!initialized())
		{
			set(x_min, y_min, x_max, y_max);
			return;
		}
				
		if (_x_min > x_min) _x_min = x_min;
		if (_y_min > y_min) _y_min = y_min;
		if (_x_max < x_max) _x_max = x_max;
		if (_y_max < y_max) _y_max = y_max;
	}
	
	
	template <typename T_>
	inline void
	BoundingBox<T_>::get(T_& x_min, T_& y_min, T_& x_max, T_& y_max)
	{
		x_min = _x_min;
		y_min = _y_min;
		x_max = _x_max;
		y_max = _y_max;
	}
	
	
	template <typename T_>
	inline bool
	BoundingBox<T_>::initialized()
	{
		return _initialized;
	}
	
	
	template <typename T_>
	inline void
	BoundingBox<T_>::set(const T_& x, const T_& y)
	{
		_x_min = x;
		_y_min = y;
		_x_max = x;
		_y_max = y;
		_initialized = true;
	}
	
	
	
	template <typename T_>
	inline void
	BoundingBox<T_>::set(const T_& x_min, const T_& y_min, const T_& x_max, const T_& y_max)
	{
		assert(x_min <= x_max);
		assert(y_min <= y_max);
		
		_x_min = x_min;
		_y_min = y_min;
		_x_max = x_max;
		_y_max = y_max;
		_initialized = true;		
	}

} // namespace mapgeneration_gui


#endif //BOUNDINGBOX_H
