cmake_minimum_required(VERSION 3.10)
project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

#set(CMAKE_PREFIX_PATH CACHE STRING "Directory with installed Protobuf")
#if(NOT CMAKE_PREFIX_PATH)
#    message(FATAL_ERROR "Please, specify Protobuf directory via -DCMAKE_PREFIX_PATH=<dir>")
#endif()
set(CMAKE_PREFIX_PATH D:/Projects_Cpp/yandex-cpp-mentor/Protobuf/package) # где установлен protobuf
#message(STATUS "CMAKE_PREFIX_PATH dir is ${CMAKE_PREFIX_PATH}, change via -DCMAKE_PREFIX_PATH=<dir>")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto transport_router.proto)

set(JSON_FILES json.h json.cpp json_builder.h json_builder.cpp json_reader.h json_reader.cpp)
set(HANDLER_FILES request_handler.h request_handler.cpp)
set(DOMAIN_FILES domain.h domain.cpp geo.h geo.cpp)
set(CATALOGUE_FILES transport_catalogue.h transport_catalogue.cpp)
set(RENDER_FILES map_renderer.h map_renderer.cpp svg.h svg.cpp)
set(ROUTER_FILES transport_router.h transport_router.cpp router.h graph.h)
set(SERIALIZATOR_FILES serialization.h serialization.cpp ranges.h)
set(PROTO_FILES transport_catalogue.proto svg.proto map_renderer.proto transport_router.proto)

set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../tests")
set(TEST_FILES ${TESTS_DIR}/testrunner.h ${TESTS_DIR}/testrunner.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} main.cpp ${JSON_FILES} ${HANDLER_FILES}
    ${DOMAIN_FILES} ${CATALOGUE_FILES} ${RENDER_FILES} ${ROUTER_FILES} ${SERIALIZATOR_FILES} ${PROTO_FILES} ${TEST_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
