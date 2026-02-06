cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_EXTENSIONS OFF)


project(git2cpp CXX)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
message(STATUS "🔧 C++ standard: ${CMAKE_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(GIT2CPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Versionning
# ===========
file(STRINGS "${GIT2CPP_SOURCE_DIR}/version.hpp" git2cpp_version_defines
    REGEX "#define GIT2CPP_VERSION_(MAJOR|MINOR|PATCH)")

foreach(ver ${git2cpp_version_defines})
    if(ver MATCHES "#define GIT2CPP_VERSION_(MAJOR|MINOR|PATCH) ([0-9]+);$")
        set(PROJECT_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()

set(CMAKE_PROJECT_VERSION
    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

message(STATUS "Building git2cpp v${CMAKE_PROJECT_VERSION}")

# Dependencies
# ============

find_package(libgit2)
find_package(termcolor)
# CLI11 is a single header, not packaged for cmake

# Build
# =====

set(GIT2CPP_SRC
    ${GIT2CPP_SOURCE_DIR}/subcommand/add_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/add_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/branch_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/branch_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/checkout_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/checkout_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/clone_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/clone_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/diff_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/diff_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/commit_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/commit_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/config_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/config_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/fetch_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/fetch_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/init_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/init_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/log_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/log_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/merge_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/merge_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/mv_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/mv_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/push_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/push_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/rebase_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/rebase_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/remote_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/remote_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/reset_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/reset_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/revlist_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/revlist_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/revparse_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/revparse_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/rm_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/rm_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/stash_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/stash_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/status_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/status_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/tag_subcommand.cpp
    ${GIT2CPP_SOURCE_DIR}/subcommand/tag_subcommand.hpp
    ${GIT2CPP_SOURCE_DIR}/utils/ansi_code.cpp
    ${GIT2CPP_SOURCE_DIR}/utils/ansi_code.hpp
    ${GIT2CPP_SOURCE_DIR}/utils/common.cpp
    ${GIT2CPP_SOURCE_DIR}/utils/common.hpp
    ${GIT2CPP_SOURCE_DIR}/utils/git_exception.cpp
    ${GIT2CPP_SOURCE_DIR}/utils/git_exception.hpp
    ${GIT2CPP_SOURCE_DIR}/utils/output.cpp
    ${GIT2CPP_SOURCE_DIR}/utils/output.hpp
    ${GIT2CPP_SOURCE_DIR}/utils/progress.cpp
    ${GIT2CPP_SOURCE_DIR}/utils/progress.hpp
    ${GIT2CPP_SOURCE_DIR}/utils/terminal_pager.cpp
    ${GIT2CPP_SOURCE_DIR}/utils/terminal_pager.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/annotated_commit_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/annotated_commit_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/branch_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/branch_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/commit_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/commit_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/config_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/config_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/diff_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/diff_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/diffstats_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/diffstats_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/index_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/index_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/object_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/object_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/patch_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/patch_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/rebase_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/rebase_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/refs_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/refs_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/remote_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/remote_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/repository_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/repository_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/revwalk_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/revwalk_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/signature_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/signature_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/status_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/status_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/tag_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/tag_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/tree_wrapper.cpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/tree_wrapper.hpp
    ${GIT2CPP_SOURCE_DIR}/wrapper/wrapper_base.hpp
    ${GIT2CPP_SOURCE_DIR}/main.cpp
    ${GIT2CPP_SOURCE_DIR}/version.hpp
)

add_executable(git2cpp ${GIT2CPP_SRC})
target_link_libraries(git2cpp PRIVATE libgit2::libgit2package termcolor::termcolor)
