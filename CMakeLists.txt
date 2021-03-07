cmake_minimum_required(VERSION 3.5)

project(
	"uHTTP" VERSION 0.1
	DESCRIPTION "Compact HTTP server software over (win/bsd)sockets."
	LANGUAGES "C"
)

set(
	UHTTP_SOURCES
	"src/server.c"
	"src/client.c"
	"src/list.c"
	"src/winsock.c")

add_library(
uhttp-shared
	SHARED ${UHTTP_SOURCES}
)
target_include_directories(uhttp-shared PUBLIC "inc" PRIVATE "src")

add_library(
	uhttp-static
	STATIC ${UHTTP_SOURCES}
)
target_include_directories(uhttp-static PUBLIC "inc" PRIVATE "src")

add_executable(
	uhttp-cli
	${UHTTP_SOURCES}
	"src/cli/main.c"
)
target_include_directories(uhttp-cli PRIVATE "inc" "src")

if(WIN32)
	find_library(WINSOCK2 "ws2_32.lib")
	target_link_libraries(uhttp-shared ${WINSOCK2})
	target_link_libraries(uhttp-static ${WINSOCK2})
	target_link_libraries(uhttp-cli ${WINSOCK2})
endif()

add_subdirectory("test")
