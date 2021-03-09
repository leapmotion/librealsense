string(REPLACE ${PROJECT_SOURCE_DIR}/ "" _rel_path ${CMAKE_CURRENT_LIST_DIR})
include(${_rel_path}/core/CMakeLists.txt)
include(${_rel_path}/linux/CMakeLists.txt)
include(${_rel_path}/res/CMakeLists.txt)

if(FORCE_LIBUVC)
    include(${_rel_path}/libuvc/CMakeLists.txt)
endif()

if(BUILD_FOR_OTHER_WIN)
    include(${_rel_path}/win/CMakeLists.txt)
endif()

if(ANDROID_USB_HOST_UVC)
    include(${_rel_path}/android/CMakeLists.txt)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

target_sources(${LRS_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/backend.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/driver_peripheral.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/driver_rigel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/log.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/realuvc_driver.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/types.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/videocapture.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/api.h"
        "${CMAKE_CURRENT_LIST_DIR}/backend.h"
        "${CMAKE_CURRENT_LIST_DIR}/concurrency.h"
        "${CMAKE_CURRENT_LIST_DIR}/leap_xu.h"
        "${CMAKE_CURRENT_LIST_DIR}/types.h"
		
        "${CMAKE_CURRENT_LIST_DIR}/win/win-helpers.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/win/win-uvc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/win/win-usb.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/win/win-hid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/win/win-backend.cpp"
)

target_link_libraries(${LRS_TARGET} PUBLIC ${OpenCV_LIBS})

if(TARGET OpenCV::highgui)
  add_executable(
      mytest
        "${CMAKE_CURRENT_LIST_DIR}/mytest.cpp"
  )

  target_link_libraries(
      mytest
          ${LRS_TARGET}
          ${OpenCV_LIBS}
  )

  add_executable(
      viewer
        "${CMAKE_CURRENT_LIST_DIR}/viewer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/option_parse.cpp"
  )

  target_link_libraries(
      viewer
          ${LRS_TARGET}
          OpenCV::highgui
  )
endif()


