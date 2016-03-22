# (C) 2013, Sebastian Klose, <kloses@in.tum.de>

#	this automatically generates a target executable, using all 
#	.cpp, .c, .hpp and .h files of the current directory
#	The name of the target will be either the one passed as argument,
#	or (if none is passed) the name of the folder
#
#	Dependencies we search for and link against:
#	- Eigen3

MACRO ( MACRO_TEST_APPLICATION ADDITIONAL_LIBS )

include( GLCLTOHEADER )

SET(DATA_FOLDER	"${CMAKE_SOURCE_DIR}/data")
ADD_DEFINITIONS(-DDATA_FOLDER="${DATA_FOLDER}")

GET_FILENAME_COMPONENT(folderName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
SET(APP_NAME "${folderName}")

MESSAGE(STATUS "Configuring: ${APP_NAME}") 

# get all headers and cpp files in the folder:
FILE(GLOB SRCS_${APP_NAME} "*.cpp" "*.h" "*.hpp" "*.mm")
FILE(GLOB CLSRCS_${APP_NAME} "*.cl")
FILE(GLOB GLSLSRCS_${APP_NAME} "*.vert" "*.frag" )

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

ADD_EXECUTABLE(${APP_NAME} ${SRCS_${APP_NAME}} ${CLHEADER_${APP_NAME}} )

INCLUDE_DIRECTORIES( 
	"${CMAKE_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}" # needed for files generated in build tree
	"${Eigen3_INCLUDE_DIR}"
	"${OpenCL_INCLUDE_DIR}"
)

IF( UNIX AND NOT APPLE )
    SET( ADDITIONAL_LIBS ${ADDITIONAL_LIBS} rt )
ENDIF()

TARGET_LINK_LIBRARIES( ${APP_NAME} cvt ${ADDITIONAL_LIBS} )
ADD_DEPENDENCIES( ${APP_NAME} cvt ${CMAKE_CURRENT_SOURCE_DIR} ${GENDEPS_${APP_NAME}} Plugins )

SET_TARGET_PROPERTIES("${APP_NAME}" PROPERTIES DEBUG_POSTFIX "_d")

IF( ${CMAKE_GENERATOR} MATCHES "Xcode" )
	SET_TARGET_PROPERTIES("${APP_NAME}" PROPERTIES PREFIX "../")
ENDIF()

ENDMACRO (MACRO_TEST_APPLICATION)
