# 添加 RadarProcessor 库
add_library(RadarProcessor
    RadarProcessor.h
    RadarProcessor.cpp
)

target_link_libraries(RadarProcessor
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Eigen3::Eigen
        OpenMP::OpenMP_CXX
)
target_include_directories(RadarProcessor
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
