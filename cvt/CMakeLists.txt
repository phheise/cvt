# add source, current source and binary dir to search for includes
INCLUDE_DIRECTORIES( 
	${CMAKE_SOURCE_DIR} 
        ${CMAKE_CURRENT_SOURCE_DIR} )

# build type should be set
IF(${CMAKE_BUILD_TYPE})
	MESSAGE(FATAL_ERROR "Build type is empty.")
ENDIF(${CMAKE_BUILD_TYPE})

# put libs/archives into lib folder and executables into bin
SET(EXECUTABLE_OUTPUT_PATH		"${CMAKE_BINARY_DIR}/bin")
SET(RUNTIME_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH			"${CMAKE_BINARY_DIR}/lib")
SET(ARCHIVE_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/lib")
SET(LIBRARY_OUTPUT_DIRECTORY	"${CMAKE_BINARY_DIR}/lib")

INCLUDE(GLCLTOHEADER)
INCLUDE(CVTTESTS)

# TODO: this should be part of the findscript?
SET( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/lib/nvidia-current" )

FIND_PACKAGE(Doxygen)
if(DOXYGEN_FOUND)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxyfile.in
   ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
   add_custom_target(doc
   ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   COMMENT "Generating API documentation with Doxygen" VERBATIM
   )
endif(DOXYGEN_FOUND)

# required dependencies
FIND_PACKAGE(Eigen3 REQUIRED QUIET)
FIND_PACKAGE(ffmpeg REQUIRED QUIET)
FIND_PACKAGE(OpenCL REQUIRED QUIET)

SET( CMAKE_SYSTEM_INCLUDE_PATH /usr/include/nvidia-current ${CMAKE_SYSTEM_INCLUDE_PATH} )
IF( UNIX AND NOT APPLE )
   FIND_PACKAGE(OpenGL REQUIRED)
   FIND_PACKAGE(X11 REQUIRED)
ENDIF()

# libs
SET(CVT_DEP_LIBRARIES
   ${LIBPNG_LIBRARY}
   ${OPENGL_LIBRARIES}
   ${OpenCL_LIBRARIES}
   ${X11_X11_LIB}
   ${ffmpeg_LIBRARIES}
)
IF( UNIX AND NOT APPLE )
	SET( CVT_DEP_LIBRARIES
        ${CVT_DEP_LIBRARIES}
		rt
		pthread
    )
ENDIF()

# configure the version file
CONFIGURE_FILE (
   "CVTVersion.h.in"
   "${CMAKE_CURRENT_BINARY_DIR}/util/Version.h"
)
ADD_CUSTOM_TARGET( gen_version_file DEPENDS CVTVersion.h.in )

SET( GLSL_SHADER
   gl/shader/120/basic_120.vert
   gl/shader/120/basic_120.frag
   gl/shader/120/text_120.vert
   gl/shader/120/text_120.frag
   gl/shader/120/icon_120.vert
   gl/shader/120/icon_120.frag
   gl/shader/120/basictex_120.vert
   gl/shader/120/basictex_120.frag
   gl/shader/120/basicrrtex_120.frag
   gl/shader/120/basicrr_120.frag
   gl/shader/120/basiclight_120.frag
   gl/shader/120/basiclight_120.vert
   gl/shader/120/texmls_120.frag
   gl/shader/120/texmls_120.vert
   gl/shader/150/basic_150.vert
   gl/shader/150/basic_150.frag
   gl/shader/150/text_150.vert
   gl/shader/150/text_150.frag
   gl/shader/150/icon_150.vert
   gl/shader/150/icon_150.frag
   gl/shader/150/basictex_150.vert
   gl/shader/150/basictex_150.frag
   gl/shader/150/basicrrtex_150.frag
   gl/shader/150/basicrr_150.frag
   gl/shader/150/basiclight_150.frag
   gl/shader/150/basiclight_150.vert
   gl/shader/150/basiclighttex_150.frag
   gl/shader/150/basiclighttex_150.vert
   gl/shader/150/texmls_150.frag
   gl/shader/150/texmls_150.vert
   gl/scene/shader/gls_default.frag
   gl/scene/shader/gls_default.vert
)

SET( CL_KERNELS
    cl/kernel/gaussiir.cl
    cl/kernel/gaussiir2.cl
    cl/kernel/BC.cl
    cl/kernel/clear.cl
    cl/kernel/clearbuffer.cl
    cl/kernel/fill.cl
    cl/kernel/imageutil.cl
    cl/kernel/gradx.cl
    cl/kernel/gradxy.cl
    cl/kernel/median3.cl
    cl/kernel/FlowColorCode.cl
    cl/kernel/fgp/fgp.cl
    cl/kernel/fgp/fgp_data.cl
    cl/kernel/fgp/fgpgrayweightedhuber.cl
    cl/kernel/fgp/fgpgrayweightedhuber_data.cl
    cl/kernel/boxfilter/boxfilter_prefixsum.cl
    cl/kernel/boxfilter/boxfilter.cl
    cl/kernel/prefixsum/prefixsum_pblock.cl
    cl/kernel/prefixsum/prefixsum_pblock_sqr.cl
    cl/kernel/prefixsum/prefixsum_pblock_mul2.cl
    cl/kernel/prefixsum/prefixsum_pblock_mul2_shifted.cl
    cl/kernel/prefixsum/prefixsum_pblock_outerrgb.cl
    cl/kernel/prefixsum/prefixsum_horiz.cl
    cl/kernel/prefixsum/prefixsum_vert.cl
    cl/kernel/prefixsum/prefixsum_block2.cl
    cl/kernel/guidedfilter/guidedfilter_calcab.cl
    cl/kernel/guidedfilter/guidedfilter_calcab_outerrgb.cl
    cl/kernel/guidedfilter/guidedfilter_applyab_gc.cl
    cl/kernel/guidedfilter/guidedfilter_applyab_gc_outer.cl
    cl/kernel/guidedfilter/guidedfilter_applyab_cc.cl
    cl/kernel/pyr/pyrdown_binom3.cl
    cl/kernel/pyr/pyrdown_binom5.cl
    cl/kernel/pyr/pyrup.cl
    cl/kernel/pyr/pyrupmul.cl
    cl/kernel/pyr/pyrdown.cl
    cl/kernel/tvl1flow/tvl1.cl
    cl/kernel/tvl1flow/tvl1_dataadd.cl
    cl/kernel/tvl1flow/tvl1_warp.cl
    cl/kernel/stereo/stereoCV.cl
    cl/kernel/stereo/stereoCVFBP.cl
    cl/kernel/stereo/stereoCVQSearch.cl
    cl/kernel/stereogcv/costdepth.cl
    cl/kernel/stereogcv/costdepthgrad.cl
    cl/kernel/stereogcv/costdepthncc.cl
    cl/kernel/stereogcv/costmin.cl
    cl/kernel/stereogcv/costdepthconv.cl
    cl/kernel/stereogcv/occlusioncheck.cl
    cl/kernel/tvl1stereo/tvl1.cl
    cl/kernel/tvl1stereo/tvl1_warp.cl
    cl/kernel/TSDFVolume/TSDFVolume.cl
	cl/kernel/PDROF.cl
	cl/kernel/PDHuber.cl
	cl/kernel/PDHuberWeighted.cl
	cl/kernel/PDHuberWeightedInpaint.cl
	cl/kernel/pmhstereo.cl
	cl/kernel/MC.cl
)

FOREACH( CLFILE ${CL_KERNELS} )
    CLTOHEADER( ${CLFILE} ${CMAKE_SOURCE_DIR} )
   # add it to the generated cl headers
   SET( CL_GEN_HEADER_CVT ${CL_GEN_HEADER_CVT} ${CL_GEN_OUTPUT} )
ENDFOREACH(CLFILE)

FOREACH( GLSLFILE ${GLSL_SHADER} )
    GLSLTOHEADER( ${GLSLFILE} ${CMAKE_SOURCE_DIR} )
   SET( GLSL_GEN_HEADER_CVT ${GLSL_GEN_HEADER_CVT} ${GLSL_GEN_OUTPUT} )
ENDFOREACH(GLSLFILE)

SET( CVT_HEADERS
   cl/CLBuffer.h
   cl/CLCommandQueue.h
   cl/CLContext.h
   cl/CLDevice.h
   cl/CLEvent.h
   cl/CLException.h
   cl/CLImage2D.h
   cl/CLImageFormat.h
   cl/CLKernel.h
   cl/CLLocalSpace.h
   cl/CLNDRange.h
   cl/CLObject.h
   cl/CLObject.inl
   cl/CLPlatform.h
   cl/CLProgram.h
   cl/CLUtil.h
   cl/OpenCL.h
   container/List.h
   container/RBTree.h
   com/AsyncTCPServer.h
   com/AsyncTCPClient.h
   com/AsyncUDPClient.h
   com/Host.h
   com/SharedMemory.h
   com/Socket.h
   com/TCPClient.h
   com/TCPServer.h
   com/UDPClient.h
   geom/ArcBall.h
   geom/Box.h
   geom/CBezier.h
   geom/Ellipse.h
   geom/KDTree.h
   geom/Line2D.h
   geom/MarchingCubes.h
   geom/Polygon.h
   geom/PointSet.h
   geom/QBezier.h
   geom/Rect.h
   geom/scene/Scene.h
   geom/scene/SceneCamera.h
   geom/scene/SceneGeometry.h
   geom/scene/SceneLight.h
   geom/scene/SceneLines.h
   geom/scene/SceneLoader.h
   geom/scene/SceneMaterial.h
   geom/scene/SceneMesh.h
   geom/scene/SceneNode.h
   geom/scene/ScenePoints.h
   geom/scene/SceneSpatial.h
   geom/scene/SceneTexture.h
   gfx/Color.h
   gfx/IBorder.h
   gfx/IBoxFilter.h
   gfx/IDecompose.h
   gfx/Image.h
   gfx/IExpr.h
   gfx/IExprType.h
   gfx/IFill.h
   gfx/IFormat.h
   gfx/ICanny.h
   gfx/IColorCode.h
   gfx/IColorCodeMap.h
   gfx/IComponents.h
   gfx/IConvert.h
   gfx/IConvolve.h
   gfx/IMapScoped.h
   gfx/IMorphological.h
   gfx/IThreshold.h
   gfx/ILoader.h
   gfx/ISaver.h
   gfx/ifilter/ROFDenoise.h
   gfx/ifilter/ROFFGPFilter.h
   gfx/ifilter/Homography.h
   gfx/ifilter/GaussIIR.h
   gfx/ifilter/BrightnessContrast.h
   gfx/ifilter/ITransform.h
   gfx/ifilter/IWarp.h
   gfx/ifilter/IntegralFilter.h
   gfx/ifilter/BoxFilter.h
   gfx/ifilter/GuidedFilter.h
   gfx/ifilter/StereoGCVFilter.h
   gfx/ifilter/TVL1Flow.h
   gfx/ifilter/TVL1Stereo.h
   gfx/IFilter.h
   gfx/IScaleFilter.h
   gfx/ImageAllocator.h
   gfx/ImageAllocatorMem.h
   gfx/ImageAllocatorCL.h
   gfx/ImageAllocatorGL.h
   gfx/Clipping.h
   gfx/Drawable.h
   gfx/GFXEngine.h
   gfx/GFXEngineImage.h
   gfx/GFX.h
   gfx/IKernel.h
   gfx/ColorspaceXYZ.h
   gfx/Font.h
   gfx/Alignment.h
   gfx/Path.h
   gfx/PDROF.h
   gfx/PDROFInpaint.h
   gfx/DummyFont.h
   gl/OpenGL.h
   gl/GLFBO.h
   gl/GLFormat.h
   gl/GLContext.h
   gl/GLException.h
   gl/GLProgram.h
   gl/GLBuffer.h
   gl/GLLines.h
   gl/GLPoints.h
   gl/GLTexture.h
   gl/GLVertexArray.h
   gl/GFXEngineGL.h
   gl/GFXEngineGLFBO.h
   gl/GLRBO.h
   gl/GLTexFont.h
   gl/GLTeapot.h
   gl/GLTeapot.inl
   gl/progs/GLBasicProg.h
   gl/progs/GLFillRoundRectProg.h
   gl/progs/GLDrawTextProg.h
   gl/progs/GLDrawImageProg.h
   gl/progs/GLDrawIconProg.h
   gl/progs/GLDrawModelProg.h
   gl/progs/GLDrawTexModelProg.h
   gl/progs/GLFillPathProg.h
   gl/progs/GLTexMLSProg.h
   gl/scene/GLSBaseModel.h
   gl/scene/GLSCamera.h
   gl/scene/GLSLight.h
   gl/scene/GLSMaterial.h
   gl/scene/GLSTexture.h
   gl/scene/GLSRenderable.h
   gl/scene/GLSRenderableGroup.h
   gl/scene/GLSShader.h
   gl/scene/GLSTransformable.h
   gl/scene/GLSVisitor.h
   gl/scene/GLScene.h
   gl/scene/GLSShaderProgram.h
   gui/Application.h
   gui/BasicTimer.h
   gui/Button.h
   gui/Events.h
   gui/event/CloseEvent.h
   gui/event/Event.h
   gui/event/HideEvent.h
   gui/event/MouseMoveEvent.h
   gui/event/MousePressEvent.h
   gui/event/MouseReleaseEvent.h
   gui/event/MoveEvent.h
   gui/event/PaintEvent.h
   gui/event/ResizeEvent.h
   gui/event/ShowEvent.h
   gui/event/KeyEvent.h
   gui/GLView.h
   gui/ImageView.h
   gui/internal/TimerInfo.h
   gui/internal/TimerInfoList.h
   gui/internal/WidgetImplDefault.h
   gui/KeyCodes.h
   gui/Label.h
   gui/Moveable.h
   gui/ParamSetView.h
   gui/ParamView.h
   gui/PlotView.h
   gui/PlotData.h
   gui/PlotDataSamples.h
   gui/Slider.h
   gui/TimeoutHandler.h
   gui/ToggleButton.h
   gui/Window.h
   gui/Widget.h
   gui/WidgetContainer.h
   gui/WidgetLayout.h
   io/CameraInfo.h
   io/CameraMode.h
   io/CameraModeSet.h
   io/Camera.h
   io/FileSystem.h
   io/FloFile.h
   io/ImageSequence.h
   io/IOHandler.h
   io/IOSelect.h
   io/KittiVOParser.h
   io/Resources.h
   io/RawVideoWriter.h
   io/RawVideoReader.h
   io/RGBDInput.h
   io/RGBDParser.h
   io/VideoInput.h
   io/VideoReader.h
   io/Postscript.h
   io/GFXEnginePS.h
   io/StereoInput.h
   math/CostFunction.h
   math/Complex.h
   math/FFT.h
   math/Fixed.h
   math/Function.h
   math/JointMeasurements.h
   math/GaussNewton.h
   math/LevenbergMarquard.h
   math/Math.h
   math/Matrix.h
   math/Matrix2.h
   math/Matrix3.h
   math/Matrix4.h
   math/Matrix6.h
   math/TerminationCriteria.h
   math/Polynomial.h
   math/Quaternion.h
   math/sac/SampleConsensusModel.h
   math/sac/RANSAC.h
   math/sac/Line2DSAC.h
   math/sac/HomographySAC.h
   math/sac/EssentialSAC.h
   math/sac/EPnPSAC.h
   math/sac/P3PSac.h
   math/SE3.h
   math/SL3.h
   math/Translation2D.h
   math/Sim2.h
   math/GA2.h
   math/Vector.h
   math/Vector2.h
   math/Vector3.h
   math/Vector4.h
   math/Vector6.h
   math/graph/Graph.h
   math/graph/GraphNode.h
   math/graph/GraphEdge.h
   math/graph/GraphVisitor.h
   math/SparseBlockMatrix.h
   util/CPU.h
   util/CVTAssert.h
   util/CVTTest.h
   util/Condition.h
   util/ConfigFile.h
   util/Debug.h
   util/Delegate.h
   util/Delegate.inl
   util/Data.h
   util/DataIterator.h
   util/Exception.h
   util/EigenBridge.h
   util/Mutex.h
   util/ParamInfo.h
   util/ParamSet.h
   util/Range.h
   util/RNG.h
   util/Signal.h
   util/ScopedBuffer.h
   util/SIMDDebug.h
   util/SIMD.h
   util/SIMDSSE.h
   util/SIMDSSE2.h
   util/SIMDSSE3.h
   util/SIMDSSSE3.h
   util/SIMDSSE41.h
   util/SIMDSSE42.h
   util/SIMDAVX.h
   util/TQueue.h
   util/Thread.h
   util/Time.h
   util/Util.h
   util/Flags.h
   util/Stack.h
   util/String.h
   util/Plugin.h
   util/PluginManager.h
   util/PluginFile.h
   util/internal/ParamTypes.def
   vision/BoardDetector.h
   vision/CameraCalibration.h
   vision/ESM.h
   vision/EPnP.h
   vision/features/AGAST.h
   vision/features/agast/ASTDetector.h
   vision/features/agast/OAST9_16.h
   vision/features/agast/Agast5_8.h
   vision/features/agast/Agast7_12s.h
   vision/features/agast/Agast7_12d.h
   vision/features/BRIEF.h
   vision/features/BRIEFPattern.h
   vision/features/FAST.h
   vision/features/Feature.h
   vision/features/FeatureDescriptor.h
   vision/features/FeatureDescriptorExtractor.h
   vision/features/FeatureDetector.h
   vision/features/FeatureMatch.h
   vision/features/FeatureSet.h
   vision/features/Harris.h
   vision/features/NMSFilter.h
   vision/features/MatchBruteForce.h
   vision/features/ORB.h
   vision/features/ORBPattern.h
   vision/features/RowLookupTable.h
   vision/features/GridFilter.h
   vision/IntegralImage.h
   vision/ImagePyramid.h
   vision/Flow.h
   vision/HCalibration.h
   vision/KLTPatch.h
   vision/LSH.h
   vision/MeasurementModel.h
   vision/Patch.h
   vision/PatchGenerator.h
   vision/PMHuberStereo.h
   vision/ReprojectionError.h
   vision/PointCorrespondences3d2d.h
   vision/StereoCameraCalibration.h
   vision/StereoRectification.h
   vision/TSDFVolume.h
   vision/Vision.h
   vision/SparseBundleAdjustment.h
   vision/rgbdvo/ApproxMedian.h
   vision/rgbdvo/CostFunction.h
   vision/rgbdvo/DVOCostFunction.h
   vision/rgbdvo/ErrorLogger.h
   vision/rgbdvo/IntensityKeyframe.h
   vision/rgbdvo/KeyframeData.h
   vision/rgbdvo/InformationSelection.h
   vision/rgbdvo/GradientThresholdSelection.h
   vision/rgbdvo/Optimizer.h
   vision/rgbdvo/GNOptimizer.h
   vision/rgbdvo/LMOptimizer.h
   vision/rgbdvo/PhotometricError.h
   vision/rgbdvo/ReferenceFactory.h
   vision/rgbdvo/RGBDKeyframe.h
   vision/rgbdvo/RGBDPreprocessor.h
   vision/rgbdvo/RGBDVisualOdometry.h
   vision/rgbdvo/RGBDWarp.h
   vision/RobustWeighting.h
   vision/rgbdvo/SystemBuilder.h
   vision/slam/SlamMap.h
   vision/slam/Keyframe.h
   vision/slam/MapFeature.h
   vision/slam/MapMeasurement.h
   vision/slam/FlatSLAMMap.h
   vision/slam/stereo/StereoSLAM.h
   vision/slam/stereo/DescriptorDatabase.h
   vision/slam/stereo/FeatureTracking.h
   #vision/slam/stereo/ORBTracking.h
   #vision/slam/stereo/KLTTracking.h
   #vision/slam/stereo/ORBStereoMatching.h
   vision/slam/stereo/MapOptimizer.h
   vision/slam/stereo/FeatureAnalyzer.h
   vision/slam/stereo/DepthInitializer.h
   #vision/slam/stereo/ORBStereoInit.h
   #vision/slam/stereo/PatchStereoInit.h
   io/xml/XMLAttribute.h
   io/xml/XMLCData.h
   io/xml/XMLComment.h
   io/xml/XMLDecl.h
   io/xml/XMLDecoder.h
   io/xml/XMLDecoderUTF8.h
   io/xml/XMLDocument.h
   io/xml/XMLElement.h
   io/xml/XMLEncoding.h
   io/xml/XMLLeaf.h
   io/xml/XMLNode.h
   io/xml/XMLSerializable.h
   io/xml/XMLText.h
   ${GLSL_SHADER}
   ${CL_KERNELS}
   ${GLSL_GEN_HEADER_CVT}
   ${CL_GEN_HEADER_CVT}
   ${CMAKE_CURRENT_BINARY_DIR}/util/Version.h
)

SOURCE_GROUP( GLSL FILES ${GLSL_SHADER} )
SOURCE_GROUP( CL FILES ${CL_KERNELS} )

SET( CVT_SOURCES
	cl/OpenCL.cpp
	cl/CLBuffer.cpp
	cl/CLCommandQueue.cpp
	cl/CLContext.cpp
	cl/CLDevice.cpp
	cl/CLEvent.cpp
	cl/CLException.cpp
	cl/CLImage2D.cpp
	cl/CLImageFormat.cpp
	cl/CLKernel.cpp
	cl/CLProgram.cpp
	com/Host.cpp
	com/Socket.cpp
	com/TCPClient.cpp
	com/TCPServer.cpp
	com/UDPClient.cpp
	gfx/GFX.cpp
	gfx/GFXEngineImage.cpp
	gfx/IBoxFilter.cpp
	gfx/IConvert.cpp
	gfx/IConvolve.cpp
    gfx/IDecompose.cpp
	gfx/IFill.cpp
	gfx/IFormat.cpp
	gfx/Color.cpp
	gfx/Image.cpp
	gfx/ImageOperations.cpp
	gfx/ImageTest.cpp
	gfx/IMorphological.cpp
	gfx/IThreshold.cpp
	gfx/ifilter/ROFDenoise.cpp
	gfx/ifilter/ROFFGPFilter.cpp
	gfx/ifilter/Homography.cpp
	gfx/ifilter/GaussIIR.cpp
	gfx/ifilter/BrightnessContrast.cpp
	gfx/ifilter/ITransform.cpp
	gfx/ifilter/IWarp.cpp
	gfx/ifilter/IntegralFilter.cpp
	gfx/ifilter/BoxFilter.cpp
	gfx/ifilter/GuidedFilter.cpp
	gfx/ifilter/StereoGCVFilter.cpp
	gfx/ifilter/TVL1Flow.cpp
	gfx/ifilter/TVL1Stereo.cpp
	gfx/ImageAllocatorCL.cpp
	gfx/ImageAllocatorGL.cpp
	gfx/ImageAllocatorMem.cpp
	gfx/IScaleFilter.cpp
	gfx/IKernel.cpp
	gfx/ColorspaceXYZ.cpp
	geom/KDTreeTest.cpp
	geom/MarchingCubes.cpp
	geom/Rect.cpp
	geom/PointSet.cpp
	geom/PointSetTest.cpp
	geom/scene/Scene.cpp
	geom/scene/SceneGeometry.cpp
	geom/scene/SceneMesh.cpp
	gl/GLContext.cpp
	gl/GLBuffer.cpp
	gl/GLFBO.cpp
	gl/GLProgram.cpp
	gl/GLTexture.cpp
	gl/GLVertexArray.cpp
	gl/OpenGL.cpp
	gl/GFXEngineGL.cpp
	gl/GLTexFont.cpp
	gl/GLTeapot.cpp
	gl/progs/GLBasicProg.cpp
	gl/progs/GLFillRoundRectProg.cpp
	gl/progs/GLDrawTextProg.cpp
	gl/progs/GLDrawImageProg.cpp
	gl/progs/GLDrawIconProg.cpp
	gl/progs/GLDrawModelProg.cpp
	gl/progs/GLDrawTexModelProg.cpp
	gl/progs/GLFillPathProg.cpp
	gl/progs/GLTexMLSProg.cpp
    gl/scene/GLScene.cpp
    gl/scene/GLSShader.cpp
	gl/scene/GLSVisitor.cpp
	gl/scene/GLSRenderableGroup.cpp
	gl/scene/GLSBaseModel.cpp
    gl/scene/GLSShaderProgram.cpp
	gui/Application.cpp
	gui/BasicTimer.cpp
	gui/Button.cpp
	gui/GLView.cpp
	gui/Moveable.cpp
	gui/ParamSetView.cpp
	gui/ParamView.cpp
	gui/PlotView.cpp
	gui/ImageView.cpp
	gui/internal/TimerInfo.cpp
	gui/internal/TimerInfoList.cpp
	gui/ToggleButton.cpp
	gui/Widget.cpp
	gui/WidgetContainer.cpp
	gui/Window.cpp
	gui/Label.cpp
	io/CameraInfo.cpp
	io/CameraModeSet.cpp
	io/Camera.cpp
	io/FileSystem.cpp
	io/FloFile.cpp
	io/ImageSequence.cpp
	io/IOSelect.cpp
	io/KittiVOParser.cpp
	io/Resources.cpp
	io/RawVideoWriter.cpp
	io/RawVideoReader.cpp
	io/RGBDParser.cpp
	io/VideoReader.cpp
	math/Complex.cpp
	math/FFT.cpp
	math/Fixed.cpp
	math/JointMeasurements.cpp
	math/JointMeasurementsTest.cpp
	math/Math.cpp
	math/Vector.cpp
	math/Matrix.cpp
	math/Polynomial.cpp
	math/SE3Test.cpp
	math/SL3Test.cpp
	math/Sim2Test.cpp
	math/GA2Test.cpp
	util/Data.cpp
	util/ConfigFile.cpp
	util/ParamInfo.cpp
	util/ParamSet.cpp
	util/Range.cpp
	util/SIMD.cpp
	util/SIMDSSE.cpp
	util/SIMDSSE2.cpp
	util/SIMDSSE3.cpp
	util/SIMDSSSE3.cpp
	util/SIMDSSE41.cpp
	util/SIMDSSE42.cpp
	util/SIMDAVX.cpp
	util/SIMDTest.cpp
	util/Time.cpp
	util/String.cpp
	util/PluginManager.cpp
	util/PluginFile.cpp
	vision/BoardDetector.cpp
	vision/CameraCalibrationTest.cpp
	vision/EPnP.cpp
    vision/features/AGAST.cpp
    vision/features/agast/OAST9_16.cpp
    vision/features/agast/Agast5_8.cpp
    vision/features/agast/Agast7_12s.cpp
    vision/features/agast/Agast7_12d.cpp
	vision/features/FAST.cpp
	vision/features/fast/fast9.cpp
	vision/features/fast/fast10.cpp
	vision/features/fast/fast11.cpp
	vision/features/fast/fast12.cpp
	vision/features/FeatureSet.cpp
	vision/features/Harris.cpp
	vision/features/GridFilter.cpp
	vision/Flow.cpp
	vision/IntegralImage.cpp
	vision/ImagePyramidTest.cpp
	vision/KLTPatchTest.cpp
	vision/features/ORB.cpp
	vision/features/RowLookupTable.cpp
	vision/features/RowLookupTableTest.cpp
	vision/PatchGenerator.cpp
	vision/Patch.cpp
	vision/PMHuberStereo.cpp
    vision/ReprojectionError.cpp
	vision/SparseBundleAdjustment.cpp
	vision/StereoRectification.cpp
	vision/rgbdvo/InformationSelectionTest.cpp
	vision/slam/Keyframe.cpp
    vision/slam/FlatSLAMMap.cpp
	vision/slam/SlamMap.cpp
	vision/slam/stereo/FeatureTracking.cpp
	#vision/slam/stereo/KLTTracking.cpp
	#vision/slam/stereo/ORBTracking.cpp
	vision/slam/stereo/StereoSLAM.cpp
	#vision/slam/stereo/ORBStereoInit.cpp
	#vision/slam/stereo/PatchStereoInit.cpp
	vision/TSDFVolume.cpp
	vision/Vision.cpp
	io/xml/XMLDecoder.cpp
	io/xml/XMLDecoderUTF8.cpp
)

IF(UNIX)
   IF(APPLE)
	   SET(CVT_HEADERS ${CVT_HEADERS}
         io/QTKitCamera.h
         gui/internal/OSX/ApplicationOSX.h
         gui/internal/OSX/WidgetImplWinGLOSX.h
         gui/internal/OSX/OSXGLView.h
         gui/internal/OSX/OSXTimerHandler.h
         gui/internal/OSX/TimerInfoListOSX.h
         gui/internal/OSX/TimerInfoOSX.h
		 gui/internal/OSX/OSXKeyMap.h
		 gui/internal/OSX/CGLContext.h
      )
		SET( CVT_SOURCES ${CVT_SOURCES}
			io/QTKitCamera.mm
			gui/internal/OSX/ApplicationOSX.mm
			gui/internal/OSX/WidgetImplWinGLOSX.mm
			gui/internal/OSX/OSXGLView.mm
			gui/internal/OSX/TimerInfoListOSX.cpp
			gui/internal/OSX/OSXTimerHandler.mm
			gui/internal/OSX/TimerInfoOSX.mm
		)

	SET( CVT_DEP_LIBRARIES
         ${CVT_DEP_LIBRARIES}
         "-framework Cocoa"
         "-framework CoreFoundation"
         "-framework QuartzCore"
         "-framework QTKit"
         "-framework OpenGL"
      )
   ELSE(APPLE)
	   SET(CVT_HEADERS ${CVT_HEADERS}
         io/V4L2Camera.h
         gui/internal/X11/GLXContext.h
         gui/internal/X11/ApplicationX11.h
         gui/internal/X11/WidgetImplWinGLX11.h
         gui/internal/X11/X11Handler.h
		 gui/internal/X11/X11KeyMap.h
      )
  SET(CVT_SOURCES ${CVT_SOURCES}
         io/V4L2Camera.cpp
         gui/internal/X11/ApplicationX11.cpp
         gui/internal/X11/WidgetImplWinGLX11.cpp
         gui/internal/X11/X11Handler.cpp
		 gui/internal/X11/X11KeyMap.cpp
      )
   ENDIF(APPLE)
ENDIF()

# optional uEyeUsbCamera driver if available
SET( CVT_HAS_UEYE NO )
FIND_PACKAGE(uEyeUsb)
OPTION( CVT_USEUEYE "Use UEyeUsbCamera" TRUE )
IF( UEYEUSB_FOUND AND CVT_USEUEYE )
    SET(CVT_HEADERS ${CVT_HEADERS} io/UEyeUsbCamera.h io/UEyeStereo.h )
    SET(CVT_SOURCES ${CVT_SOURCES} io/UEyeUsbCamera.cpp )
    INCLUDE_DIRECTORIES( BEFORE ${UEYEUSB_INCLUDE_DIRS} )
    SET(CVT_DEP_LIBRARIES ${CVT_DEP_LIBRARIES} ${UEYEUSB_LIBRARIES})
    ADD_DEFINITIONS( -DUEYEUSB_FOUND )
    SET( CVT_HAS_UEYE YES )
ENDIF()

OPTION( CVT_USE_DC1394 "Use DC1394 library if available" TRUE )
SET( CVT_HAS_DC1394 NO )
IF( ${CVT_USE_DC1394} )
    FIND_PACKAGE(DC1394)
    IF( ${DC1394_FOUND} )
	    SET( CVT_HEADERS ${CVT_HEADERS} io/DC1394Camera.h io/ChameleonStereo.h )
        SET( CVT_SOURCES ${CVT_SOURCES} io/DC1394Camera.cpp io/ChameleonStereo.cpp )
        INCLUDE_DIRECTORIES( BEFORE ${LIBDC1394_INCLUDE_DIR} )
        SET( CVT_DEP_LIBRARIES ${CVT_DEP_LIBRARIES} ${LIBDC1394_LIBRARY} )
        ADD_DEFINITIONS( -DDC1394_FOUND )
        SET( CVT_HAS_DC1394 YES )
   ENDIF()
ENDIF()

SET( CVT_HAS_OpenNI NO )
FIND_PACKAGE( OpenNI )
OPTION( CVT_USE_OPENNI "Use OpenNI" TRUE )
IF( OpenNI_FOUND AND CVT_USE_OPENNI )
    SET( CVT_HEADERS ${CVT_HEADERS} io/OpenNICamera.h io/OpenNIManager.h )
    SET( CVT_SOURCES ${CVT_SOURCES} io/OpenNICamera.cpp io/OpenNIManager.cpp )
    INCLUDE_DIRECTORIES( BEFORE ${OpenNI_INCLUDE_DIRS} )
    SET( CVT_DEP_LIBRARIES ${CVT_DEP_LIBRARIES} ${OpenNI_LIBRARIES} )
    ADD_DEFINITIONS( -DOpenNI_FOUND )
    SET( CVT_HAS_OPENNI YES )
ENDIF()

SET( CVT_HAS_OPENNI2 NO )
FIND_PACKAGE( OpenNI2 )
OPTION( CVT_USE_OPENNI2 "Use OpenNI2" FALSE )
IF( OpenNI2_FOUND AND CVT_USE_OPENNI2 )
    SET( CVT_HEADERS ${CVT_HEADERS} io/OpenNI2Camera.h )
    SET( CVT_SOURCES ${CVT_SOURCES} io/OpenNI2Camera.cpp )
    INCLUDE_DIRECTORIES( BEFORE ${OpenNI2_INCLUDE_DIRS} )   
    SET( CVT_DEP_LIBRARIES ${CVT_DEP_LIBRARIES} ${OpenNI2_LIBRARIES} )
    ADD_DEFINITIONS( -DOpenNI2_FOUND )
   SET( CVT_HAS_OPENNI2 YES )
ENDIF()

# include directories
INCLUDE_DIRECTORIES(
    BEFORE
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${Eigen3_INCLUDE_DIRS}
   ${OpenCL_INCLUDE_DIRS}
   ${X11_X11_INCLUDE_PATH}
   ${ffmpeg_INCLUDE_DIRS}
)

SET(CVT_ALL "${CVT_HEADERS}" "${CVT_SOURCES}")

ADD_LIBRARY( cvt ${CVT_ALL} )
SET( LIB_VERSION "${CVT_VERSION_MAJOR}.${CVT_VERSION_MINOR}.${CVT_VERSION_PATCH}" )
SET_TARGET_PROPERTIES( cvt PROPERTIES VERSION "${LIB_VERSION}" )
SET_TARGET_PROPERTIES( cvt PROPERTIES DEBUG_POSTFIX "_d")	
SET_TARGET_PROPERTIES( cvt PROPERTIES ZEROOPT_POSTFIX "_d")	
TARGET_LINK_LIBRARIES( cvt ${CVT_DEP_LIBRARIES} )
ADD_DEPENDENCIES( cvt gen_version_file )

#PREFIX hack for needed for Xcode and MVSC Projects
IF( ${CMAKE_GENERATOR} MATCHES "Xcode" )
	SET_TARGET_PROPERTIES( cvt PROPERTIES PREFIX "../")	
ENDIF()

# lib install
INSTALL( TARGETS "cvt" LIBRARY DESTINATION lib COMPONENT cvt )

GET_FILENAME_COMPONENT(folderName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
#install header
foreach(HEADER_FILE ${CVT_HEADERS})
    STRING( REPLACE "${CMAKE_CURRENT_BINARY_DIR}/" "" CLEAN_HEADER ${HEADER_FILE} )
    GET_FILENAME_COMPONENT(pathName "${CLEAN_HEADER}" PATH)
    INSTALL(FILES "${HEADER_FILE}" 
			DESTINATION "include/${folderName}/${pathName}" 
			COMPONENT Headers
	)
endforeach(HEADER_FILE)

SET_TARGET_PROPERTIES( cvt
         PROPERTIES COMPILE_DEFINITIONS
         PLUGIN_PATH="${CMAKE_BINARY_DIR}/bin/plugins/" 
)

CVTTESTS(cvt cvt/util/cvttestsproto.h)
IF( ${CMAKE_BUILD_TYPE} STREQUAL "DEBUG" OR
    ${CMAKE_BUILD_TYPE} STREQUAL "ZEROOPT" )
   MESSAGE( "DEBUG CONFIGURATION -> adding cvttest" )
   ADD_EXECUTABLE( cvttest util/CVTTest.cpp )
   ADD_DEPENDENCIES( cvttest cvt_test_extraction )

   IF( ${CMAKE_GENERATOR} MATCHES "Xcode" )
      SET_TARGET_PROPERTIES( cvttest PROPERTIES PREFIX "../" )
   ENDIF()
   TARGET_LINK_LIBRARIES( cvttest cvt ${CVT_DEP_LIBRARIES} )
ENDIF()

#special flags for some files
SET_SOURCE_FILES_PROPERTIES(util/SIMDSSE.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse")
SET_SOURCE_FILES_PROPERTIES(util/SIMDSSE2.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2")
SET_SOURCE_FILES_PROPERTIES(util/SIMDSSE3.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2 -msse3")
SET_SOURCE_FILES_PROPERTIES(util/SIMDSSSE3.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2 -msse3 -mssse3")
SET_SOURCE_FILES_PROPERTIES(util/SIMDSSE41.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2 -msse3 -mssse3 -msse4.1")
SET_SOURCE_FILES_PROPERTIES(util/SIMDSSE42.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2")
SET_SOURCE_FILES_PROPERTIES(util/SIMDAVX.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx")
SET_SOURCE_FILES_PROPERTIES(vision/features/FAST.cpp PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2")

# CVTConfig file for installation/package
SET( CMAKE_INSTALL_PREFIX /usr )
set( CVT_SEARCH_NAMES	"cvt cvt_d" )
set( CVT_INCLUDE_DIR	"${CMAKE_INSTALL_PREFIX}/include" )
set( CVT_INCLUDE_DIRS	"${CVT_INCLUDE_DIR}" )
set( CVT_LIB_DIR		"${CMAKE_INSTALL_PREFIX}/lib" )
set( CVT_SCRIPT_PATH	"${CMAKE_INSTALL_PREFIX}/share/cvt/cmake" )
set( CVT_PLUGIN_PATH	"${CMAKE_INSTALL_PREFIX}/share/cvt/plugins" )
set( CVT_GLCLTOHEADER_EXEC "${CMAKE_INSTALL_PREFIX}/share/cvt/utils/glcltoheader" )
set( CVT_DEFINITIONS -DCVT_DATA_FOLDER=\"${CMAKE_INSTALL_PREFIX}/share/cvt/data\" )
set( CVT_FOUND_VAR		"CVT" )
CONFIGURE_FILE ( "CVTConfig.cmake.in" "${CMAKE_BINARY_DIR}/ship/CVTConfig.cmake" @ONLY )

INSTALL( FILES "${CMAKE_BINARY_DIR}/ship/CVTConfig.cmake" DESTINATION share/cvt )

####### Build tree config #######
set( CVT_INCLUDE_DIR	"${CMAKE_SOURCE_DIR}" )
set( CVT_INCLUDE_DIRS	"${CVT_INCLUDE_DIR} ${CMAKE_BINARY_DIR}" )
set( CVT_LIB_DIR		"${LIBRARY_OUTPUT_PATH}" )
set( CVT_SCRIPT_PATH	"${CMAKE_SOURCE_DIR}/cmake" )
set( CVT_PLUGIN_PATH	"${CMAKE_BINARY_DIR}/bin/plugins" )
set( CVT_GLCLTOHEADER_EXEC ${CVT_GLCLTOHEADER} )
set( CVT_DEFINITIONS -DCVT_DATA_FOLDER=\"${CVT_DATA_FOLDER}\" )
IF( CMAKE_BUILD_TYPE STREQUAL "DEBUG" OR
    CMAKE_BUILD_TYPE STREQUAL "ZEROOPT" )
	SET( CVT_SEARCH_NAMES	"cvt_d" )
	export( PACKAGE CVT_DEBUG )
	SET( CVT_FOUND_VAR "CVT_DEBUG" )
	CONFIGURE_FILE ( "CVTConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/CVT_DEBUGConfig.cmake" @ONLY )
ELSE()
	SET( CVT_SEARCH_NAMES "cvt" )
	export( PACKAGE CVT )
	SET( CVT_FOUND_VAR "CVT" )
	CONFIGURE_FILE ( "CVTConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/CVTConfig.cmake" @ONLY )
ENDIF()
