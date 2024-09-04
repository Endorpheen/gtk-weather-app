cmake_minimum_required(VERSION 3.5)

project(qtweatherapp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "/usr")
set(Qt5_DIR "/usr/lib/x86_64-linux-gnu/cmake/Qt5")

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools Network)

set(TS_FILES qtweatherapp_ru_RU.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ${TS_FILES}
)

add_executable(qtweatherapp
    ${PROJECT_SOURCES}
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(qtweatherapp PRIVATE
    Qt5::Widgets
    Qt5::Network
)

include(GNUInstallDirs)
install(TARGETS qtweatherapp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qtweatherapp)
endif()
