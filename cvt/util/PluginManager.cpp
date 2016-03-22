/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <cvt/util/PluginManager.h>
#include <cvt/util/Exception.h>
#include <cvt/io/FileSystem.h>
#include <cvt/util/String.h>
#include <cvt/util/Util.h>
#include <vector>

namespace cvt {
	PluginManager* PluginManager::_instance = NULL;

	PluginManager& PluginManager::instance()
	{
		if( !_instance ) {
			_instance = new PluginManager();
			_instance->loadDefault();
		}
		return *_instance;
	}

	PluginManager::PluginManager()
	{
		// add folders to plugin path vector:
		if( FileSystem::exists( PLUGIN_PATH ) ){
			_pluginPaths.push_back( PLUGIN_PATH );
		}

		if( FileSystem::exists( "/usr/share/cvt/plugins" ) ){
			_pluginPaths.push_back( "/usr/share/cvt/plugins/" );
		}

		String envVar;
		if( Util::getEnv( envVar, "CVT_PLUGIN_PATH" ) ){
			_pluginPaths.push_back( envVar );
		}
	}

	void PluginManager::loadDefault()
	{
		// Static default plugins

		// Runtime loaded plugins
		std::vector<String> entries;
		for( size_t f = 0; f < _pluginPaths.size(); f++ ){
			FileSystem::ls( _pluginPaths[ f ], entries );
			for( std::vector<String>::iterator it = entries.begin(), end = entries.end(); it != end; ++it ) {
				String path( _pluginPaths[ f ] );
				try {
					path += *it;
					//std::cout << path << std::endl;
					loadPlugin( path.c_str() );
				} catch( Exception e ) {
					std::cout << "Could not load plugin at path: " << path << std::endl;
				}
			}
		}
	}

	void PluginManager::cleanup()
	{
		if( _instance )
			delete _instance;
	}

}
