FIND_PATH(OpenNI2_INCLUDE_DIR 
	NAMES OpenNI.h
	PATHS
	${CMAKE_INCLUDE_PATH}
	/usr/local/include
	/usr/local/include/openni2
	/usr/include
	/usr/include/openni2
    $ENV{OPENNI2_INCLUDE}
)

FIND_LIBRARY(OpenNI2_LIBRARY
	NAMES OpenNI2
	PATHS
	${CMAKE_LIBRARY_PATH}	
	/usr/local/lib
	/usr/lib
	/opt/local/lib
    $ENV{OPENNI2_REDIST}
)

INCLUDE( LibFindMacros )
# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this lib depends on.
set( OpenNI2_PROCESS_INCLUDES OpenNI2_INCLUDE_DIR )
set( OpenNI2_PROCESS_LIBS OpenNI2_LIBRARY )
libfind_process( OpenNI2 )
