include(FindPackageHandleStandardArgs)

find_path(mlpack_INCLUDE_DIRS
	NAMES "mlpack/core.hpp"
)

find_library(mlpack_LIBRARIES mlpack)

find_package_handle_standard_args(mlpack DEFAULT_MSG 
    mlpack_INCLUDE_DIRS
    mlpack_LIBRARIES
)
