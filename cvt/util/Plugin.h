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

#ifndef CVT_PLUGIN_H
#define CVT_PLUGIN_H

#include <stdint.h>
#include <cvt/util/String.h>
#include <cvt/util/Version.h>

namespace cvt {
	class PluginManager;

	struct PluginInfo {
		uint32_t magic;
		uint32_t major;
		uint32_t minor;
		void (*init)( PluginManager* manager );
	};

#define CVT_PLUGIN_MAGIC 0x43565450
#define CVT_PLUGIN_MAJOR CVT_VERSION_MAJOR
#define CVT_PLUGIN_MINOR CVT_VERSION_MINOR

#define CVT_PLUGIN( initfunc ) \
	extern "C" { \
		cvt::PluginInfo _cvtplugin = { CVT_PLUGIN_MAGIC, CVT_PLUGIN_MAJOR , CVT_PLUGIN_MINOR, ( void ( * )( cvt::PluginManager* ) ) initfunc }; \
	}

	enum PluginType {
		PLUGIN_ILOADER,
		PLUGIN_ISAVER,
		PLUGIN_IFILTER,
		PLUGIN_SCENELOADER,
		PLUGIN_SCENESAVER
	};

	class Plugin {
		public:
			Plugin( PluginType type );
			virtual ~Plugin();
			virtual const String& name() const = 0;
			PluginType pluginType() const;

		private:
			PluginType  _type;
	};

	inline Plugin::Plugin( PluginType type ) : _type( type )
	{
	}

	inline Plugin::~Plugin()
	{
	}

	inline PluginType Plugin::pluginType() const
	{
		return _type;
	}

}

#endif
