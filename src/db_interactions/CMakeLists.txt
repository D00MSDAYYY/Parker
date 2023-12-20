cmake_minimum_required(VERSION 3.27.0)

project(db_interactions)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PostgreSQL_ROOT "/opt/homebrew/opt/postgresql@16")
find_package(PostgreSQL REQUIRED)

add_library(${PROJECT_NAME} )

target_sources(${PROJECT_NAME} 
PUBLIC 
	db_interactions.hpp
PRIVATE
	db_interactions.cpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
	TgBot
	PostgreSQL::PostgreSQL
)




