# Meta-Objects
set(espresso_MOC_HDRS
  espresso.hui
)
QT5_WRAP_CPP(espresso_MOC_SRCS ${espresso_MOC_HDRS} OPTIONS -I${PROJECT_SOURCE_DIR}/src)

add_library(espresso MODULE
  espresso_funcs.cpp
  ${espresso_MOC_SRCS}
)
set_target_properties(espresso PROPERTIES
   LIBRARY_OUTPUT_DIRECTORY ${Aten_BINARY_DIR}/data/plugins
   COMPILE_DEFINITIONS "QT_PLUGIN"
   PREFIX ""
)

if(UNIX AND NOT APPLE)
install(TARGETS espresso
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/aten/plugins COMPONENT RuntimePlugins
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/aten/plugins COMPONENT RuntimePlugins
)
endif(UNIX AND NOT APPLE)

target_include_directories(espresso PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_BINARY_DIR}/src
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
)
