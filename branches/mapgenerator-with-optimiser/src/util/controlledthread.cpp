/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "controlledthread.h"

#include "util/mlog.h"

namespace mapgeneration_util
{


	ControlledThread::ControlledThread()
	: _should_stop_event(), _event(), _paused(false), _running(false), _should_pause(false), _should_stop(false)
	{
	}
	
	
	ControlledThread::~ControlledThread()
	{
		if (running())
			mlog(MLog::warning, "ControlledThread::~ControlledThread")
				<< "Deleting running thread!";
	}
	
	
	void
	ControlledThread::controlled_pause(bool wait)
	{
		mlog(MLog::debug, "ControlledThread::pause") 
			<< "Pauses are not yet implemented, had to drink a coffee....";
	}
	
	
	void
	ControlledThread::controlled_start(bool wait)
	{
		if (running()) return;

		_event.reset();
		_should_stop_event.reset();
		start();
		
		if (wait && !running())
			_event.wait(); /** @todo unsafe */
	}


	void
	ControlledThread::controlled_stop(bool wait)
	{
		if (!running()) return;
		
		_event.reset();		
		_should_stop = true;
		_should_stop_event.signal();
		
		if (wait && running())
			_event.wait(); /** @todo unsafe */
	}


	void
	ControlledThread::thread_init()
	{
	}


	void
	ControlledThread::thread_run()
	{
	}

	
	void
	ControlledThread::thread_deinit()
	{
	}
	
	
	void
	ControlledThread::run()
	{
		thread_init();
		_running = true;
		_event.signal();
		thread_run();
		thread_deinit();
		_running = false;
		_event.signal();
	}
	
} // namespace mapgeneration_util
