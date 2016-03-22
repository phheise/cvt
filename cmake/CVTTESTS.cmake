MACRO(CVTTESTS _LIB _OUT )
	IF(UNIX)
		IF(NOT APPLE)
			# linux
			SET(LIBNAME "lib/lib${_LIB}_d.so")
		ELSE()
			# mac os xcode
			IF( ${CMAKE_GENERATOR} MATCHES "Xcode" )
				SET(LIBNAME "lib/${_LIB}_d.dylib")
			ELSE()
				SET(LIBNAME "lib/lib${_LIB}_d.dylib")
			ENDIF()
		ENDIF()
	ELSE()
		# windows
	ENDIF()

    ADD_CUSTOM_COMMAND(
        OUTPUT  ${CMAKE_BINARY_DIR}/${_OUT}
        COMMAND ${CMAKE_SOURCE_DIR}/util/extracttests.sh
                ${CMAKE_BINARY_DIR}/${LIBNAME}
                ${CMAKE_BINARY_DIR}/${_OUT}
        DEPENDS ${_LIB}
    )
    ADD_CUSTOM_TARGET( cvt_test_extraction DEPENDS ${CMAKE_BINARY_DIR}/${_OUT} )
ENDMACRO(CVTTESTS)

