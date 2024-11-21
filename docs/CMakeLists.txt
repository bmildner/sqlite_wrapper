find_package(Doxygen REQUIRED dot)

set(DOXYGEN_HTML_OUTPUT            ${PROJECT_BINARY_DIR}/docs/html)
set(DOXYGEN_JAVADOC_AUTOBRIEF      YES)
set(DOXYGEN_GENERATE_HTML          YES)
set(DOXYGEN_HAVE_DOT               YES)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

doxygen_add_docs(build_documentation
    ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include
    ALL  # does not seem to work
    COMMENT "Generate HTML documentation"
)
