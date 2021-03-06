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
)

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

if(WIN32)
	find_library(WINSOCK2 "ws2_32.lib")
	target_link_libraries(uhttp-shared ${WINSOCK2})
	target_link_libraries(uhttp-static ${WINSOCK2})
endif()

