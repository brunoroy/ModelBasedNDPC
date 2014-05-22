#
# This cmake script will build a basic Houdini DSO.
#
cmake_minimum_required(VERSION 2.6)
set(PROJECT_NAME MokkoGridPatches)

############## SET ############
set(PROJECT_ROOT .)
set(PROJECT_SOURCE ${PROJECT_ROOT}/src)
set(PROJECT_INCLUDE ${PROJECT_ROOT}/include)

project(${PROJECT_NAME})

############# FIND PACKAGE ###############
find_package(HDK)

############## HOUDINI ###################
# run our helper process to grab the compile and link flags from hcustom
#execute_process(COMMAND hdk-config.py --notag --cmake=${CMAKE_BINARY_DIR}/FindHDK.cmake)
execute_process(COMMAND hdk-config.py --notag --cmake=/usr/share/cmake-2.8/Modules/FindHDK.cmake)

# now get the HDK definitions from the FindHDK.cmake module that was just generated
set(CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}")

################# SOURCE ##################
file(GLOB source ${PROJECT_SOURCE}/*.cpp ${PROJECT_SOURCE}/*.C ${PROJECT_EXTERNAL}/libsvm/*.cpp)
set(CMAKE_INSTALL_PREFIX "${HDK_HIH_DIR}")

# setup to compile in the correct HDK environment
add_definitions(${HDK_DEFINITIONS})
# Prevents redefinition of MIN and MAX macros in windows.h (which causes conflicts with eigen library)
add_definitions(-DNOMINMAX)
# support c++11 standard (on GNOME)
if (CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-std=gnu++0x)
endif(CMAKE_COMPILER_IS_GNUCXX)

########### INCLUDES #################
include_directories(${HDK_INCLUDE_DIRS})
include_directories(${PROJECT_ROOT})
include_directories(${PROJECT_INCLUDE})
include_directories(${PROJECT_SOURCE})

########## LINKS ######################s
link_directories(${HDK_LIBRARY_DIRS})

# this command calls hdk-config.py to build a special file that will be the repository of the taginfo
add_custom_command(OUTPUT sesitag.C
                   DEPENDS ${source}
                   COMMAND ../hdk-config.py --tagfile=${CMAKE_BINARY_DIR}/sesitag.C)

if (APPLE)
    set(LTYPE MODULE)
else (APPLE)
    set(LTYPE SHARED)
endif (APPLE)

# this tells cmake to buid the DSO
add_library(${PROJECT_NAME} ${LTYPE} ${source}  sesitag.C)

# houdini doesn't want the DSO to have "lib" as it's prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# unfortunately cmake's MODULE library type generates a ".so" file on OSX
if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dylib")
endif (APPLE)

#target_link_libraries (${ProjectName} ${FluidsCore})
target_link_libraries(
	${PROJECT_NAME} 
	${HDK_LIBRARIES}
)

# finally tell cmake where to put the results
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION dso)
install(FILES ${PROJECT_NAME}.png DESTINATION config/Icons)
