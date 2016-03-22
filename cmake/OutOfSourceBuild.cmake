# require out of source building
IF(CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "In-source build is not supported. Please build out-of-source.")
ENDIF(CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
