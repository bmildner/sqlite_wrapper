find_package(Doxygen REQUIRED dot)

include(FetchContent)

# get doxygen-awesome-css from github
cmake_policy(SET CMP0135 NEW)  # set file timestamps to extraction time
FetchContent_Declare(doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip)
FetchContent_MakeAvailable(doxygen-awesome-css)
# Save the location the files were cloned into
# This allows us to get the path to doxygen-awesome.css
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

# Doxygen Config for doxygen-awesome-css
set(DOXYGEN_HTML_EXTRA_STYLESHEET  ${AWESOME_CSS_DIR}/doxygen-awesome.css)
set(DOXYGEN_HTML_COLORSTYLE        LIGHT)
set(DOXYGEN_GENERATE_TREEVIEW      YES)
set(DOXYGEN_DISABLE_INDEX          NO)
set(DOXYGEN_FULL_SIDEBAR           NO)

# Doxygen Config
set(DOXYGEN_PROJECT_NAME           "SQLite Wrapper")
set(DOXYGEN_HTML_OUTPUT            ${PROJECT_BINARY_DIR}/docs/html)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(DOXYGEN_INCLUDE_PATH           ${CMAKE_CURRENT_SOURCE_DIR}/../include)
set(DOXYGEN_PREDEFINED             DOXYGEN=1)
set(DOXYGEN_MACRO_EXPANSION        YES)
set(DOXYGEN_EXTRACT_ALL            YES)
set(DOXYGEN_RECURSIVE              YES)
set(DOXYGEN_JAVADOC_AUTOBRIEF      YES)
set(DOXYGEN_GENERATE_HTML          YES)
set(DOXYGEN_HAVE_DOT               YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT    YES)
set(DOXYGEN_UML_LOOK               YES)
set(DOXYGEN_SHOW_GROUPED_MEMB_INC  YES)
set(DOXYGEN_TAB_SIZE               2)
set(DOXYGEN_WARN_NO_PARAMDOC       YES)
set(DOXYGEN_WARN_AS_ERROR          YES)
set(DOXYGEN_QUIET                  YES)

doxygen_add_docs(build_documentation
    ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/docs
    ALL
    COMMENT "Generate HTML documentation"
)
