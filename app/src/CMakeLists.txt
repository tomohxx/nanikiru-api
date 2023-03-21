cmake_minimum_required(VERSION 3.1)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wextra -g -DCHECK_HAND -DCHECK_WALL")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra")
set(lib "${PROJECT_SOURCE_DIR}/necessary-and-unnecessary-tiles")
set(OPENSSL_USE_STATIC_LIBS ON)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar zxvf ${lib}/index_dw.tar.gz
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

find_package(OpenSSL REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json 3.11.2 REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)

include_directories(${lib})

add_executable(
  main
  main.cpp
  player_impl.cpp
  win_prob1.cpp
  win_prob2.cpp
  settile.cpp
  getlabel.cpp
  ${lib}/calsht_dw.cpp
)
target_compile_definitions(
  main
  PRIVATE SCHEMA_FILE_PATH="${PROJECT_SOURCE_DIR}/schema.json"
  PRIVATE INDEX_FILE_PATH="${PROJECT_BINARY_DIR}"
)
target_link_libraries(
  main
  PRIVATE cpprest
  PRIVATE pthread
  PRIVATE OpenSSL::Crypto
  PRIVATE spdlog::spdlog
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE nlohmann_json_schema_validator
)
