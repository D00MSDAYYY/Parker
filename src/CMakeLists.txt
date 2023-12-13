cmake_minimum_required(VERSION 3.27.0)
project(Parker)

add_subdirectory(3rdParty)

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
)
