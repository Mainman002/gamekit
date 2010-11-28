INCLUDE(Platform/Generic-ADSP-Common)

SET(CMAKE_CXX_OUTPUT_EXTENSION ".doj")

SET(CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
SET(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "")
SET(CMAKE_CXX_FLAGS_RELEASE_INIT "")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "")

SET(CMAKE_CXX_CREATE_STATIC_LIBRARY 
     "<CMAKE_CXX_COMPILER> -build-lib -proc ${ADSP_PROCESSOR} -si-revision ${ADSP_PROCESSOR_SILICIUM_REVISION} -o <TARGET> <CMAKE_CXX_LINK_FLAGS> <OBJECTS>")

SET(CMAKE_CXX_LINK_EXECUTABLE
     "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

SET(CMAKE_CXX_CREATE_SHARED_LIBRARY)
SET(CMAKE_CXX_CREATE_MODULE_LIBRARY)
