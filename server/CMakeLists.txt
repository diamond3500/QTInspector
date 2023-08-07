cmake_minimum_required(VERSION 3.24.0)
project(server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Quick)

FILE (GLOB sources *.cc *.h *.cpp *.ui *.qrc)
SOURCE_GROUP("sources" FILES ${sources})


set(PROJECT_SOURCES ${sources})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(server
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET server APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(server SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(server
            ${PROJECT_SOURCES}
        )
    endif()
endif()
include_directories(../source ../source/3rd/protobuf/src)
target_link_libraries(server PRIVATE Qt${QT_VERSION_MAJOR}::Widgets QtInspectorCore Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Quick)

set_target_properties(server PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS server
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(server)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    #set_target_properties(server PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")
    set_target_properties(server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_PREFIX_PATH}/bin")
endif()

target_compile_options(server PRIVATE /W4)
set_property(TARGET server PROPERTY COMPILE_FLAGS "/wd4100")
set_property(TARGET server PROPERTY COMPILE_WARNING_AS_ERROR ON)