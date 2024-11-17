# Version
cmake_minimum_required(VERSION 3.10)
# Project
project(vulkan_app)

enable_language(OBJCXX)

# Set the bundle and app name
set(APP_NAME ${PROJECT_NAME})
# Set sources
set(SOURCE_FILES main.mm AAPLViewController.mm AAPLViewController.h AAPLRenderer.mm AAPLRenderer.h AAPLAppDelegate.mm AAPLAppDelegate.h)
# Set bundle ids and targets
set(APP_BUNDLE_IDENTIFIER "com.test.vulkanapp")
set(DEPLOYMENT_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}")
set(MY_CODE_SIGN_IDENTITY "Apple Development")
set(DEVELOPMENT_TEAM_ID "XXXXXXXXXX") # Set your team id here or in Xcode
set(DEVICE_FAMILY "1")  # 1 for iOS

set(FRAMEWORK_NAME "Framework")
set(PRODUCT_NAME ${APP_NAME})
set(EXECUTABLE_NAME ${APP_NAME})

set(CMAKE_MACOSX_BUNDLE YES)

add_executable(
    ${APP_NAME}
    MACOSX_BUNDLE
    ${SOURCE_FILES}
)

set_target_properties("${APP_NAME}"
    PROPERTIES
    MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_NAME}
    MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_BUNDLE_NAME ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_ICON_FILE ""
    MACOSX_BUNDLE_LONG_VERSION_STRING "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_COPYRIGHT "Copyright YOU"
    MACOSX_BUNDLE_SIGNATURE ${APP_BUNDLE_IDENTIFIER}
    MACOSX_DEPLOYMENT_TARGET "${DEPLOYMENT_TARGET}")

# Turn on ARC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")

# Find Vulkan
find_package(Vulkan REQUIRED COMPONENTS MoltenVK)

set(FRAMEWORKS_TO_EMBED)
if(Vulkan_MoltenVK_LIBRARY)
    list(APPEND FRAMEWORKS_TO_EMBED "${Vulkan_MoltenVK_LIBRARY};")
else()
    message(FATAL_ERROR "Can't find MoltenVK library. Please install the Vulkan SDK or MoltenVK project and set VULKAN_SDK.")
endif()
if(Vulkan_LIBRARY)
    list(APPEND FRAMEWORKS_TO_EMBED "${Vulkan_LIBRARY};")
endif()
if(Vulkan_Layer_VALIDATION)
    # trouble is can't turn this on/off if XCode decides to build debug and we're configured for release.  Need to revist
    # note the Vulkan validation layer must be present and enabled even in release mode for the shader_debugprintf sample
    #if(("${VKB_DEBUG}" STREQUAL "ON") OR ("${VKB_VALIDATION_LAYERS}" STREQUAL "ON"))
    list(APPEND FRAMEWORKS_TO_EMBED "${Vulkan_Layer_VALIDATION}")
    #endif()
endif()

# Create the app target
# add these later RESOURCE "${RESOURCES}"
set_target_properties("${APP_NAME}"
                      PROPERTIES
                      XCODE_EMBED_FRAMEWORKS  "${FRAMEWORKS_TO_EMBED}"
                      XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${MACOSX_BUNDLE_GUI_IDENTIFIER}"
                      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
                      XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
                      XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${DEPLOYMENT_TARGET}"
                      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${MY_CODE_SIGN_IDENTITY}"
                      XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
                      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM_ID}"
                      XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "${DEVICE_FAMILY}"
                      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
                      XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
                      XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES NO
                      XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
                      XCODE_ATTRIBUTE_ENABLE_TESTABILITY YES
                      XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
                      XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})

# Set the app's linker search path to the default location on iOS
set_target_properties(
    ${APP_NAME}
    PROPERTIES
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
    "@executable_path/Frameworks"
)

# Set initial default storyboards
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/LaunchScreen.storyboard
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.storyboard
)
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/LaunchScreen.storyboard
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.storyboard
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)

# Include Vulkan headers
include_directories(${Vulkan_INCLUDE_DIRS})

# Link system frameworks
target_link_libraries(${APP_NAME} PRIVATE
    "${Vulkan_LIBRARIES}"
    "-framework UIKit"
    "-framework Foundation"
    "-framework CoreGraphics"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
    "-framework IOKit"
    "-framework IOSurface"
)

# Turn on code signing for iOS
if(IOS)
    message(STATUS "Building for iOS device or iOS Simulator with a code signing identity; turning on code signing")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "YES")
    set_target_properties(${APP_NAME} PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED            "YES"
        XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY		"YES"
    )
endif()
