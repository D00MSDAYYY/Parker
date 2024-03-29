﻿cmake_minimum_required(VERSION 3.27.0)
project(Parker)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(3rdParty)

set(PostgreSQL_ROOT "/opt/homebrew/opt/postgresql@16")
find_package(PostgreSQL REQUIRED)

add_executable(${PROJECT_NAME} )

target_sources(${PROJECT_NAME} 
PUBLIC 
	FSM_states.hpp
	email.hpp
	data_base.hpp
	excel.hpp

PRIVATE
	string_trimming.hpp
	main.cpp
	email.cpp
	data_base.cpp
	excel.cpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
	TgBot
	mailio
	OpenXLSX::OpenXLSX
	PostgreSQL::PostgreSQL
)
