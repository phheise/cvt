#  Eigen2_INCLUDE_DIR, where to find the Eigen folder
#  Eigen2_FOUND, If false, do not try to use GLUT.

FIND_PATH( Eigen2_INCLUDE_DIR Eigen/Core
		/usr/include/eigen2
		/usr/local/include/eigen2
		/opt/local/include/eigen2
	 )

INCLUDE( LibFindMacros )
# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set( Eigen2_PROCESS_INCLUDES Eigen2_INCLUDE_DIR )
#set( Eigen2_PROCESS_LIBS Eigen2_LIBRARY )
libfind_process( Eigen2 )
