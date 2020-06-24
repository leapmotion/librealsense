# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# View the makefile commands during build
#set(CMAKE_VERBOSE_MAKEFILE on)

include(GNUInstallDirs)
# include librealsense helper macros
include(CMake/lrs_macros.cmake)
include(CMake/version_config.cmake)
include(CMake/lrs_options.cmake)

set(BUILD_EASYLOGGINGPP OFF)

if(ENABLE_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
      set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif(CCACHE_FOUND)
endif()

macro(global_set_flags)
    set(LRS_TARGET librealuvc)
    set(LRS_LIB_NAME ${LRS_TARGET})

    add_definitions(-DELPP_THREAD_SAFE)
    add_definitions(-DELPP_NO_DEFAULT_LOG_FILE)

    if (ENABLE_ZERO_COPY)
        add_definitions(-DZERO_COPY)
    endif()

    if (BUILD_EASYLOGGINGPP)
        add_definitions(-DBUILD_EASYLOGGINGPP)
    endif()

    if(TRACE_API)
        add_definitions(-DTRACE_API)
    endif()

    if(HWM_OVER_XU)
        add_definitions(-DHWM_OVER_XU)
    endif()

    if (ENFORCE_METADATA)
      add_definitions(-DENFORCE_METADATA)
    endif()

    if(FORCE_LIBUVC)
        set(BACKEND RS2_USE_LIBUVC_BACKEND)
        message( WARNING "Using libuvc!" )
    endif()

    add_definitions(-D${BACKEND} -DUNICODE)
endmacro()

function(target_link_libraries_foreach)
  cmake_parse_arguments(
    LIBREALUVC
      ""
      "TARGET;SCOPE"
      "LIBS"
    ${ARGN})

  message(STATUS "target_link_libraries: ${LIBREALUVC_TARGET} ${LIBREALUVC_SCOPE} ${LIBREALUVC_LIBS}")
  foreach(target in ${LIBREALUVC_TARGET} ${LIBREALUVC_TARGET}_static ${LIBREALUVC_TARGET}_shared)
    if (TARGET ${target})
      target_link_libraries(${target} ${LIBREALUVC_SCOPE} ${LIBREALUVC_LIBS})
    endif()
  endforeach()
endfunction()

macro(global_target_config)
    target_link_libraries_foreach(TARGET ${LRS_TARGET} SCOPE PRIVATE LIBS ${CMAKE_THREAD_LIBS_INIT} ${TRACKING_DEVICE_LIBS})

    include_directories(${LRS_TARGET} src)

    set_target_properties (${LRS_TARGET} PROPERTIES FOLDER Library)

    target_include_directories(${LRS_TARGET}
        PRIVATE
            # ${ROSBAG_HEADER_DIRS}
            # ${BOOST_INCLUDE_PATH}
            # ${LZ4_INCLUDE_PATH}
            # ${LIBUSB_LOCAL_INCLUDE_PATH}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
            PRIVATE ${USB_INCLUDE_DIRS}
    )
endmacro()

macro(add_OpenCV)
    message(STATUS "Building with OpenCV")
	find_package(OpenCV REQUIRED)
    target_compile_definitions(${LRS_TARGET} PRIVATE WITH_OPENCV=1)
    target_link_libraries_foreach(TARGET ${LRS_TARGET} SCOPE PUBLIC LIBS ${OpenCV_LIBS})
    target_include_directories(${LRS_TARGET} PUBLIC ${OpenCV_INCLUDE_DIR})
endmacro()

macro(add_tm2)
    message(STATUS "Building with TM2")
    add_subdirectory(third-party/libtm)
    if(USE_EXTERNAL_USB)
        add_dependencies(tm libusb)
    endif()
    target_compile_definitions(${LRS_TARGET} PRIVATE WITH_TRACKING=1 BUILD_STATIC=1)
    target_link_libraries_foreach(TARGET ${LRS_TARGET} SCOPE PRIVATE LIBS tm ${CMAKE_THREAD_LIBS_INIT} ${TRACKING_DEVICE_LIBS})
    target_include_directories(${LRS_TARGET} PRIVATE third-party/libtm/libtm/include)
endmacro()
