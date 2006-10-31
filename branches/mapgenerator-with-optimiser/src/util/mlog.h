/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MLOG_H
#define MLOG_H

#include <iostream>

namespace mapgeneration_util
{

	/** 
	 * @brief MLog provides a simple logging facility.
	 * 
	 * Nearly nothing is implemented or supported at the moment.
	 */
	class MLog : protected std::streambuf, public std::ostream
	{
				
		public:
		
			/**
			 *  @brief Enumeration for the different log levels from 
			 * debug (nobody cares) to emergency (something is _really_ going
			 * wrong.
			 */
			enum Level 
			{
				emergency = 1, 
				alert, 
				error, 
				warning, 
				notice, 
				info, 
				debug 
			};
		

			MLog();

			~MLog();
			
			/**
			 * @brief Gets the display level.
			 */
			inline Level
			get_display_level() const;
			
			
			/**
			 * @brief The operator<< to send data into the log.
			 */
			std::ostream& operator<<(std::ostream& os);
			
			
			/**
			 * @brief The operator() to set the level for the next messages.
			 */
			inline MLog& 
			operator()(Level level);
			

			/**
			 * @brief The operator() to set the source name for the next 
			 * messages.
			 */
			inline MLog& 
			operator()(const char* source);
			

			/**
			 * @brief The operator() to set level and source name for the next
			 * messages.
			 */
			inline MLog& 
			operator()(MLog::Level level, const char* source);
			
			
			/**
			 * @brief Sets the display level.
			 */
			inline void
			set_display_level(Level display_level);
			
			
		protected:

			/**
			 * @brief The overflow method to process the stream characters.
			 */
			int overflow (int character);


		private:
			
			/**
			 * @brief The greatest level that is displayed.
			 */
			Level _display_level;
			
			/**
			 * @brief The current level.
			 */
			Level _level;
			
			/**
			 * @brief The current source.
			 */
			std::string _source;
			
			/**
			 * @brief The buffer that holds the log messages as long as they
			 * are not sent to the loggers.
			 * 
			 * Loggers are not yet implemented, at the moment output is always
			 * sent to std::out.
			 */
			std::string _buffer;
	};
	
	
	inline MLog::Level
	MLog::get_display_level() const
	{
		return _display_level;
	}
	
	
	inline MLog& 
	MLog::operator()(Level level)
	{
		_level = level;
		
		return *this;
	}


	inline MLog& 
	MLog::operator()(const char* source)
	{
		_source = source;
		
		return *this;
	}


	inline MLog& 
	MLog::operator()(MLog::Level level, const char* source)
	{
		_level = level;
		_source = source;
		
		
		return *this;
	}
	
	
	inline void
	MLog::set_display_level(MLog::Level display_level)
	{
		_display_level = display_level;
	}
	
	
	extern MLog mlog;
	

} // namespace mapgeneration_util

#endif // MLOG_H
