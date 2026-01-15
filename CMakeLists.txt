cmake_minimum_required(VERSION 3.16)

project(PhoneBook VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql)

set(TS_FILES PhoneBook_ru_RU.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PhoneBook
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        Contact.cpp
        ContactBook.cpp
        PhoneNumber.cpp
        Validator.cpp

        Contact.h
        ContactBook.h
        Date.h
        PhoneNumber.h
        Validator.h

        databasemanager.h
        databasemanager.cpp
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(PhoneBook SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(PhoneBook
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(PhoneBook
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PhoneBook)
endif()

set_target_properties(PhoneBook PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PhoneBook
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PhoneBook)
endif()

# ----------------------------
# Task1: Console version (NO Qt)
# ----------------------------
add_executable(PhoneBookConsole
    console/main_console.cpp
    console/Contact.cpp
    console/ContactBook.cpp
    console/PhoneNumber.cpp
    console/Validator.cpp
)

target_include_directories(PhoneBookConsole PRIVATE console)
target_compile_features(PhoneBookConsole PRIVATE cxx_std_17)
