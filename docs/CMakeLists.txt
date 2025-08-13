# Documentation build configuration

# Find Doxygen
find_package(Doxygen REQUIRED)

if(DOXYGEN_FOUND)
    # Set Doxygen configuration
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    # Check if Doxyfile.in exists, if not create a basic one
    if(NOT EXISTS ${DOXYGEN_IN})
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    endif()
    
    # Configure the file if template exists
    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    endif()
    
    # Add target for generating documentation
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    # Install documentation
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION share/doc/StockPredict
        OPTIONAL
    )
else()
    message(STATUS "Doxygen not found - documentation will not be built")
endif()
