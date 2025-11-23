cmake_minimum_required(VERSION 3.10)
project(TargetAnalyzerFinal)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

# Все исходные файлы
add_executable(TargetAnalyzerFinal
    main.cpp
    common/hole_detector.cpp
    common/shooting_metrics.cpp  
    common/visualization.cpp
    weapons/pm.cpp
)

# Директории с заголовками
target_include_directories(TargetAnalyzerFinal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/weapons
)

target_link_libraries(TargetAnalyzerFinal ${OpenCV_LIBS})