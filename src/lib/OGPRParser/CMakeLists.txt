# 添加 OGPRParser 库
add_library(OGPRParser
    OGPRParser.cpp
    OGPRParser.h
)
target_link_libraries(OGPRParser
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Eigen3::Eigen
)
target_include_directories(OGPRParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
