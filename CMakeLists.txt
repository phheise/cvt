PROJECT(CVTools CXX C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)
cmake_policy(SET CMP0012 NEW)

# OS X RPATH policy onlv available from 3.0 onwards
IF(CMAKE_MAJOR_VERSION EQUAL 3)
    cmake_policy(SET CMP0042 OLD)
ENDIF()

SET( CVT_VERSION_MAJOR 0 )
SET( CVT_VERSION_MINOR 1 )
SET( CVT_VERSION_PATCH 1 )

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
INCLUDE(OutOfSourceBuild)
INCLUDE(BuildTypes)
INCLUDE(CompilerFlags)
INCLUDE(CompileDefinitions)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# tool to generate header files out of shader or cl sources
add_executable( glcltoheader util/glcltoheader.cpp )
SET( CVT_GLCLTOHEADER ${PROJECT_BINARY_DIR}/bin/glcltoheader )
INSTALL( TARGETS "glcltoheader" RUNTIME DESTINATION "share/cvt/utils" COMPONENT utils )

# include build directory for the generated headers!
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

ADD_SUBDIRECTORY(cvt)
ADD_SUBDIRECTORY(plugins)

IF( ${CMAKE_GENERATOR} MATCHES "Xcode" )
	SET_TARGET_PROPERTIES(glcltoheader PROPERTIES PREFIX "../")	
ENDIF()

ADD_SUBDIRECTORY( package )
