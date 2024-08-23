cmake_minimum_required(VERSION 3.5)
project(smplog CXX)

macro(remove_warnings)
    add_definitions(-Wno-unused-value -Wno-unused-but-set-variable 
                    -Wno-unused-variable 
                    -Wno-unused-function)
endmacro()

remove_warnings()

set(CMAKE_CXX_STANDARD 20)
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})


set(ARM64_ENV false)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
	set(ARM64_ENV true)
	message(STATUS "system processor is aarch64")
else()
	message(STATUS "system processor is not aarch64")
endif()


set(deps_root ${PROJECT_SOURCE_DIR}/3rd)
set(3rd_include_root "/opt/xx/")

set(xx_libs "/opt/xx/")


#set lib-variable (platform-dependency library path)
if (WIN32 AND NOT MINGW)  
	message("Now is windows")
	set(sqlite_lib_path ${sqlite_lib_path}/${CMAKE_BUILD_TYPE})
elseif (UNIX)
	set(sqlite_lib_path ${xx_libs})
	set(sqlite_lib_path ${xx_libs}/sqlite/lib)
    message("Now is UNIX-like OS's. Including aPPLE os x  and CygWin")  
else()
	message(FATAL_ERROR "Unkown platform")
endif() 

#set compiler option

if(ARM64_ENV)
	if(MSVC)
		message(STATUS "Compiler msvc!")
	else(MSVC)
		if(CMAKE_BUILD_TYPE MATCHES "Release")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fPIC -Wno-unused-function -Wno-unused-value ")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC -Wno-unused-function -Wno-unused-value " )
		else(CMAKE_BUILD_TYPE MATCHES "Debug")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAG} -O0 -g Wall- -fPIC -Wno-unused-function -Wno-unused-value ")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} -O0 -g -Wall -fPIC -Wno-unused-function -Wno-unused-value ")
		endif(CMAKE_BUILD_TYPE MATCHES "Release")
	endif(MSVC)
else(ARM64_ENV)
	if(MSVC)
		message(STATUS "Compiler msvc!")
	else(MSVC)
		if(CMAKE_BUILD_TYPE MATCHES "Release")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -O3 -fPIC")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -O3 -fPIC" )
		else(CMAKE_BUILD_TYPE MATCHES "Debug")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAG} -O0 -m64 -g Wall- -fPIC")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} -m64 -O0 -g -Wall -fPIC")
		endif(CMAKE_BUILD_TYPE MATCHES "Release")
	endif(MSVC)

endif(ARM64_ENV)

message("gcc compiler option:")
message("CXX:${CMAKE_CXX_FLAGS}")
message("C:${CMAKE_C_FLAGS}")
message("(MSVC)Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message("(GCC)Generated with config types: ${CMAKE_BUILD_TYPE}")

# set compiler include search directory
include_directories(
    inc
	${3rd_include_root}/sqlite/include
	)
if(WIN32)
	include_directories(${deps_root}/libevent/include/windows)
else()
	include_directories(${3rd_include_root}/libevent/include)
endif()
	include_directories(${3rd_include_root}/libevent/include)

# set linker library search directory
#if(WIN32)
link_directories(
	${sqlite_lib_path}
)
#endif()


option(BUILD_EXAMPLE "Build and run example." OFF)

############IO.Net############
set(IO_Common_Headers
	inc/common/global.h
	inc/common/util.h
	)

set(IO_Common_Srcs
	src/common/util.cpp)

set(IO_Threadpool_Headers )

set(IO_Log_Headers
	inc/logging/log_mgr.h
	inc/logging/ilog.h
	inc/logging/file_log.h
	inc/logging/csv_log.h
	inc/logging/sqlite_log.h
	)

set(IO_Log_Srcs
	src/logging/log_mgr.cpp
	src/logging/file_log.cpp
	src/logging/csv_log.cpp
	src/logging/sqlite_log.cpp
	src/logging/file_log_mgr.cpp
	)


		
	
set(IO_Headers inc/io/io_director.h)

set(IO_Srcs src/io/io_director.cpp)

set(Security_Headers )

set(Security_Srcs )

############IO.Net############
set(smplog_SRCS 
	${IO_Common_Headers} ${IO_Common_Srcs}
	${CXX_Threading_Headers}
	${IO_Log_Headers} ${IO_Log_Srcs}
	${IO_Headers} ${IO_Srcs}
)

message(STATUS "smplog_SRCS:${smplog_SRCS}")


ADD_LIBRARY(smplog_static STATIC ${smplog_SRCS})
# 对smplog_static的重名为smplog
SET_TARGET_PROPERTIES(smplog_static PROPERTIES  OUTPUT_NAME "smplog")
SET_TARGET_PROPERTIES(smplog_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
ADD_LIBRARY(smplog SHARED ${smplog_SRCS})
# 对smplog_static的重名为smplog
SET_TARGET_PROPERTIES(smplog PROPERTIES  OUTPUT_NAME "smplog")
SET_TARGET_PROPERTIES(smplog PROPERTIES CLEAN_DIRECT_OUTPUT 1)



target_compile_options(smplog
PRIVATE
-DCXX_DLLEXPORT)
if(WIN32)
	target_compile_options(smplog
	PRIVATE
	-DWIN32)
endif()

if(MSVC)
	set(PlatformLib 
	ws2_32
	SQLiteCpp
	sqlite3
	)
else(MSVC)
	set(PlatformLib 
	sqlite3
	SQLiteCpp
	dl
	pthread
	)
endif(MSVC)

target_link_libraries(smplog ${PlatformLib})
                      
if(BUILD_EXAMPLE)
	add_executable(smplog_test example/test.cpp)
	target_link_libraries(smplog_test smplog)
endif(BUILD_EXAMPLE)
