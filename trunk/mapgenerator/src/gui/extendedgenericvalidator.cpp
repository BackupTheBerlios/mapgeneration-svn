/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "extendedgenericvalidator.h"

#include <iostream>
#include <typeinfo>

namespace mapgeneration_gui
{
	
	ExtendedGenericValidator::ExtendedGenericValidator(wxArrayString* wx_array_string)
	: _wx_array_string(wx_array_string), wxValidator::wxValidator()
	{
	}

	
	ExtendedGenericValidator::ExtendedGenericValidator(pubsub::GenericService* service)
	: wxValidator::wxValidator()
	{
		/** @todo write it! */
	}

	
	ExtendedGenericValidator::ExtendedGenericValidator(const ExtendedGenericValidator& validator)
	: _wx_array_string(validator._wx_array_string), wxValidator::wxValidator()
	{
	}
	
	
	ExtendedGenericValidator*
	ExtendedGenericValidator::Clone() const
	{
		return new ExtendedGenericValidator(*this);
	}
	
	
	bool
	ExtendedGenericValidator::TransferFromWindow()
	{
		const wxWindow* window = GetWindow();
		if ((window != 0) && (dynamic_cast<const wxControlWithItems*>(window) != 0))
		{
			const wxControlWithItems* control_with_items
				= dynamic_cast<const wxControlWithItems*>(window);
			
			_wx_array_string->Clear();
			
			const int count = control_with_items->GetCount();
			for(int i = 0; i < count; ++i)
			{
				_wx_array_string->Add(control_with_items->GetString(i));
			}
				
		} else
		{
			return false;
		}
		
		return true;
	}


	bool
	ExtendedGenericValidator::TransferToWindow()
	{
		wxWindow* window = GetWindow();
		if ((window != 0) && (dynamic_cast<wxControlWithItems*>(window) != 0))
		{
			wxControlWithItems* control_with_items
				= dynamic_cast<wxControlWithItems*>(window);
			
			(dynamic_cast<wxItemContainer*>(control_with_items))->Clear();
			
			const int count = _wx_array_string->GetCount();
			for(int i = 0; i < count; ++i)
			{
				control_with_items->Append(_wx_array_string[i]);
			}
				
		} else
		{
			return false;
		}
		
		return true;
	}

} // namespace mapgeneration_gui
