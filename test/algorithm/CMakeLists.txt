
################################################################################

add_executable( stlab.test.algorithm
        clamp_test.cpp
        median_test.cpp
        minmax_test.cpp
        select_test.cpp
        ../main.cpp )

target_link_libraries( stlab.test.algorithm PUBLIC stlab::testing )
add_test( NAME stlab.test.algorithm COMMAND stlab.test.algorithm )

################################################################################
#
# tests are compiled without compiler extensions to ensure the stlab headers
# are not dependent upon any such extension.
#
set_target_properties(
  stlab.test.algorithm
  PROPERTIES CXX_EXTENSIONS OFF )

#
# Many of the stlab tests are executed using the system executor which defaults
# to a number of threads equal to the system parallelism.
#
# Here we (attempt to) query the system processor count and store the value in
# the `nProcessors` variable, where a non-zeros value indicates success.
#
# Provided the query was successful, we inform ctest of the test parallism.
#

include(ProcessorCount)
ProcessorCount(nProcessors)

if(nProcessors)
  set_tests_properties(
    stlab.test.algorithm
    PROPERTIES PROCESSORS ${nProcessors})
endif()
