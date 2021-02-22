    if (WIN32)
        include(CMake/windows_config.cmake)
    endif()

    if(UNIX)
        include(CMake/unix_config.cmake)
    endif()

    if(ANDROID_NDK_TOOLCHAIN_INCLUDED)
        set(ANDROID_USB_HOST_UVC ON)
        include(CMake/android_config.cmake)
    endif()
