cmake_minimum_required(VERSION 3.10)
project(qoireader)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RELEASE)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-Wall -Wextra -Werror )#-Wconversion -Wpedantic)
endif()



#set SOURCES to all cpp files in src
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")


add_executable(qoireader "${PROJECT_SOURCE_DIR}/src/main.cpp" ${SOURCES})

target_include_directories(qoireader
        PUBLIC ${PROJECT_SOURCE_DIR}/include/
        PRIVATE $(PROJECT_SOURCE_DIR)/src
)




set_target_properties(qoireader PROPERTIES
    OUTPUT_NAME "run"
)

set_target_properties( PROPERTIES VERSION 1.0 SOVERSION 1)


#install
set(QOIREADER_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS qoireader
	RUNTIME DESTINATION ${QOIREADER_DIR}
)


