FIND_PATH(LIBDC1394_INCLUDE_DIR 
	NAMES dc1394/dc1394.h
	PATHS
	${CMAKE_INCLUDE_PATH}
	/usr/local/dc1394/include	
	/usr/local/include
	/usr/include
	/opt/local/include
)

FIND_LIBRARY(LIBDC1394_LIBRARY
	NAMES dc1394
	PATHS
	${CMAKE_LIBRARY_PATH}
	/usr/local/dc1394/lib
	/usr/local/lib
	/usr/lib
	/opt/local/lib
)

INCLUDE( LibFindMacros )
# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set( DC1394_PROCESS_INCLUDES LIBDC1394_INCLUDE_DIR )
set( DC1394_PROCESS_LIBS LIBDC1394_LIBRARY )
libfind_process( DC1394 )
