SET(CMAKE_CONFIGURATION_TYPES Debug Release Atom zeroopt)

# set the default build type if needed
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "debug" CACHE STRING "Choose build type (Release, Debug or zeroopt)" FORCE)
	MESSAGE(STATUS "BUILD_TYPE is unset, defaulting to \"${CMAKE_BUILD_TYPE}\"")
ENDIF(NOT CMAKE_BUILD_TYPE)

string( TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE )

SET(BUILD_SHARED_LIBS TRUE)
