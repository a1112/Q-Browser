add_executable(tst_host_runtime_config tst_host_runtime_config.cpp)
add_executable(tst_audio_broker tst_audio_broker.cpp)
target_link_libraries(tst_audio_broker PRIVATE q_browser_host Qt6::Test Qt6::Widgets)
target_compile_definitions(tst_audio_broker PRIVATE Q_BROWSER_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
q_browser_enable_strict_warnings(tst_audio_broker)
q_browser_add_qt_test(NAME audio_broker TARGET tst_audio_broker)

target_link_libraries(tst_host_runtime_config
  PRIVATE
    q_browser_host
    q_browser_package
    Qt6::Test)

if(WIN32)
  target_link_libraries(tst_host_runtime_config PRIVATE Advapi32)
endif()

q_browser_enable_strict_warnings(tst_host_runtime_config)
q_browser_add_qt_test(NAME host_runtime_config TARGET tst_host_runtime_config)

add_executable(tst_host_capability_runtime tst_host_capability_runtime.cpp)
target_link_libraries(tst_host_capability_runtime
  PRIVATE
    q_browser_host
    Qt6::Network
    Qt6::Test
    Qt6::Widgets)
target_compile_definitions(tst_host_capability_runtime
  PRIVATE
    Q_BROWSER_BROKER_TESTING)
q_browser_enable_strict_warnings(tst_host_capability_runtime)
q_browser_add_qt_test(NAME host_capability_runtime TARGET tst_host_capability_runtime)

add_executable(tst_browser_tab_model tst_browser_tab_model.cpp)
target_link_libraries(tst_browser_tab_model
  PRIVATE
    q_browser_host
    Qt6::Test)
q_browser_enable_strict_warnings(tst_browser_tab_model)
q_browser_add_qt_test(NAME browser_tab_model TARGET tst_browser_tab_model)

add_executable(tst_browser_window_geometry tst_browser_window_geometry.cpp)
target_link_libraries(tst_browser_window_geometry
  PRIVATE
    q_browser_host
    Qt6::Test)
q_browser_enable_strict_warnings(tst_browser_window_geometry)
q_browser_add_qt_test(
  NAME browser_window_geometry
  TARGET tst_browser_window_geometry)

add_executable(tst_browser_session_store tst_browser_session_store.cpp)
target_link_libraries(tst_browser_session_store
  PRIVATE
    q_browser_host
    Qt6::Test)
if(WIN32)
  target_link_libraries(tst_browser_session_store PRIVATE Advapi32)
endif()
q_browser_enable_strict_warnings(tst_browser_session_store)
q_browser_add_qt_test(NAME browser_session_store TARGET tst_browser_session_store)

add_executable(tst_new_tab_page tst_new_tab_page.cpp)
target_link_libraries(tst_new_tab_page
  PRIVATE
    q_browser_host
    Qt6::Test
    Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::Widgets)
q_browser_enable_strict_warnings(tst_new_tab_page)
q_browser_add_qt_test(NAME new_tab_page TARGET tst_new_tab_page)

add_executable(tst_demo_gallery tst_demo_gallery.cpp)
target_link_libraries(tst_demo_gallery PRIVATE q_browser_host Qt6::Test Qt6::Widgets)
q_browser_enable_strict_warnings(tst_demo_gallery)
q_browser_add_qt_test(NAME demo_gallery TARGET tst_demo_gallery)

add_executable(tst_performance_monitor tst_performance_monitor.cpp)
target_link_libraries(tst_performance_monitor PRIVATE q_browser_host Qt6::Test Qt6::Widgets)
q_browser_enable_strict_warnings(tst_performance_monitor)
q_browser_add_qt_test(NAME performance_monitor TARGET tst_performance_monitor)

add_executable(tst_browser_chrome tst_browser_chrome.cpp)
add_executable(tst_qml_site_loader tst_qml_site_loader.cpp)
target_link_libraries(tst_qml_site_loader PRIVATE q_browser_host Qt6::Test Qt6::Network)
q_browser_enable_strict_warnings(tst_qml_site_loader)
q_browser_add_qt_test(NAME qml_site_loader TARGET tst_qml_site_loader)
target_link_libraries(tst_browser_chrome
  PRIVATE
    q_browser_host
    Qt6::Test
    Qt6::Widgets)
q_browser_enable_strict_warnings(tst_browser_chrome)
q_browser_add_qt_test(NAME browser_chrome TARGET tst_browser_chrome)
