add_executable(psychicui_playground
    playground.cpp
)
target_include_directories(psychicui_playground PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${YOGA_INCLUDE_DIRS}
    ${SKIA_INCLUDE_DIRS}
    ${RXCPP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../psychicui/signals/
)
#target_link_libraries(psychicui_playground psychicui ${PSYCHICUI_EXTRA_LIBS})