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

#ifndef CVT_PLUGINMANAGER_H
#define CVT_PLUGINMANAGER_H

#include <cvt/util/Plugin.h>
#include <cvt/util/PluginFile.h>
#include <cvt/gfx/IFilter.h>
#include <cvt/gfx/ILoader.h>
#include <cvt/gfx/ISaver.h>
#include <cvt/geom/scene/SceneLoader.h>
#include <cvt/util/String.h>
#include <vector>
#include <map>

namespace cvt {
	class Application;

	class PluginManager {
		friend class Application;
		public:
			static PluginManager& instance();
			void registerPlugin( Plugin* plugin );
			void loadPlugin( const String& path );

			IFilter* getIFilter( size_t i ) const;
			IFilter* getIFilter( const String& name ) const;
			size_t getIFilterSize() const;

			ILoader* getILoaderForFilename( const String& name );
			ISaver* getISaverForFilename( const String& name );

			SceneLoader* getSceneLoaderForFilename( const String& name );

		private:
			PluginManager();
			PluginManager( const PluginManager& );
			~PluginManager();
			void loadDefault();
			static void cleanup();

			std::vector<PluginFile*> _plugins;
			std::vector<IFilter*> _ifilters;
			std::vector<ILoader*> _iloaders;
			std::vector<ISaver*> _isavers;
			std::vector<SceneLoader*> _sceneloaders;
			std::map< const String, IFilter*> _ifiltermap;

			std::vector<String>	  _pluginPaths;

			static PluginManager* _instance;
	};


	inline PluginManager::~PluginManager()
	{
		for( std::vector<IFilter*>::iterator it = _ifilters.begin(), end = _ifilters.end(); it != end; ++it  )
			delete *it;
		_ifilters.clear();

		for( std::vector<ILoader*>::iterator it = _iloaders.begin(), end = _iloaders.end(); it != end; ++it  )
			delete *it;
		_iloaders.clear();

		for( std::vector<ISaver*>::iterator it = _isavers.begin(), end = _isavers.end(); it != end; ++it  )
			delete *it;
		_isavers.clear();

		for( std::vector<SceneLoader*>::iterator it = _sceneloaders.begin(), end = _sceneloaders.end(); it != end; ++it  )
			delete *it;
		_sceneloaders.clear();

		for( std::vector<PluginFile*>::iterator it = _plugins.begin(), end = _plugins.end(); it != end; ++it  )
			delete *it;
		_plugins.clear();

		_ifiltermap.clear();
	}

	inline void PluginManager::registerPlugin( Plugin* plugin )
	{
		if( !plugin )
			return;
		switch( plugin->pluginType() ) {
			case PLUGIN_IFILTER:
				{
					_ifilters.push_back( ( IFilter* ) plugin );
					_ifiltermap[ plugin->name() ] = ( IFilter* ) plugin;
				}
				break;
			case PLUGIN_ILOADER:
				{
					//std::cout << "Loaded ILoader " << plugin->name() << " Extension:" << ( ( ILoader* ) plugin )->extension( 0 ) << std::endl;
					_iloaders.push_back( ( ILoader* ) plugin );
				}
				break;
			case PLUGIN_ISAVER:
				{
					//std::cout << "Loaded ISaver " << plugin->name() << " Extension:" << ( ( ISaver* ) plugin )->extension( 0 ) << std::endl;
					_isavers.push_back( ( ISaver* ) plugin );
				}
				break;
			case PLUGIN_SCENELOADER:
				{
					_sceneloaders.push_back( ( SceneLoader* ) plugin );
				}
				break;
			default:
				break;
		}
	}

	inline void PluginManager::loadPlugin( const String& path )
	{
		PluginFile* p = new PluginFile( path.c_str() );
		p->load();
		_plugins.push_back( p );
	}


	inline IFilter* PluginManager::getIFilter( size_t n ) const
	{
		return _ifilters[ n ];
	}

	inline IFilter* PluginManager::getIFilter( const String& name ) const
	{
		std::map< const String, IFilter*>::const_iterator it;
		if( ( it = _ifiltermap.find( name ) ) != _ifiltermap.end() ) {
			return it->second;
		}
		return NULL;
	}

	inline size_t PluginManager::getIFilterSize() const
	{
		return _ifilters.size();
	}

	inline ILoader* PluginManager::getILoaderForFilename( const String& name )
	{
		for( std::vector<ILoader*>::iterator it = _iloaders.begin(), end = _iloaders.end(); it != end; ++it  )
		{
			for( size_t i = 0, end = ( *it )->sizeExtensions(); i < end; i++ ) {
				if( name.hasSuffix( ( *it )->extension( i ) ) )
					return *it;
			}
		}
		return NULL;
	}

	inline ISaver* PluginManager::getISaverForFilename( const String& name )
	{
		for( std::vector<ISaver*>::iterator it = _isavers.begin(), end = _isavers.end(); it != end; ++it  ) {
			for( size_t i = 0, end = ( *it )->sizeExtensions(); i < end; i++ ) {
				if( name.hasSuffix( ( *it )->extension( i ) ) )
					return *it;
			}
		}
		return NULL;
	}

	inline SceneLoader* PluginManager::getSceneLoaderForFilename( const String& name )
	{
		for( std::vector<SceneLoader*>::iterator it = _sceneloaders.begin(), end = _sceneloaders.end(); it != end; ++it  )
		{
			for( size_t i = 0, end = ( *it )->sizeExtensions(); i < end; i++ ) {
				if( name.hasSuffix( ( *it )->extension( i ) ) )
					return *it;
			}
		}
		return NULL;
	}

}

#endif
