/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EXPORT_H
#define EXPORT_H

#include "../dbconnection.h"
#include "../edgecache.h"
#include "../tilecache.h"

using namespace mapgeneration;

namespace mapgeneration_export
{
	
	/**
	 * @brief Export provides methods to export the mapgeneration DB structure
	 * into almost every format.
	 * 
	 * Actually there is only one export function supported :-)
	 * This format is the FileGraph format of the routing group.
	 * 
	 * @see Wiki/FileArchive
	 */
	class Export {

		public:
	
			/**
			 * @brief Simple Enumeration containing every supported export format.
			 */
			enum Format {
				_ROUTING_FILE
			};

			
			/**
			 * @brief Constructor that inits the export format.
			 * 
			 * @param format the export format
			 */
			Export(Format format);
		
		
			/**
			 * @brief Destructor.
			 */
			~Export();


			/**
			 * @brief Start conversion.
			 */
			inline void
			convert();
			
			
		private:
		
			/**
			 * @brief Pointer to the EdgeCache.
			 */
			EdgeCache* _edge_cache;
			
			
			/**
			 * @brief Pointer to the DBConnection.
			 */
			DBConnection* _db_connection;
			
		
			/**
			 * @brief Saves the export format.
			 */
			Format _format;
			
			
			/**
			 * @brief Pointer to the TileCache.
			 */
			TileCache* _tile_cache;
			
			
			/**
			 * @brief Converts to the routing file format.
			 */
			void
			convert_to_routing_file_format();
		
	};


	inline void
	Export::convert()
	{
		switch(_format)
		{
			case _ROUTING_FILE:
				convert_to_routing_file_format();
				break;
		}
	}

} // namespace mapgeneration_export

#endif
