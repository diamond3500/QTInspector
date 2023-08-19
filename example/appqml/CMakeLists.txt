cmake_minimum_required(VERSION 3.16)

project(appqml VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Quick)


qt_add_executable(appqml
    main.cpp
)

qt_add_qml_module(appqml
    URI appqml
    VERSION 1.0
    QML_FILES Main.qml
    res/green.png
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appqml PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appqml
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appqml
    PRIVATE Qt6::Quick
    QtInspectorCore
)

include(GNUInstallDirs)
install(TARGETS appqml
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set_target_properties(appqml PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_PREFIX_PATH}/bin")
  endif()

target_include_directories(appqml PRIVATE ../../source)
if(WIN32)
target_compile_options(appqml PRIVATE /W4)
set_property(TARGET appqml PROPERTY COMPILE_FLAGS "/wd4100")
endif()
set_property(TARGET appqml PROPERTY COMPILE_WARNING_AS_ERROR ON)
