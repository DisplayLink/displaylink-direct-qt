set(CORE_NAME "qdlsdk-core")

add_library(${CORE_NAME} STATIC
)

target_sources(${CORE_NAME} PRIVATE
    driver/types.hpp
    driver/interfaces.hpp
    display_manager.hpp
    display_manager.cpp
    single_display_controller.hpp
    single_display_controller.cpp
)

set_target_properties(${CORE_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${CORE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
