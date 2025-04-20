PROJECT(yijinjing LANGUAGES C CXX)

CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused -Wno-stringop-truncation")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

LIST(APPEND INCS "/root/anaconda3/include/python3.9"
                 ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/include/pybind11-2.2.4)

LIST(APPEND LNKS "/root/anaconda3/lib")
INCLUDE_DIRECTORIES(${INCS})
LINK_DIRECTORIES(${LNKS})

add_executable(writer-reader  writer-reader.cpp ${TARGET_CPP})
target_compile_options(writer-reader PRIVATE "-std=gnu++17")
target_link_libraries(writer-reader PRIVATE ${LIBS})

add_executable(writer  writer.cpp ${TARGET_CPP})
target_compile_options(writer PRIVATE "-std=gnu++17")
target_link_libraries(writer PRIVATE ${LIBS})

add_executable(reader  reader.cpp ${TARGET_CPP})
target_compile_options(reader PRIVATE "-std=gnu++17")
target_link_libraries(reader PRIVATE ${LIBS})

add_library(ShareMemory SHARED py_flash_store.cpp ${TARGET_CPP})
target_link_libraries(ShareMemory PRIVATE python3.9 stdc++fs)
