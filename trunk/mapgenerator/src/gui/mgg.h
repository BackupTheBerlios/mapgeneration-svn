/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MGG_H
#define MGG_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "mgg.cpp"
#endif

#include <wx/app.h>

namespace mapgeneration_gui
{
	
	class MGG: public wxApp
	{
		
		DECLARE_CLASS(MGG)
		
		
		public:
			
			MGG();
			
			
		private:
			
			virtual int OnExit();
			
			
			virtual bool OnInit();
			
	};
		
} // namespace mapgeneration_gui

using mapgeneration_gui::MGG;
DECLARE_APP(MGG)


#endif // MGG_H
