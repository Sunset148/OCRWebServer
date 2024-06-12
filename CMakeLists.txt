cmake_minimum_required(VERSION 3.12)
project(OcrLiteOnnx)

# 设置默认输出类型为BIN
if (NOT DEFINED OCR_OUTPUT)
    set(OCR_OUTPUT "BIN")
    message(STATUS "No OCR_OUTPUT, defaulting to BIN")
endif ()

# 编译选项
set(CMAKE_CXX_STANDARD 11)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-Wall -g -O0")
else ()
    add_definitions("-Wall")
endif ()

# 查找 OpenMP
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else ()
    message(FATAL_ERROR "OpenMP Not Found!")
endif ()

# 查找 OnnxRuntime
include(${CMAKE_CURRENT_SOURCE_DIR}/onnxruntime-static/OnnxRuntimeWrapper.cmake)
find_package(OnnxRuntime REQUIRED)
if (OnnxRuntime_FOUND)
    message(STATUS "OnnxRuntime_LIBS: ${OnnxRuntime_LIBS}")
    message(STATUS "OnnxRuntime_INCLUDE_DIRS: ${OnnxRuntime_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "onnxruntime Not Found!")
endif ()

# 查找 OpenCV
set(BUILD_SHARED_LIBS false)
include(${CMAKE_CURRENT_SOURCE_DIR}/opencv-static/OpenCVWrapperConfig.cmake)
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
    message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
    message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "opencv Not Found!")
endif ()

# 设置项目 include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 设置项目WebService的 include
include_directories(${PROJECT_SOURCE_DIR}/ReactorHttp-Cpp)

# 设置源文件
file(GLOB OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB Web_Service ${PROJECT_SOURCE_DIR}/ReactorHttp-Cpp/*.cpp)

set(OCR_COMPILE_CODE ${OCR_SRC})

# 设置目标类型
if (OCR_OUTPUT STREQUAL "JNI")
    # JNI
    find_package(JNI REQUIRED)
    if (JNI_FOUND)
        message("JNI FOUND")
        message(STATUS "JNI_LIBS: ${JNI_LIBS}")
        message(STATUS "JNI_INCLUDE_DIRS: ${JNI_INCLUDE_DIRS}")
        include_directories(${JNI_INCLUDE_DIRS})
    else ()
        message(FATAL_ERROR "JNI Not Found!")
    endif ()
    add_library(OcrLiteOnnx SHARED ${OCR_COMPILE_CODE})
    target_compile_definitions(OcrLiteOnnx PRIVATE __JNI__)
    target_link_libraries(OcrLiteOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS} ${JNI_LIBS} ${OpenMP_CXX_LIB_NAMES})
elseif (OCR_OUTPUT STREQUAL "CLIB")
    # CLIB
    add_library(OcrLiteOnnx SHARED ${OCR_COMPILE_CODE})
    target_compile_definitions(OcrLiteOnnx PRIVATE __CLIB__)
    target_link_libraries(OcrLiteOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS} ${OpenMP_CXX_LIB_NAMES})
elseif (OCR_OUTPUT STREQUAL "BIN")
    # BIN
    add_executable(OcrLiteOnnx ${OCR_COMPILE_CODE} ${Web_Service})
    target_compile_definitions(OcrLiteOnnx PRIVATE __EXEC__)
    target_link_libraries(OcrLiteOnnx ${OnnxRuntime_LIBS} ${OpenCV_LIBS} ${OpenMP_CXX_LIB_NAMES})
    target_compile_options(OcrLiteOnnx PRIVATE -g)

endif ()

# 安装目标
install(TARGETS OcrLiteOnnx EXPORT OcrLiteOnnx)
if (OCR_OUTPUT STREQUAL "CLIB")
    # 安装头文件
    file(GLOB OCR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    install(FILES ${OCR_INCLUDE} DESTINATION include)
endif ()

# benchmark
option(OCR_BENCHMARK "build benchmark" ON)
if (OCR_BENCHMARK AND (OCR_OUTPUT STREQUAL "BIN"))
    file(GLOB OCR_OCR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB OCR_Web_Service ${PROJECT_SOURCE_DIR}/ReactorHttp-Cpp/*.cpp)
    add_executable(benchmark ${OCR_OCR_SRC} ${OCR_Web_Service})
#    add_executable(benchmark src/PredString.cpp
#            src/AngleNet.cpp
#            src/clipper.cpp
#            src/CrnnNet.cpp
#            src/DbNet.cpp
#            src/getopt.cpp
#            src/OcrLite.cpp
#            src/OcrUtils.cpp
#            ReactorHttp-Cpp/ThreadPool.cpp
#            ReactorHttp-Cpp/Buffer.cpp
#            ReactorHttp-Cpp/Channel.cpp
#            ReactorHttp-Cpp/Dispatcher.cpp
#            ReactorHttp-Cpp/EpollDispatcher.cpp
#            ReactorHttp-Cpp/EventLoop.cpp
#            ReactorHttp-Cpp/HttpRequest.cpp
#            ReactorHttp-Cpp/Httpresponse.cpp
#            ReactorHttp-Cpp/PollDispatcher.cpp
#            ReactorHttp-Cpp/SelectDispatcher.cpp
#            ReactorHttp-Cpp/TcpConnection.cpp
#            ReactorHttp-Cpp/TcpServer.cpp
#            ReactorHttp-Cpp/WorkerThread.cpp
#            ReactorHttp-Cpp/main.cpp)
    target_link_libraries(benchmark ${OnnxRuntime_LIBS} ${OpenCV_LIBS} ${OpenMP_CXX_LIB_NAMES})
    target_compile_definitions(benchmark PRIVATE __EXEC__)

    install(TARGETS benchmark EXPORT benchmark
            ARCHIVE DESTINATION staticlib
            LIBRARY DESTINATION sharedlib
            RUNTIME DESTINATION bin)
endif ()
