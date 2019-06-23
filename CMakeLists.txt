cmake_minimum_required(VERSION 3.10)
project(ft_ping)

#Setup binary compilation
add_executable(ft_ping
		srcs/ft_ping.c
		srcs/init_network.c
		srcs/loop.c
		srcs/headers.c
		srcs/opt.c
		srcs/display.c)
target_include_directories(ft_ping PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_compile_options(ft_ping PRIVATE -Wall -Wextra -Werror -std=gnu11 -march=native -flto)
target_link_libraries(ft_ping m)