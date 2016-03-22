# - Find OpenCL
#  This module attempts to find a valid OpenCL SDK installation.
#  Currently only the AMD and NVidia OpenCL SDKS are supported.
#
#  Afterwards, the following variables will be set
#
#  OpenCL_FOUND        - system has OpenCL installed
#  OpenCL_INCLUDE_DIR  - the OpenCL include directory
#  OpenCL_LIBRARIES    - link these to use OpenCL
#
#  Note this module does not support searching for a particular 
#  OpenCL SDK version

INCLUDE( LibFindMacros )
IF (WIN32)
	FIND_PATH(OpenCL_INCLUDE_DIR 
		CL/cl.h 
	)

	# TODO this is only a hack assuming the 64 bit library will
	# not be found on 32 bit system
	FIND_LIBRARY(OpenCL_LIBRARY 
		opencl64
	)
	
	IF( OpenCL_LIBRARY )
		FIND_LIBRARY(OpenCL_LIBRARY 
			opencl32 
		)
	ENDIF( OpenCL_LIBRARY )

ELSE (WIN32)
	IF(APPLE)
		FIND_PATH(OpenCL_INCLUDE_DIR
			NAMES
			opencl.h
			PATHS
			/System/Library/Frameworks/OpenCL.framework/Headers
		)
        FIND_LIBRARY( OpenCL_LIBRARY OpenCL doc "OpenCL Framework on mac os" )
	ELSE(APPLE)
		# Unix style platforms

		#Current NVIDIA Cuda SDKs provide the opencl headers on linux
		SET(_NVIDIA_SDK_VERSIONS 5.0 5.5)
		SET(_NVIDIA_SDK_DIRS /usr/local/cuda)
		FOREACH(_NVIDIA_SDK_VERSION ${_NVIDIA_SDK_VERSIONS})
		    SET(_NVIDIA_SDK_DIRS ${_NVIDIA_SDK_DIRS} /usr/local/cuda-${_NVIDIA_SDK_VERSION})
	    ENDFOREACH(_NVIDIA_SDK_VERSION)

		FIND_PATH(OpenCL_INCLUDE_DIR
	        NAMES
                CL/opencl.h
            PATHS
                ${_NVIDIA_SDK_DIRS}
			    ~/NVIDIA_GPU_Computing_SDK/OpenCL/common
			    /usr/local/nvgpu/OpenCL/common
			    /opt/AMDAPP
			    $ENV{ATISTREAMSDKROOT}
	        PATH_SUFFIXES
	            include inc
		)

		# AMD Stream SDK, (should be updated to APPSDK) default location
		FIND_LIBRARY(OpenCL_LIBRARY 
			OpenCL 
            PATHS
			$ENV{LD_LIBRARY_PATH}
			/opt/AMDAPP/lib/x86
		)
	ENDIF (APPLE)
ENDIF (WIN32)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set( OpenCL_PROCESS_INCLUDES OpenCL_INCLUDE_DIR )
set( OpenCL_PROCESS_LIBS OpenCL_LIBRARY )
libfind_process( OpenCL )
