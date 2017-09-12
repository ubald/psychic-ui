option(PSYCHICUI_BUILD_EXAMPLE "Build PsychicUI example application?" ON)
add_feature_info("example" PSYCHICUI_BUILD_EXAMPLE "Example Application")

if (PSYCHICUI_BUILD_EXAMPLE)
    add_executable(psychicui_demo
        demo.cpp demo.cpp
        demo-stylesheet.hpp
        components/Application.hpp
        components/ToolBar.cpp
        components/ToolBar.hpp
        components/Labels.hpp
        components/Buttons.hpp
        demo.hpp components/Ranges.hpp components/Divs.hpp components/Scroller.hpp)
    target_link_libraries(psychicui_demo psychicui ${PSYCHICUI_EXTRA_LIBS})
endif ()
