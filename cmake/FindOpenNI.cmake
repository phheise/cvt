FIND_PATH(OpenNI_INCLUDE_DIR 
	NAMES XnOpenNI.h
	PATHS
	${CMAKE_INCLUDE_PATH}
	/usr/local/dc1394/include/ni
	/usr/local/include/ni
	/usr/include/ni
    /usr/include/openni
	/opt/local/include/ni
)

FIND_LIBRARY(OpenNI_LIBRARY
	NAMES OpenNI
	PATHS
	${CMAKE_LIBRARY_PATH}	
	/usr/local/lib
	/usr/lib
	/opt/local/lib
)


IF( OpenNI_LIBRARY AND OpenNI_INCLUDE_DIR )
	FIND_LIBRARY( OPENNI_PRIME_SENSOR_MODULE 
		NAMES XnCore
		PATHS
		${CMAKE_LIBRARY_PATH}		
		/usr/local/lib
		/usr/lib
		/opt/local/lib
	)
ENDIF()

INCLUDE( LibFindMacros )
# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this lib depends on.
set( OpenNI_PROCESS_INCLUDES OpenNI_INCLUDE_DIR )
set( OpenNI_PROCESS_LIBS OpenNI_LIBRARY OPENNI_PRIME_SENSOR_MODULE )
libfind_process( OpenNI )

#MESSAGE( "ONI LIBS: ${OpenNI_LIBRARIES}" )