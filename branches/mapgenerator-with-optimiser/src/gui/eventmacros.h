/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EVENTMACROS_H
#define EVENTMACROS_H

#include <wx/event.h>
#include <wx/xrc/xmlres.h>


#define \
	MGG_EVT(event, event_source, function, function_owner) \
	event_source-> \
		Connect(event_source->GetId(), event, \
			(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)(&function), \
			NULL, function_owner);

#define \
	MGG_XRC_EVT(event, parent, event_source_xrc_id_string, function, function_owner) \
	XRCCTRL(*parent, event_source_xrc_id_string, wxEvtHandler)-> \
		Connect(XRCID(event_source_xrc_id_string), event, \
			(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)(&function), \
			NULL, function_owner);

#define \
	MGG_EVT_BUTTON(event_source, function, function_owner) \
	event_source-> \
		Connect(event_source->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, \
			(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)(&function), \
			NULL, function_owner);
			
#define \
	MGG_XRC_EVT_BUTTON(parent, event_source_xrc_id_string, function, function_owner) \
	XRCCTRL(*parent, event_source_xrc_id_string, wxEvtHandler)-> \
		Connect(XRCID(event_source_xrc_id_string), wxEVT_COMMAND_BUTTON_CLICKED, \
			(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)(&function), \
			NULL, function_owner);

#define \
	MGG_EVT_PAINT(event_source, function, function_owner) \
	event_source-> \
		Connect(event_source->GetId(), wxEVT_PAINT, \
			(wxObjectEventFunction)(wxEventFunction)(wxPaintEventFunction)(&function), \
			NULL, function_owner);

#define \
	MGG_XRC_EVT_PAINT(parent, event_source_xrc_id_string, function, function_owner) \
	XRCCTRL(*parent, event_source_xrc_id_string, wxEvtHandler)-> \
		Connect(XRCID(event_source_xrc_id_string), wxEVT_PAINT, \
			(wxObjectEventFunction)(wxEventFunction)(wxPaintEventFunction)(&function), \
			NULL, function_owner);
			
#define \
	MGG_EVT_TOOL(event_source, function, function_owner) \
	event_source-> \
		Connect(event_source->GetId(), wxEVT_COMMAND_TOOL_CLICKED, \
			(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)(&function), \
			NULL, function_owner);
			
#define \
	MGG_XRC_EVT_TOOL(parent, event_source_xrc_id_string, function, function_owner) \
	XRCCTRL(*parent, event_source_xrc_id_string, wxEvtHandler)-> \
		Connect(XRCID(event_source_xrc_id_string), wxEVT_COMMAND_TOOL_CLICKED, \
			(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)(&function), \
			NULL, function_owner);

#define \
	MGG_EVT_TREE_SEL_CHANGED(event_source, function, function_owner) \
	event_source-> \
		Connect(event_source->GetId(), wxEVT_COMMAND_TREE_SEL_CHANGED, \
			(wxObjectEventFunction)(wxEventFunction)(wxTreeEventFunction)(&function), \
			NULL, function_owner);
			
#define \
	MGG_XRC_EVT_TREE_SEL_CHANGED(parent, event_source_xrc_id_string, function, function_owner) \
	XRCCTRL(*parent, event_source_xrc_id_string, wxEvtHandler)-> \
		Connect(XRCID(event_source_xrc_id_string), wxEVT_COMMAND_TREE_SEL_CHANGED, \
			(wxObjectEventFunction)(wxEventFunction)(wxTreeEventFunction)(&function), \
			NULL, funtion_owner);
			
#endif // EVENTMACROS_H
