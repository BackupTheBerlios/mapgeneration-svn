/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mapdrawer.h"

#include <iostream>
#include <set>

#include "mapgenerationdraw.h"



namespace mapgeneration_gui
{
	
	MapDrawer::MapDrawer
		(wxScrolledWindow* map_scrolled_window, GPSDraw* gps_draw, 
		TileCache* tile_cache)		
	: 	_current_draw_id(1),
		_gps_draw(gps_draw),
		_last_paint_seconds(0),
		_map_scrolled_window(map_scrolled_window),
		_tile_cache(tile_cache), 
		_tile_prefetch_caller(0, this, &MapDrawer::tile_prefetched),
		_used_tile_blocks_5(), 
		_used_tile_blocks_50(), 
		_use_prefetch(true)
	{		
		_tile_prefetch_proxy.register_subscriber(&_tile_prefetch_caller);		
		
		std::bitset<3600> empty_bitset_5;
		empty_bitset_5.reset();		
		_used_tile_blocks_5.
			insert(_used_tile_blocks_5.end(), 7200, empty_bitset_5);

		std::bitset<360> empty_bitset_50;
		empty_bitset_50.reset();
		_used_tile_blocks_50.
			insert(_used_tile_blocks_50.end(), 720, empty_bitset_50);

		if (_tile_cache != 0)
			update_used_tile_blocks();
	}
	
	
	void
	MapDrawer::draw_tile(unsigned int tile_id,
		unsigned int min_tile_id_northing, unsigned int min_tile_id_easting, 
		unsigned int max_tile_id_northing, unsigned int max_tile_id_easting)
	{
		TileCache::Pointer current_tile = _tile_cache->get(tile_id);
		if (current_tile != 0)
		{
			FixpointVector<Node>::iterator node_iter =
				current_tile.write().nodes().begin();
			FixpointVector<Node>::iterator node_iter_end =
				current_tile.write().nodes().end();
			for (; node_iter != node_iter_end; ++node_iter)
			{
				std::vector<Node::Id>::const_iterator next_node_id_iter = 
					node_iter->second.next_node_ids().begin();
				std::vector<Node::Id>::const_iterator next_node_id_iter_end =
					node_iter->second.next_node_ids().end();
				for (; next_node_id_iter != next_node_id_iter_end; ++next_node_id_iter)
				{
					int nnorthing;
					int neasting;
					GeoCoordinate::split_tile_id(Node::tile_id(*next_node_id_iter),
						nnorthing, neasting);
					if (nnorthing>=min_tile_id_northing &&
						nnorthing<=max_tile_id_northing &&
						neasting>=min_tile_id_easting &&
						neasting<=max_tile_id_easting)
					{
						TileCache::Pointer op = _tile_cache->get(Node::tile_id(*next_node_id_iter));
						if (op != 0)
						{
							GeoCoordinate nnode=op->nodes()[Node::local_id(*next_node_id_iter)].second;
							MapGenerationDraw::arrow(_gps_draw, node_iter->second, nnode);
						}
					}
				}
			}
		}
	}	


	void
	MapDrawer::on_paint(wxPaintEvent& wx_paint_event, wxDC& dc)
	{
		if (_tile_cache == 0) return;
		
		_last_paint_seconds = time(0);
		long tiles_to_display;
		
		int pixels_per_unit_x, pixels_per_unit_y;
		_map_scrolled_window->GetScrollPixelsPerUnit(&pixels_per_unit_x, &pixels_per_unit_y);
		
		int view_start_x, view_start_y, view_end_x, view_end_y;
 		wxRegionIterator update_iter(_map_scrolled_window->GetUpdateRegion());

		while (update_iter)
		{
			_map_scrolled_window->GetViewStart(&view_start_x, &view_start_y);
			view_start_x *= pixels_per_unit_x;
			view_start_y *= pixels_per_unit_y;
			
			view_start_x += update_iter.GetX();
			view_start_y += update_iter.GetY();
			view_end_x = view_start_x + update_iter.GetW();
			view_end_y = view_start_y + update_iter.GetH();

		    ++update_iter;
			
			double latitude1, longitude1;
			double latitude2, longitude2;
			
			_gps_draw->reproject(view_start_x, view_start_y, latitude1, longitude1);
			_gps_draw->reproject(view_end_x, view_end_y, latitude2, longitude2);
			
			double min_latitude = (latitude1 < latitude2 ? latitude1 : latitude2);
			double min_longitude = (longitude1 < longitude2 ? longitude1 : longitude2);
			double max_latitude = (latitude1 < latitude2 ? latitude2 : latitude1);
			double max_longitude = (longitude1 < longitude2 ? longitude2 : longitude1);
	
			_gps_draw->reproject(view_start_x, view_start_y, max_latitude, min_longitude);
			_gps_draw->reproject(view_end_x, view_end_y, min_latitude, max_longitude);
				
			for (int y=(((int)min_longitude)/10-1)*10; y<=(int)max_longitude+10; y+=10)
				for (int x=(((int)min_latitude)/10-1)*10; x<=(int)max_latitude+10; x+=10)
				{
					_gps_draw->line((double)x, (double)y, (double)x+10, (double)y);
					_gps_draw->line((double)x, (double)y, (double)x, (double)y+10);
				}
			
				
			unsigned int min_tile_id = 
				GeoCoordinate::get_tile_id(min_latitude, min_longitude);
			unsigned int max_tile_id =
				GeoCoordinate::get_tile_id(max_latitude, max_longitude);
			
			int min_tile_id_northing, min_tile_id_easting;
			int max_tile_id_northing, max_tile_id_easting;
			GeoCoordinate::split_tile_id(min_tile_id, min_tile_id_northing, min_tile_id_easting);
			GeoCoordinate::split_tile_id(max_tile_id, max_tile_id_northing, max_tile_id_easting);
				
			tiles_to_display = 
				((long)(max_tile_id_northing - min_tile_id_northing)) * 
				((long)(max_tile_id_easting - min_tile_id_easting));
	
			if (tiles_to_display <= 1296000L)
			{
				dc.SetPen(*wxThePenList->FindOrCreatePen(wxColour(150, 150, 150), 1, wxSOLID));
				for (int y=min_tile_id_northing; y<=max_tile_id_northing; y+=1)
					for (int x=min_tile_id_easting; x<=max_tile_id_easting; x+=1)
					{
						if (_used_tile_blocks_50[x/50].test(y/50))
						{
							if (_used_tile_blocks_5[x/5].test(y/5))
							{
								unsigned int tile_id = GeoCoordinate::merge_tile_id_parts(y, x);
								
								TileCache::Pointer tile_pointer;
								if (_use_prefetch)
									tile_pointer = 
										_tile_cache->get_or_prefetch(tile_id, 
															&_tile_prefetch_proxy);
								else
									tile_pointer = _tile_cache->get(tile_id);
								if (tile_pointer != 0)
								{
									MapGenerationDraw::tile_border(_gps_draw, tile_id);
									draw_tile(tile_id, 
										min_tile_id_northing, min_tile_id_easting, 
										max_tile_id_northing, max_tile_id_easting);
								}
							} else
							{
								x = (x/5+1) * 5 - 1;
							}
						} else
						{
							x = (x/50+1) * 50 - 1;
						}
					}
			}
			else if (tiles_to_display <= 6480000L)
			{
				dc.SetPen(*wxThePenList->FindOrCreatePen(wxColour(0, 0, 0), 1, wxSOLID));
				for (int y=(min_tile_id_northing/5)*5; y<=max_tile_id_northing; y+=5)
					for (int x=(min_tile_id_easting/5)*5; x<=max_tile_id_easting; x+=5)
					{
						if (_used_tile_blocks_50[x/50].test(y/50))
						{
							if (_used_tile_blocks_5[x/5].test(y/5))
							{
								unsigned int tile_id = GeoCoordinate::merge_tile_id_parts(y, x);
								MapGenerationDraw::tile_border(_gps_draw, tile_id, 5);
							}
						} else
						{
							x = (x/50+1) * 50 - 5;
						}
					}
			}
			else // if (tiles_to_display <= 648000000L)
			{
				dc.SetPen(*wxThePenList->FindOrCreatePen(wxColour(0, 0, 0), 1, wxSOLID));
				for (int y=(min_tile_id_northing/50)*50; y<=max_tile_id_northing; y+=50)
					for (int x=(min_tile_id_easting/50)*50; x<=max_tile_id_easting; x+=50)
					{
						if (_used_tile_blocks_50[x/50].test(y/50))
						{
							unsigned int tile_id = GeoCoordinate::merge_tile_id_parts(y, x);
							MapGenerationDraw::tile_border(_gps_draw, tile_id, 50);
						}
					}
			}
			
		}

		_use_prefetch = true;
	}
	
	
	void
	MapDrawer::reload()
	{
		update_used_tile_blocks();
	}

	
	void
	MapDrawer::set_tile_cache(TileCache* tile_cache)
	{
		_tile_cache = tile_cache;
		update_used_tile_blocks();
	}


	void
	MapDrawer::tile_prefetched(unsigned int id)
	{
/*		TileCache::Pointer tile_pointer=_tile_cache->get(id);
		if (tile_pointer != 0)
		{
			std::vector<unsigned int> edge_ids_on_tile = tile_pointer->get_edge_ids();
			std::vector <unsigned int>::iterator edge_ids_iter = 
				edge_ids_on_tile.begin();
			std::vector <unsigned int>::iterator edge_ids_iter_end = 
				edge_ids_on_tile.end();
			for (; edge_ids_iter != edge_ids_iter_end; ++edge_ids_iter)
				_edge_cache->prefetch(*edge_ids_iter, &_edge_prefetch_proxy);
		}
		
		if (time(0) > _last_paint_seconds)
		{
			_use_prefetch = false;
			_map_scrolled_window->Refresh();
		}*/
	}
	
	
	void
	MapDrawer::update_used_tile_blocks()
	{	
		std::vector< std::bitset<3600> >::iterator
			used_blocks_5_iter = _used_tile_blocks_5.begin();
		std::vector< std::bitset<3600> >::iterator
			used_blocks_5_iter_end = _used_tile_blocks_5.end();
		for (; used_blocks_5_iter!=used_blocks_5_iter_end; ++used_blocks_5_iter)
			used_blocks_5_iter->reset();
			
		std::vector< std::bitset<360> >::iterator
			used_blocks_50_iter = _used_tile_blocks_50.begin();
		std::vector< std::bitset<360> >::iterator
			used_blocks_50_iter_end = _used_tile_blocks_50.end();
		for (; used_blocks_50_iter!=used_blocks_50_iter_end; 
			++used_blocks_50_iter)
			used_blocks_50_iter->reset();
		
		if (_tile_cache == 0) return;
		
		std::vector<unsigned int> used_tile_ids = _tile_cache->get_used_ids();
		std::vector<unsigned int>::iterator iter = used_tile_ids.begin();
		std::vector<unsigned int>::iterator iter_end = used_tile_ids.end();
		std::sort(iter, iter_end);
		for (; iter!=iter_end; ++iter)
		{
			int tile_id_northing, tile_id_easting;
			GeoCoordinate::split_tile_id
				(*iter, tile_id_northing, tile_id_easting);
			_used_tile_blocks_5[tile_id_easting/5].set(tile_id_northing/5);
			_used_tile_blocks_50[tile_id_easting/50].set(tile_id_northing/50);
		}
	}
	
} // namespace mapgeneration_gui

