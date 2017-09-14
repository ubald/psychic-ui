add_executable(psychic-ui-playground
    playground.cpp
)
target_include_directories(psychic-ui-playground PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${YOGA_INCLUDE_DIRS}
    ${SKIA_INCLUDE_DIRS}
    ${RXCPP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../psychic-ui/signals/
)
