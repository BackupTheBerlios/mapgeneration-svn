/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "mgg.h"
#endif

#include "mgg.h"

#include <wx/xrc/xmlres.h>

#include "mainframelogic.h"
#include "util/mlog.h"


using mapgeneration_util::MLog;


IMPLEMENT_APP(MGG)


namespace mapgeneration_gui
{

	IMPLEMENT_CLASS(MGG, wxApp)
	
	MGG::MGG()
	{
	}
	
	
	int
	MGG::OnExit()
	{
		return wxApp::OnExit();
	}


	bool
	MGG::OnInit()
	{
		/** @todo Change paths (xml configuration file)
		 *  and give responsibility for loading xrcs to corresponding
		 *  components or to ModuleManager (probably better!) */
		if (wxXmlResource::Get()->Load(wxT("xrcs/mainframe.xrc")) == false)
		{
			mlog(MLog::error, "MGG") << "Problem with \"xrcs/mainframe.xrc\"!\n";
			return false;
		}
		if (wxXmlResource::Get()->Load(wxT("xrcs/openconnectiondialog.xrc")) == false)
		{
			mlog(MLog::error, "MGG") << "Problem with \"xrcs/openconnectiondialog.xrc\"!\n";
			return false;
		}
		if (wxXmlResource::Get()->Load(wxT("xrcs/preferencesdialog.xrc")) == false)
		{
			mlog(MLog::error, "MGG") << "Problem with \"xrcs/preferencesdialog.xrc\"!\n";
			return false;
		}
		if (wxXmlResource::Get()->Load(wxT("xrcs/preferencesgeneralpanel.xrc")) == false)
		{
			mlog(MLog::error, "MGG") << "Problem with \"xrcs/preferencesgeneralpanel.xrc\"!\n";
			return false;
		}
		if (wxXmlResource::Get()->Load(wxT("xrcs/testmodule.xrc")) == false)
		{
			mlog(MLog::error, "MGG") << "Problem with \"xrcs/testmodule.xrc\"!\n";
			return false;
		}
		if (wxXmlResource::Get()->Load(wxT("xrcs/mappanel.xrc")) == false)
		{
			mlog(MLog::error, "MGG") << "Problem with \"xrcs/mappanel.xrc\"!\n";
			return true;
		}
		if (wxXmlResource::Get()->Load(wxT("xrcs/tracelogpanel.xrc")) == false)
		{
			mlog(MLog::error, "MGG") << "Problem with \"xrcs/tracelogpanel.xrc\"!\n";
			return true;
		}
		
		mlog(MLog::debug, "MGG") << "All xrcs loaded!\n";

		wxXmlResource::Get()->InitAllHandlers();

		#if wxUSE_XPM
			wxImage::AddHandler( new wxXPMHandler );
		#endif
		#if wxUSE_LIBPNG
			wxImage::AddHandler( new wxPNGHandler );
		#endif
		#if wxUSE_LIBJPEG
			wxImage::AddHandler( new wxJPEGHandler );
		#endif
		#if wxUSE_GIF
			wxImage::AddHandler( new wxGIFHandler );
		#endif
		
		MainFrameLogic* main_frame_logic = new MainFrameLogic;
		main_frame_logic->load_frame();
		main_frame_logic->show_frame();
		
		return true;
	}
	
} // namespace mapgeneration_gui
