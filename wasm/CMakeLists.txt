cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm)

option(USE_RECIPE_PATCHES "Use patches from emscripten-forge recipe or not" ON)
option(USE_COCKLE_RELEASE "Use latest cockle release rather than repo main branch" OFF)

add_subdirectory(recipe)
add_subdirectory(cockle-deploy)
add_subdirectory(lite-deploy)
add_subdirectory(serve)
add_subdirectory(test)

# Build everything (package, cockle and lite deployments, tests).
add_custom_target(build ALL DEPENDS build-recipe build-cockle build-lite build-serve build-test)

# Rebuild after change in C++ code.
add_custom_target(rebuild DEPENDS rebuild-recipe rebuild-cockle rebuild-lite rebuild-test)

if (USE_COCKLE_RELEASE)
    execute_process(COMMAND npm view @jupyterlite/cockle version OUTPUT_VARIABLE COCKLE_BRANCH)
    set(COCKLE_BRANCH "v${COCKLE_BRANCH}")
else()
    set(COCKLE_BRANCH "main")
endif()

add_custom_target(cockle
    COMMENT "Using cockle from github repository ${COCKLE_BRANCH} branch"
    # Don't re-clone if directory already exists - could do better here.
    COMMAND test -d cockle || git clone https://github.com/jupyterlite/cockle --depth 1 --branch ${COCKLE_BRANCH}
    COMMAND cd cockle && npm install && npm run build
)
