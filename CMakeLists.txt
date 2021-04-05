add_definitions(-DBUILD_PLY_VIEWER)
set(READER_SUPPORT_PTX ON CACHE BOOL "Option to open ptx.")
set(READER_SUPPORT_E57 OFF CACHE BOOL "Option to open e57.")

set(PUBLIC_HEADERS 
  camera.h
  mainwindow.h
  scene.h
  viewer.h
)
set(PROJECT_SRCS
  camera.cpp
  mainwindow.cpp
  scene.cpp
  viewer.cpp  
  main.cpp  
  reader.cpp
)

InitQt("Core;Widgets;LinguistTools;OpenGL")
qt5_add_resources(UI_RESOURCES resources.qrc)

set(PUBLIC_HEADER_PATH "${CMAKE_SOURCE_DIR}/ply_viewer")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
set(APP "plyViewer")
add_executable(${APP} "${PROJECT_SRCS}" "${PUBLIC_HEADERS}" "${UI_RESOURCES}" readme.txt)
target_link_libraries(${APP}  opengl32.lib)
target_link_libraries(${APP}  PointCloudRender)
if (READER_SUPPORT_PTX)
	add_definitions(-DSUPPORT_PTX)
	target_link_libraries(${APP}  ptxlib)
endif()
if (READER_SUPPORT_E57)
	add_definitions(-DSUPPORT_E57)
	target_link_libraries(${APP}  e57reader)
endif()
target_link_libraries(${APP}  Qt5::Core  Qt5::Widgets  Qt5::OpenGL )
if(WIN32)
	  AddQtDllCopyTarget(${APP})
endif()