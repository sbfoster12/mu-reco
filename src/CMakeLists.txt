# Glob source files
file(GLOB SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/wfd5/*.cc
)

# Set reco sources
set(RECO_SOURCES
    ${SOURCE_FILES}
)

# Make an object file
add_library(reco SHARED
  ${RECO_SOURCES}
)

# Make an alias for the reco library
add_library(Reco::reco ALIAS reco)

# Include directories
target_include_directories(reco
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${ROOT_INCLUDE_DIRS}
)

# Libraries
target_link_libraries(reco
    PUBLIC
        nlohmann_json::nlohmann_json  
        ${ZLIB_LIBRARIES}  
        ${ROOT_LIBRARIES}
        DataProducts::data_products
        Unpackers::unpackers
)

#These are the reco objects for which we make root dictionary
set(Reco 
    ${PROJECT_SOURCE_DIR}/include/reco/common/*.hh
    ${PROJECT_SOURCE_DIR}/include/reco/wfd5/*.hh
)

# Make the ROOT dictionary
root_generate_dictionary(RecoDictionary
    ${Reco} 
    MODULE reco
    LINKDEF ${PROJECT_SOURCE_DIR}/include/reco/LinkDef.h
)

# Install the ROOT dictionary
install(FILES
            ${PROJECT_BINARY_DIR}/src/libreco_rdict.pcm
            ${PROJECT_BINARY_DIR}/src/libreco.rootmap
        DESTINATION lib
)

# Install the library
install(TARGETS reco
        EXPORT RecoTargets
        LIBRARY DESTINATION lib
)

# Install the headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include
)

# Export
install(EXPORT RecoTargets
  FILE RecoTargets.cmake
  NAMESPACE Reco::
  DESTINATION lib/cmake/Reco
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/RecoConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/RecoConfig.cmake"
  INSTALL_DESTINATION lib/cmake/Reco
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/RecoConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/RecoConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/RecoConfigVersion.cmake"
  DESTINATION lib/cmake/Reco
)