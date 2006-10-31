/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EXTENDEDGENERICVALIDATOR_H
#define EXTENDEDGENERICVALIDATOR_H

#include <wx/valgen.h>
#include <wx/wx.h>

#include "../util/pubsub/genericservice.h"

namespace mapgeneration_gui
{

	class ExtendedGenericValidator : public wxValidator {
		
		public:
				
			ExtendedGenericValidator(wxArrayString* wx_array_string);

			
			ExtendedGenericValidator(pubsub::GenericService* service);
			
			/**
			 * @brief Copy Constructor.
			 */
			ExtendedGenericValidator(const ExtendedGenericValidator& validator);
			
			
			virtual ExtendedGenericValidator*
			Clone() const;
			
			
			virtual bool
			TransferFromWindow();
			
			
			virtual bool
			TransferToWindow();
			
			inline virtual bool
			Validate(wxWindow* parent);
		

		private:
			
			wxArrayString* _wx_array_string;
	};
	
	
	inline bool
	ExtendedGenericValidator::Validate(wxWindow* parent)
	{
		return true;
	}
	
} // namespace mapgeneration_gui

#endif //EXTENDEDGENERICVALIDATOR_H
