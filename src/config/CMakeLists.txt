MESSAGE(STATUS "start build config!!!")
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR})
set(MakefileInc ${PROJECT_SOURCE_DIR}/Makefile.inc)  
add_custom_command(OUTPUT ${MakefileInc}
	#COMMAND echo "exe config file"
	COMMAND ${PROJECT_SOURCE_DIR}/src/config/configure
	COMMENT "exe config file"
	DEPENDS "${PROJECT_SOURCE_DIR}/src/config/configure"
	)
add_custom_target(Test1 ALL DEPENDS ${MakefileInc})
