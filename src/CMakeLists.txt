cmake_minimum_required(VERSION 3.27.0)
project(Parker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(3rdParty)
add_subdirectory(db_interactions)

add_executable(${PROJECT_NAME} )

target_sources(${PROJECT_NAME} 
PRIVATE
	main.cpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
	TgBot
	db_interactions
)
