/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "openconnectiondialoglogic.h"
#endif

#include "openconnectiondialoglogic.h"

#include <wx/valtext.h>
#include <wx/xrc/xmlres.h>

#include "extendedgenericvalidator.h"

namespace mapgeneration_gui
{
	
	IMPLEMENT_DYNAMIC_CLASS(OpenConnectionDialogLogic, DialogLogic)

	BEGIN_EVENT_TABLE(OpenConnectionDialogLogic, wxDialog)
		EVT_BUTTON(XRCID("ID_BUTTON_CANCEL"), OpenConnectionDialogLogic::on_button_cancel_click)
		EVT_BUTTON(XRCID("ID_BUTTON_CONNECT"), OpenConnectionDialogLogic::on_button_connect_click)
		
		EVT_BUTTON(XRCID("ID_BUTTON_DELETE"), OpenConnectionDialogLogic::on_button_delete_click)
		EVT_BUTTON(XRCID("ID_BUTTON_LOAD"), OpenConnectionDialogLogic::on_button_load_click)
		EVT_BUTTON(XRCID("ID_BUTTON_SAVE"), OpenConnectionDialogLogic::on_button_save_click)
		
		EVT_CLOSE(OpenConnectionDialogLogic::on_close_window)
		
		EVT_LISTBOX(XRCID("ID_LISTBOX_SAVEDSESSIONS"),
			OpenConnectionDialogLogic::on_listbox_savedsessions_selected)
		EVT_LISTBOX_DCLICK(XRCID("ID_LISTBOX_SAVEDSESSIONS"),
			OpenConnectionDialogLogic::on_listbox_savedsessions_double_clicked)
	END_EVENT_TABLE()
	
	
	OpenConnectionDialogLogic::OpenConnectionDialogLogic()
	: _current_host(),
		_current_port(),
		_saved_sessions()
	{
		set_modal(true);
	}
	
	
	const wxString&
	OpenConnectionDialogLogic::current_host() const
	{
		return _current_host;
	}
	

	const wxString&
	OpenConnectionDialogLogic::current_port() const
	{
		return _current_port;
	}


	bool
	OpenConnectionDialogLogic::intern_load_dialog()
	{
		if (wxXmlResource::Get()->
			LoadDialog(this, GetParent(), wxT("ID_DIALOG_OPENCONNECTION")) == false)
		{
			return false;
		}
		
		_listbox_savedsessions = XRCCTRL(*this, "ID_LISTBOX_SAVEDSESSIONS", wxListBox);
		
		if(FindWindow(XRCID("ID_LISTBOX_SAVEDSESSIONS")))
		{
			FindWindow(XRCID("ID_LISTBOX_SAVEDSESSIONS"))->SetValidator(ExtendedGenericValidator(&_saved_sessions));
		}
		
		if(FindWindow(XRCID("ID_TEXTCTRL_HOST")))
		{
			FindWindow(XRCID("ID_TEXTCTRL_HOST"))->SetValidator(wxTextValidator(wxFILTER_ASCII, &_current_host));
		}
		
		if(FindWindow(XRCID("ID_TEXTCTRL_PORT")))
		{
			FindWindow(XRCID("ID_TEXTCTRL_PORT"))->SetValidator(wxTextValidator(wxFILTER_NUMERIC, &_current_port));
		}
		
		return true;
	}
	

	void
	OpenConnectionDialogLogic::on_button_cancel_click(wxCommandEvent& event)
	{
		OnCancel(event);
	}


	void
	OpenConnectionDialogLogic::on_button_connect_click(wxCommandEvent& event)
	{
		OnOK(event);
	}
	
	
	void
	OpenConnectionDialogLogic::on_button_delete_click(wxCommandEvent& event)
	{
		const int selected_item_index = _listbox_savedsessions->GetSelection();
		if (selected_item_index == wxNOT_FOUND)
			return;
		    
		_listbox_savedsessions->Delete(selected_item_index);
		TransferDataFromWindow();
	}


	void
	OpenConnectionDialogLogic::on_button_load_click(wxCommandEvent& event)
	{
		const int selected_item_index = _listbox_savedsessions->GetSelection();
		if (selected_item_index == wxNOT_FOUND)
			return;
		
		const wxString& selected_item = _listbox_savedsessions->GetString(selected_item_index);
		const size_t index_of_colon = selected_item.Find(':');
		if (index_of_colon == -1)
			return;
		    
		_current_host = selected_item.Mid(0, index_of_colon);
		_current_port = selected_item.Mid(index_of_colon + 1);
		TransferDataToWindow();
		_listbox_savedsessions->SetSelection(selected_item_index);
	}


	void
	OpenConnectionDialogLogic::on_button_save_click(wxCommandEvent& event)
	{
		TransferDataFromWindow();
		
		if ((_current_host.Len() == 0) || (_current_port.Len() == 0))
			return;
		
		const wxString new_savedsession(_current_host + ":" + _current_port);
		
		bool found_same_item = false;
		int count = _saved_sessions.GetCount();
		for (int i = 0; i < count; ++i)
		{
			if (_saved_sessions[i] == new_savedsession)
			{
				found_same_item = true;
				break;
			}
		}
		
		if (!found_same_item)
		{
			_saved_sessions.Add(new_savedsession);
			TransferDataToWindow();
		}
	}


	void
	OpenConnectionDialogLogic::on_listbox_savedsessions_double_clicked(wxCommandEvent& event)
	{
		on_button_connect_click(event);
	}


	void
	OpenConnectionDialogLogic::on_listbox_savedsessions_selected(wxCommandEvent& event)
	{
		on_button_load_click(event);
	}


	void
	OpenConnectionDialogLogic::on_close_window(wxCloseEvent& event)
	{
		if (IsModal())
		{
			EndModal(wxID_CANCEL);
		} else
		{
			SetReturnCode(wxID_CANCEL);
			Show(false);
		}
	}

} // namespace mapgeneration_gui
