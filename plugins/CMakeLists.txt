INCLUDE( MacroPlugin )

# create a custom target (for use as dependency for other targets)
ADD_CUSTOM_TARGET( Plugins )

# required libs
FIND_PACKAGE( Eigen3 REQUIRED QUIET )
FIND_PACKAGE( OpenCL REQUIRED QUIET )
FIND_PACKAGE( OpenGL REQUIRED QUIET )

# add the plugin folders
ADD_SUBDIRECTORY(ifilter)
ADD_SUBDIRECTORY(iloader)
ADD_SUBDIRECTORY(isaver)
ADD_SUBDIRECTORY(sceneloader)
