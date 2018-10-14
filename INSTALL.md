 # Software Technology Lab Concurrency Library
 
 ## Requirements
 * C++ 17 compatible compiler
    * Visual Studio 2017 Update 5 or later
    * Clang 3.9 or later
    * GCC 7 or later
    
 * boost.test (only for the unit tests)
    
 ## Configuration Options
 The concurrency library is a header only library. Add ./ to the include path.
  
 Under certain conditions, it uses different implementations for optional in the following order:
 * boost::optional, if the define STLAB_FORCE_BOOST_OPTIONAL is set, or
 * std::optional, if C++17 is enabled and std::optional is available, or
 * std::experimental::optional, if it is available, otherwise
 * boost::optional
 
 Under certain conditions, it uses different implementations for variant in the following order:
  * boost::variant, if the define STLAB_FORCE_BOOST_VARIANT is set, or
  * std::variant, if C++17 is enabled and std::variant is available, or
  * boost::variant
  
  

 
 