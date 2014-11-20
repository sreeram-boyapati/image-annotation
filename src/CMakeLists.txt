project(src)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

#To add OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

#To add boost
find_package(Boost 1.54.0 COMPONENTS system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

#To add openmp
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

SET(CMAKE_EXE_LINKER_FLAGS, "-g")
SET(PROJECT_LIBRARIES segment_images.cpp surffeatures.cpp mongo_helper_funcs.cpp kmeans_wrapper.cpp segment_images.h surffeatures.h mongo_helper_funcs.h kmeans_wrapper.h)
SET(SOURCES main.cpp ${PROJECT_LIBRARIES})
SET(TESTING testing.cpp ${PROJECT_LIBRARIES})

add_executable(main ${SOURCES})
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main ${Boost_LIBRARIES})
target_link_libraries(main "/usr/local/lib/libmongoclient.so")

add_executable(testing ${TESTING})
target_link_libraries(testing ${OpenCV_LIBS})
target_link_libraries(testing ${Boost_LIBRARIES})
target_link_libraries(testing "/usr/local/lib/libmongoclient.so")

