/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CONTROLLEDTHREAD_H
#define CONTROLLEDTHREAD_H

#include <cc++/thread.h>


namespace mapgeneration_util
{

	/**
	 * @brief ControlledThread provides a framework for starting, stopping and pausing
	 * threads in a clean and fast way.
	 * 
	 * ControlledThread is a subclass of ost::Thread. It adds three states to 
	 * a thread: Running, paused and stopped. And this is how it
	 * works:
	 * You have to create a subclass as usual for thread. Then you can start it 
	 * by calling controlled_start(). The thread is immediatly started and calls
	 * thread_init() and thread_run(). Controlled_start() without a parameter 
	 * or wait=true waits until thread_init() is finished. If you want the 
	 * method to be non blocking you have to call controlled_start(false).
	 * When you want to stop the thread you call controlled_stop().
	 * Controlled_stop() sends a _should_stop_event
	 * and sets _should_stop to true, so the thread_run()-method should either
	 * wait for the event or at least check should_stop() sometimes and leave
	 * if should_stop() is true. Controlled_stop() waits for the thread_run 
	 * method to stop and the following thread_deinit() call to finish before
	 * returning. If you want controlled_stop() to be non blocking you have
	 * to use the parameter false as for controlled_start().
	 * Pauses are not yet implemented at the moment.
	 */
	class ControlledThread : public ost::Thread {

		public:

			ControlledThread();


			~ControlledThread();
			
			
			/**
			 * @brief Not yet implemented.
			 */
			void
			controlled_pause(bool wait = true);
			
			
			/**
			 * @brief Returns the pause state of the thread.
			 * 
			 * Because pauses are not yet implemented the return value should
			 * be false.
			 * @return false
			 */
			inline bool
			paused();
			
			
			/**
			 * @brief Returns true if the thread is running and neither paused
			 * nor stopped.
			 * @return True if thread is running, false otherwise.
			 */
			inline bool
			running();
			
			
			/**
			 * @brief Starts the thread and waits for the thread to initialize
			 * if the wait parameter is true.
			 * @param wait Wait for thread initialization?
			 */
			void
			controlled_start(bool wait = true);


			/**
			 * @brief Stops the thread and waits for the thread to deinitialize
			 * if the wait parameter is true.
			 * @param wait Wait for the thread to deinitialize?
			 */
			void
			controlled_stop(bool wait = true);
		
				
		protected:
		
			/**
			 * @brief The ost::event to signal the thread that he has to stop.
			 */
			ost::Event _should_stop_event;
			
		
			/**
			 * @brief Deinitialization method for the thread, can be overloaded
			 * by subclasses. Empty by default.
			 */
			virtual void 
			thread_deinit();


			/**
			 * @brief Initialization method for the thread, can be overloaded
			 * by subclasses. Empty by default.
			 */
			virtual void
			thread_init();


			/**
			 * @brief Run method for the thread, can be overloaded
			 * by subclasses. Empty by default.
			 */			
			virtual void
			thread_run();


			/**
			 * @brief Returns true if the thread should stop.
			 * @return True if the thread should stop.
			 */
			inline bool
			should_stop();


		private:

			/**
			 * @brief True if, and only if, the thread is paused.
			 */
			bool _paused;

			/**
			 * @brief True if the thread is running.
			 */
			bool _running;

			/**
			 * @brief True if the thread should pause.
			 */		
			bool _should_pause;

			/**
			 * @brief True if the thread should stop.
			 */
			bool _should_stop;
			
			/**
			 * @brief Event for the wait=true cases in the controlled_??? 
			 * methods.
			 */
			ost::Event _event;
			
			/**
			 * @brief Main run method of the thread.
			 */
			void
			run();

	};
	
	
	inline bool
	ControlledThread::paused()
	{
		return _paused;
	}
	
	
	inline bool
	ControlledThread::running()
	{
		return _running;
	}
	

	inline bool
	ControlledThread::should_stop()
	{
		return _should_stop;
	}

	
	

} // namespace mapgeneration_util

#endif //CONTROLLEDTHREAD_H
