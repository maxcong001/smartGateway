FILE(GLOB_RECURSE RF24_NODE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c)


SET(RF24_NODE_TARGET _${RF24_NODE_BINARY})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

ADD_EXECUTABLE(${RF24_NODE_TARGET} ${RF24_NODE_SOURCES})

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)

TARGET_LINK_LIBRARIES(${RF24_NODE_TARGET} ${PROJECT_NAME} pthread event log4cplus ncurses)

SET_TARGET_PROPERTIES(${RF24_NODE_TARGET} PROPERTIES OUTPUT_NAME ${RF24_NODE_BINARY})

ADD_DEFINITIONS(-DTEST)