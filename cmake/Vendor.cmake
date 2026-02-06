# Catch 2
CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/catch2
    VERSION 3.12.0
    OPTIONS
        "CATCH_BUILD_TESTING OFF"
        "CATCH_INSTALL_DOCS OFF"
)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
