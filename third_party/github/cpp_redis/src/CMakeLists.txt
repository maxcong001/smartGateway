# Generate shared library by default.
SET(BUILD_SHARED_LIBS ON)
SET(LIBRARY_NAME_CPP_REDIS cpp_redis)

# Set default install dir
IF(NOT CMAKE_INSTALL_LIBDIR)
	SET(CMAKE_INSTALL_LIBDIR lib)
ENDIF()

MESSAGE(STATUS "Installation path prefix: " ${CMAKE_INSTALL_PREFIX})
MESSAGE(STATUS "Library installation directory: " ${CMAKE_INSTALL_LIBDIR})
MESSAGE(STATUS "PROJECT_SOURCE_DIR:" ${PROJECT_SOURCE_DIR})

# Where DB Wrapper's header files can be found.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/github/cpp_redis/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/github/cpp_redis/include/network)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/github/cpp_redis/include/builders)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/github/cpp_redis/include/tacopie)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/github/cpp_redis/include/tacopie/utils)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/github/cpp_redis/include/tacopie/network)
# Include sub directory adapters and utils.
#ADD_SUBDIRECTORY(utils)

# Collect all source files.
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c)
#AUX_SOURCE_DIRECTORY(. SRC_LIST)
#AUX_SOURCE_DIRECTORY(./utils SRC_LIST_UTILS)

# Not compile test files into library
IF(NOT ENABLE_TEST)
	SET (TEST_CPP_LIST
#		utils/DBWTestUtil.cpp
		stub.cpp
	)

	list(REMOVE_ITEM SOURCES ${TEST_CPP_LIST})
ENDIF()


# Generate shared library lib${PROJECT_NAME}.
#ADD_LIBRARY (${PROJECT_NAME} SHARED ${SRC_LIST} ${SRC_LIST_ADAPTERS}
#		${SRC_LIST_UTILS} ${SRC_LIST_ADAPTERS_REDIS})
ADD_LIBRARY (${LIBRARY_NAME_CPP_REDIS} SHARED ${SOURCES})

# Set library version
SET_TARGET_PROPERTIES(
		${LIBRARY_NAME_CPP_REDIS}
		PROPERTIES
		SOVERSION ${M_VERSION_MAJOR}
		VERSION ${M_VERSION}
)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)
IF(NOT ENABLE_TEST)
TARGET_LINK_LIBRARIES(${LIBRARY_NAME_CPP_REDIS} event event_pthreads tacopie)
ELSE()
TARGET_LINK_LIBRARIES(${LIBRARY_NAME_CPP_REDIS} event event_pthreads tacopie)
ENDIF()

# Install libdbwraper to /usr/local/lib by default
INSTALL(
		TARGETS ${LIBRARY_NAME_CPP_REDIS}
		LIBRARY
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install DB Wrapper's header files to /usr/local/include/${PROJECT_NAME}
INSTALL(
		DIRECTORY ${PROJECT_SOURCE_DIR}/third_party/github/cpp_redis/include/
		DESTINATION include/${PROJECT_NAME}
)

# Install COPYRIGHT, README, bench
#INSTALL(FILES COPYRIGHT README.md DESTINATION share/doc/${PROJECT_NAME})
