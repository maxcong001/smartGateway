

# Generate shared library by default.
SET(BUILD_SHARED_LIBS ON)

# Set default install dir
IF(NOT CMAKE_INSTALL_LIBDIR)
	SET(CMAKE_INSTALL_LIBDIR lib)
ENDIF()

MESSAGE(STATUS "Installation path prefix: " ${CMAKE_INSTALL_PREFIX})
MESSAGE(STATUS "Library installation directory: " ${CMAKE_INSTALL_LIBDIR})
MESSAGE(STATUS "PROJECT_SOURCE_DIR:" ${PROJECT_SOURCE_DIR})

# Where header files can be found.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/common)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/common/utility)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/common/app_common)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/common/RF24Driver)

# Include sub directory adapters and utils.
ADD_SUBDIRECTORY(server/http_server)
ADD_SUBDIRECTORY(server/RF24_server)

# Collect all source files.
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c)
#AUX_SOURCE_DIRECTORY(. SRC_LIST)
#AUX_SOURCE_DIRECTORY(./utils SRC_LIST_UTILS)

SET (REMOVE_LIST_FOR_SO
    server/http_server/http_server.cpp
	server/RF24_server/RF24_server.cpp
)
list(REMOVE_ITEM SOURCES ${REMOVE_LIST_FOR_SO})

# Not compile test files into library
IF(NOT ENABLE_TEST)
	SET (TEST_CPP_LIST
#		utils/MTestUtil.cpp
		stub.cpp
	)

	list(REMOVE_ITEM SOURCES ${TEST_CPP_LIST})
ENDIF()


# Generate shared library lib${PROJECT_NAME}.
#ADD_LIBRARY (${PROJECT_NAME} SHARED ${SRC_LIST} ${SRC_LIST_ADAPTERS}
#		${SRC_LIST_UTILS} ${SRC_LIST_ADAPTERS_REDIS})
ADD_LIBRARY (${PROJECT_NAME} SHARED ${SOURCES})

# Set library version
SET_TARGET_PROPERTIES(
		${PROJECT_NAME}
		PROPERTIES
		SOVERSION ${M_VERSION_MAJOR}
		VERSION ${M_VERSION}
)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)
IF(NOT ENABLE_TEST)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)
ELSE()
TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)
ENDIF()

# Install lib to /usr/local/lib by default
INSTALL(
		TARGETS ${PROJECT_NAME}
		LIBRARY
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install header files to /usr/local/include/${PROJECT_NAME}
INSTALL(
		DIRECTORY ${PROJECT_SOURCE_DIR}/include/
		DESTINATION include/${PROJECT_NAME}
)

# Install COPYRIGHT, README, bench
#INSTALL(FILES COPYRIGHT README.md DESTINATION share/doc/${PROJECT_NAME})
