#  Eigen3_INCLUDE_DIR, where to find the Eigen folder
#  Eigen3_FOUND, If false, do not try to use GLUT.

FIND_PATH( Eigen3_INCLUDE_DIR Eigen/Core
		/usr/include/eigen3
		/usr/local/include/eigen3
		/opt/local/include/eigen3
	 )

INCLUDE( LibFindMacros )
# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set( Eigen3_PROCESS_INCLUDES Eigen3_INCLUDE_DIR )
#set( Eigen3_PROCESS_LIBS Eigen3_LIBRARY )
libfind_process( Eigen3 )

