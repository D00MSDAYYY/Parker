cmake_minimum_required(VERSION 3.27.0)
project(db_interactions)

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
)
