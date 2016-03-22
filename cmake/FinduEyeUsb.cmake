# On success this script defines
# UEYEUSB_INCLUDE_DIR           location of ueye.h
# UEYEUSB_LIBRARY               location of libueye_api library
# UEYEUSB_FOUND					whether the library has been found or not 

FIND_PATH(UEYEUSB_INCLUDE_DIR  ueye.h
	${CMAKE_INCLUDE_PATH}
	$ENV{HOME}/include/
	/usr/include/opencv
	$ENV{ProgramFiles}/IDS/ueye/Develop/include
)

FIND_LIBRARY(UEYEUSB_LIBRARY
	NAMES
	libueye_api.so
	PATHS
	${CMAKE_LIBRARY_PATH}
	/usr/lib
	$ENV{ProgramFiles}/IDS/ueye/Develop/lib
)

INCLUDE( LibFindMacros )
# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this lib depends on.
set( UEYEUSB_PROCESS_INCLUDES UEYEUSB_INCLUDE_DIR )
set( UEYEUSB_PROCESS_LIBS UEYEUSB_LIBRARY )
libfind_process( UEYEUSB )

