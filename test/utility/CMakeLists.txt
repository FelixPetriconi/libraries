
################################################################################

add_executable( stlab.test.tuple
        tuple_test.cpp
        tuple_algorithm_test.cpp
        ../main.cpp )

target_link_libraries( stlab.test.tuple PUBLIC stlab::testing )
add_test( NAME stlab.test.tuple COMMAND stlab.test.tuple )

################################################################################

add_executable( stlab.test.traits
        traits_test.cpp
        ../main.cpp )

target_link_libraries( stlab.test.traits PUBLIC stlab::testing )
add_test( NAME stlab.test.traits COMMAND stlab.test.traits )

################################################################################
#
# tests are compiled without compiler extensions to ensure the stlab headers
# are not dependent upon any such extension.
#
set_target_properties(
  stlab.test.tuple
  stlab.test.traits
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
    stlab.test.tuple
    PROPERTIES PROCESSORS ${nProcessors})
endif()
