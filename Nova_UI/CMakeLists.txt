cmake_minimum_required(VERSION 3.16)

project(Nova VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Multimedia REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql)

include_directories(${CMAKE_SOURCE_DIR}/../Nova_Backend/include)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ViewDatabaseWidget.cpp
)

set(BACKEND_SOURCES
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/Core/NeuralNet.cpp
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/Humanizer/WordVectorHelper.cpp
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/Humanizer/TopicExtractor.cpp
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/Humanizer/ResponseVariator.cpp
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/Humanizer/ContextTracker.cpp
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/Humanizer/ResponseSelector.cpp
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/Controller.cpp
    ${CMAKE_SOURCE_DIR}/../Nova_Backend/src/utils.cpp
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(untitled
      MANUAL_FINALIZATION
      ${PROJECT_SOURCES}
      ${BACKEND_SOURCES}
      asset/asset.qrc
      asset/bot.png
      asset/book.png
      asset/bot_icon.png
      asset/bot_teach_icon.png
      asset/pop.wav
      ViewDatabaseWidget.hpp
      ViewDatabaseWidget.cpp
  )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET untitled APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(untitled SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(untitled
            ${PROJECT_SOURCES}
        )
    endif()
endif()

include_directories("C:/msys64/mingw64/include")
link_directories("C:/msys64/mingw64/lib")

target_link_libraries(untitled
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt6::Multimedia
        Qt${QT_VERSION_MAJOR}::Sql
        "C:/msys64/mingw64/lib/libsqlite3.a"
      )

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.untitled)
endif()
set_target_properties(untitled PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS untitled
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(untitled)
endif()
