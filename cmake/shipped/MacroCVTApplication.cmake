# (C) 2010, Sebastian Klose, <kloses@in.tum.de>

#	this automatically generates a target executable, using all 
#	.cpp, .c, .hpp and .h files of the current directory
#	The name of the target will be either the one passed as argument,
#	or (if none is passed) the name of the folder
#
#	Furthermore, it also generates the header files out of all CL files
#
#	Dependencies we search for and link against:
#	- Eigen3
#	- OpenCL

MACRO ( CVT_APPLICATION ADDITIONAL_LIBS )

include( GLCLTOHEADER )

GET_FILENAME_COMPONENT(folderName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
SET( APP_NAME "${folderName}" )

MESSAGE(STATUS "Configuring: ${APP_NAME}") 

# get all headers and cpp files in the folder:
FILE(GLOB SRCS_${APP_NAME} "*.cpp" "*.h" "*.hpp" "*.mm")
FILE(GLOB CLSRCS_${APP_NAME} "*.cl")
FILE(GLOB GLSLSRCS_${APP_NAME} "*.vert" "*.frag" )

SET( GENDEPS_${APP_NAME} "" )

FOREACH (CLSOURCE ${CLSRCS_${APP_NAME}})
	GET_FILENAME_COMPONENT( CLFILE ${CLSOURCE} NAME )
	CLTOHEADER(${CLFILE})

	# add it to the generated cl headers
	SET( CLHEADER_${APP_NAME} ${CLHEADER_${APP_NAME}} "${CL_GEN_OUTPUT}" )

	# add the dependency to this generated file
	SET( GENDEPS_${APP_NAME} ${GENDEPS_${APP_NAME}} "CLGEN_${CLFILE}" )
ENDFOREACH(CLSOURCE)

FOREACH (GLSLSOURCE ${GLSLSRCS_${APP_NAME}})
	GET_FILENAME_COMPONENT( GLSLFILE ${GLSLSOURCE} NAME )
	GLSLTOHEADER(${GLSLFILE})
	SET( GLSLHEADER_${APP_NAME} ${GLSLHEADER_${APP_NAME}} "${GLSL_GEN_OUTPUT}" )
	SET( GENDEPS_${APP_NAME} ${GENDEPS_${APP_NAME}} "GLSLGEN_${GLSLFILE}" )
ENDFOREACH(GLSLSOURCE)

INCLUDE_DIRECTORIES( 
	"${CMAKE_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

ADD_EXECUTABLE(${APP_NAME} ${SRCS_${APP_NAME}} ${CLHEADER_${APP_NAME}} ${GLSLHEADER_${APP_NAME}} )
TARGET_LINK_LIBRARIES( ${APP_NAME} ${CVT_LIBRARIES} ${ADDITIONAL_LIBS} )

IF( GENDEPS_${APP_NAME} )
	ADD_DEPENDENCIES( ${APP_NAME} ${GENDEPS_${APP_NAME}} )
ENDIF()

SET_TARGET_PROPERTIES("${APP_NAME}" PROPERTIES DEBUG_POSTFIX "_d")

IF( ${CMAKE_GENERATOR} MATCHES "Xcode" )
	SET_TARGET_PROPERTIES("${APP_NAME}" PROPERTIES PREFIX "../")
ENDIF()

ENDMACRO( CVT_APPLICATION )
