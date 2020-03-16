target_sources( stlab INTERFACE
        $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}clamp.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}median.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}minmax.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}select.hpp
        $<INSTALL_INTERFACE:
        include/stlab/algorithm/clamp.hpp
        include/stlab/algorithm/median.hpp
        include/stlab/algorithm/minmax.hpp
        include/stlab/algorithm/select.hpp>)


#target_sources(stlab INTERFACE
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/concurrency/channel.hpp>
#        $<INSTALL_INTERFACE:include/stlab/concurrency/channel.hpp>)
