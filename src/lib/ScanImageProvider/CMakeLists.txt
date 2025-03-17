add_library(ScanImageProvider
    ScanImageProvider.h
    ScanImageProvider.cpp
)

target_link_libraries(ScanImageProvider
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Eigen3::Eigen
        OGPRParser
        RadarProcessor
        ${OpenCV_LIBS}
        OpenMP::OpenMP_CXX
)

target_include_directories(ScanImageProvider
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)