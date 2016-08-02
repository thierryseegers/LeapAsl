include(FindPackageHandleStandardArgs)

find_path(KenLM_INCLUDE_DIRS
	NAMES "lm/model.hh"
	PATH_SUFFIXES kenlm
)

find_library(KenLM_LIBRARIES kenlm)

find_package_handle_standard_args(KenLM DEFAULT_MSG 
    KenLM_INCLUDE_DIRS
    KenLM_LIBRARIES
)
