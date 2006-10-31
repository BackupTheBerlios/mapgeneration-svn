/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef DIALOGLOGIC_H
#define DIALOGLOGIC_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "dialoglogic.cpp"
#endif

#include <wx/dialog.h>

namespace mapgeneration_gui
{
	
	class DialogLogic : public wxDialog
	{
		
		DECLARE_ABSTRACT_CLASS(DialogLogic);
		
		public:
		
			inline DialogLogic();
			
			
			inline bool
			load_dialog(wxWindow* parent);
			

			inline int
			show_dialog();
			
			
		protected:
			
			inline bool
			get_modal() const;


			inline virtual bool
			intern_load_dialog();
			
			
			inline void
			set_modal(bool modal);
			
			
		private:
			
			bool _modal;
			
	};
	
	
	DialogLogic::DialogLogic()
	{
	}


	bool
	DialogLogic::get_modal() const
	{
		return _modal;
	}
	
	
	bool
	DialogLogic::intern_load_dialog()
	{
		return false;
	}
	
	
	bool
	DialogLogic::load_dialog(wxWindow* parent)
	{
		if (parent != NULL)
		{
			SetParent(parent);
		
			return intern_load_dialog();
		}
		
		return false;
	}
	
	
	void
	DialogLogic::set_modal(bool modal)
	{
		_modal = modal;
	}


	int
	DialogLogic::show_dialog()
	{
		/** @todo works that??? */
		if (_modal)
		{
			return ShowModal();
		} else
		{
			Show();
			return 0; /** @todo check return value */
		}
	}
	
} // namespace mapgeneration_gui

#endif // PREFERENCESDIALOGLOGIC_H
