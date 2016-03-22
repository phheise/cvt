FIND_FILE( GLCLTOHEADER_DST
	glcltoheader
	/usr/local/share/cvt/utils
	/usr/share/cvt/utils
)

MACRO(GLSLTOHEADER _filename )
    GET_FILENAME_COMPONENT(_basename ${_filename} NAME_WE)
    GET_FILENAME_COMPONENT(_ext ${_filename} EXT )
    GET_FILENAME_COMPONENT(_path ${_filename} PATH )

	string(REGEX REPLACE "\\.(.*)" "\\1" _ext ${_ext} )

	STRING( STRIP "${_path}" _path )
	IF( NOT "${_path}" STREQUAL "" )       
		SET(_path "${_path}/" )
    ENDIF()
	
	SET( GLSL_GEN_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${_path}${_basename}_${_ext}.h" )
    ADD_CUSTOM_COMMAND(
		OUTPUT  "${GLSL_GEN_OUTPUT}"
		COMMAND "${GLCLTOHEADER_DST}" 
				"${CMAKE_CURRENT_SOURCE_DIR}/${_filename}"
				"${CMAKE_CURRENT_SOURCE_DIR}/${_path}${_basename}_${_ext}.h" 
				"${_basename}_${_ext}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}" 
    )
	ADD_CUSTOM_TARGET( GLSLGEN_${_filename} DEPENDS ${GLSL_GEN_OUTPUT} )
ENDMACRO(GLSLTOHEADER)

MACRO(CLTOHEADER _filename)
    GET_FILENAME_COMPONENT(_basename ${_filename} NAME_WE)
    GET_FILENAME_COMPONENT(_path ${_filename} PATH )

	STRING( STRIP "${_path}" _path )
	IF( NOT "${_path}" STREQUAL "" )       
		SET(_path "${_path}/" )
    ENDIF()

	SET( CL_GEN_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${_path}${_basename}.h" )
    ADD_CUSTOM_COMMAND(
		OUTPUT  "${CL_GEN_OUTPUT}"		
		COMMAND "${GLCLTOHEADER_DST}" 
                "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}"
				"${CMAKE_CURRENT_SOURCE_DIR}/${_path}${_basename}.h"
				"${_basename}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}" 
    )
	ADD_CUSTOM_TARGET( CLGEN_${_filename} DEPENDS ${CL_GEN_OUTPUT} )
ENDMACRO(CLTOHEADER)


