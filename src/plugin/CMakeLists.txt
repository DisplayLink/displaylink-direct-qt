set(PLUGIN_NAME "qdlsdk")
set(PLUGIN_IMPL_NAME "qdlsdk-plugin-impl")

add_library(${PLUGIN_IMPL_NAME} STATIC
    bridge.hpp
    bridge.cpp
    logging.hpp
    logging.cpp
    screen.hpp
    screen.cpp
    integration.hpp
    integration.cpp
    dirty_rects.hpp
    dirty_rects.cpp
)

target_include_directories(${PLUGIN_IMPL_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PLUGIN_IMPL_NAME} PUBLIC
    qdlsdk-core
    dlsdk::dlsdk
    Qt6::Core
    Qt6::CorePrivate
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::FbSupportPrivate
    Qt6::InputSupportPrivate
)

set_target_properties(${PLUGIN_IMPL_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    AUTOMOC ON
    AUTORCC ON
)

qt_add_plugin(${PLUGIN_NAME} SHARED)

target_sources(${PLUGIN_NAME} PRIVATE
    dlsdk.json
    main.cpp
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${PLUGIN_IMPL_NAME}
)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    AUTOMOC ON
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/plugins/platforms"
)

install(TARGETS ${PLUGIN_NAME}
    LIBRARY DESTINATION ${QDLSDK_PLUGIN_INSTALL_DIR}
    RUNTIME DESTINATION ${QDLSDK_PLUGIN_INSTALL_DIR}
)
