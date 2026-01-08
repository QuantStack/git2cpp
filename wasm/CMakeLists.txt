cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm)

add_subdirectory(recipe)
add_subdirectory(cockle-deploy)
add_subdirectory(lite-deploy)
add_subdirectory(test)

# Build everything (package, cockle and lite deployments, tests).
add_custom_target(build ALL DEPENDS build-recipe build-cockle build-lite build-test)

# Rebuild after change in C++ code.
add_custom_target(rebuild DEPENDS rebuild-recipe rebuild-cockle rebuild-lite rebuild-test)

# Serve both cockle and JupyterLite deployments.
add_custom_target(serve COMMAND npx static-handler --cors --coop --coep --corp serve)
