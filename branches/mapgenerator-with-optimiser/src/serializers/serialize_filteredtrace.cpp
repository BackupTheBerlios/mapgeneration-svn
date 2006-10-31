/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <fstream>
#include <cc++/thread.h>

#include "filteredtrace.h"
#include "filteredtracereceiver.h"
#include "util/serializer.h"

namespace mapgeneration
{
	
	class FilteredTraceReceiverImpl : public FilteredTraceReceiver
	{
		
		public:
			
			FilteredTraceReceiverImpl(const std::string& filename);
			
			
			void
			new_trace(FilteredTrace& filtered_trace);
			
			
		private:
			
			int _counter;
			std::string _filename;
			ost::Mutex _mutex;
			
	};
	
	
	FilteredTraceReceiverImpl::FilteredTraceReceiverImpl(
		const std::string& filename)
	: _counter(0), _filename(filename), _mutex()
	{
	}
	
	
	void
	FilteredTraceReceiverImpl::new_trace(FilteredTrace& filtered_trace)
	{
		ost::MutexLock lock(_mutex);
		
		std::stringstream ss;
		ss << _filename << _counter;
		ss.flush();
		
		++_counter;
		
		std::ofstream out_file_stream(ss.str().c_str());
		Serializer::serialize(out_file_stream, filtered_trace);
		out_file_stream.close();
	}
	
} // namespace mapgeneration


#include "defaultconfiguration.h"
#include "tracefilter.h"
#include "traceserver.h"
#include "util/configuration.h"
#include "util/mlog.h"
#include "util/pubsub/servicesystem.h"

using namespace mapgeneration;
using mapgeneration_util::MLog;
using pubsub::ServiceList;


int main(int argc, char* argv[])
{
	// startup sequence.
	mlog(MLog::info, "SerializeFilteredTrace") << "Initializing ServiceList.\n";
	ServiceList* service_list = new pubsub::ServiceList();
	mlog(MLog::info, "SerializeFilteredTrace") << "ServiceList initialized.\n";
	
	mlog(MLog::info, "SerializeFilteredTrace") << "Loading configuration.\n";
	DefaultConfiguration default_configuration;
	std::vector< Configuration::Parameter > dc
		= default_configuration.get();
	Configuration configuration(CONFIGURATION_PATH, service_list, "",
		&dc);
	configuration.read_configuration();
	mlog(MLog::info, "SerializeFilteredTrace") << "Configuration loaded.\n";
	
	mlog(MLog::info, "SerializeFilteredTrace")
		<< "Checking licence acceptance.\n";
	std::string licence_accepted;
	if (!service_list->get_service_value("accepted_licence", 
		licence_accepted) ||
		(licence_accepted != "yes"))
	{
		std::cout << "\nYou did not yet accept the licence.\n"
				  << "This program is licenced under the Academic Free License version 2.1.\n"
				  << "The complete licence text is available in the LICENCE file and at\n"
				  << "http://www.opensource.org/licenses/afl-2.1.php\n"
				  << "\n"
				  << "If you understand and accept the licence terms enter 'y' to continue, \n"
				  << "otherwise press enter to abort program execution.\n"
				  << "If you are the only user you may also change the accepted_licence\n"
				  << "value in the mapgenerator.conf file to 'yes' to accept the licence.\n\n";
		char c = getchar();
		if (c != 'y' && c != 'Y')
		{
			mlog(MLog::info, "ExecutionManager") << 
				"Licence not accepted.\n";
			throw("Licence not accepted!");
		}
	}
	mlog(MLog::info, "SerializeFilteredTrace") << "Licence accepted.\n";
	
	mlog(MLog::info, "SerializeFilteredTrace")
		<< "Starting FilteredTraceReceiver.\n";
	FilteredTraceReceiver* filtered_trace_receiver = 0;
	if (argc > 1)
	{
		filtered_trace_receiver
			= new FilteredTraceReceiverImpl(std::string(argv[1]));
	} else
	{
		filtered_trace_receiver
			= new FilteredTraceReceiverImpl("filteredtrace_");
	}
	mlog(MLog::info, "SerializeFilteredTrace")
		<< "FilteredTraceReceiver started.\n";
	
	mlog(MLog::info, "SerializeFilteredTrace") << "Starting TraceFilter.\n";
	TraceFilter* trace_filter
		= new TraceFilter(service_list, filtered_trace_receiver);
	trace_filter->controlled_start();
	mlog(MLog::info, "SerializeFilteredTrace") << "TraceFilter started.\n";
	
	mlog(MLog::info, "SerializeFilteredTrace") << "Starting TraceServer.\n";
	/** @todo change parameters here: TileManager -> TraceFilter */
	TraceServer* trace_server = new TraceServer(service_list, trace_filter);
	trace_server->controlled_start();
	mlog(MLog::info, "SerializeFilteredTrace") << "TraceServer started.\n";
	
	// everthing started. we can receive traces now.
	// stop this program with "x".
	bool stop = false;
	while (!stop)
	{
		char c = getchar();
		
		if (c == 'x')
			stop = true;
	}
	
	// shutdown sequence.
	mlog(MLog::info, "SerializeFilteredTrace")
		<< "SerializeFilteredTrace shutting down! :-(\n";
	
	mlog(MLog::info, "SerializeFilteredTrace") << "Stopping TraceServer...\n";
	trace_server->controlled_stop();
	delete trace_server;
	mlog(MLog::info, "SerializeFilteredTrace") << "TraceServer stopped.\n";

	mlog(MLog::info, "SerializeFilteredTrace") << "Stopping TraceFilter...\n";
	trace_filter->controlled_stop();
	delete trace_filter;
	mlog(MLog::info, "SerializeFilteredTrace") << "TraceFilter stopped.\n";
	
	mlog(MLog::info, "SerializeFilteredTrace")
		<< "Deleting FilteredTraceReceiver...\n";
	delete filtered_trace_receiver;
	mlog(MLog::info, "SerializeFilteredTrace")
		<< "FilteredTraceReceiver deleted.\n";
	
	mlog(MLog::info, "SerializeFilteredTrace") << "Deleting ServiceList...\n";
	delete service_list;
	mlog(MLog::info, "SerializeFilteredTrace") << "ServiceList deleted.\n";
}
