cmake_minimum_required(VERSION 3.10)
project(TargetAnalyzerFinal)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

add_executable(TargetAnalyzerFinal 
    main.cpp 
    hole_detector.h hole_detector.cpp
    shooting_metrics.h shooting_metrics.cpp  
    visualization.h visualization.cpp
)

target_link_libraries(TargetAnalyzerFinal PRIVATE ${OpenCV_LIBS})
