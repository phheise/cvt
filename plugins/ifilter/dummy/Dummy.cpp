#include <cvt/util/Plugin.h>
#include <iostream>

static void _init( void* )
{
	std::cout <<  "Hello - I'm the dummy plugin" << std::endl;
}

extern "C" {
	cvt::PluginInfo _cvtplugin = { 0x43565450, 0, 1, ( void ( * )( cvt::PluginManager* ) ) _init };
}
