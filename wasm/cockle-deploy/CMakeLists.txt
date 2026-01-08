cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm-cockle-deploy)

include("../common.cmake")

set(SERVE_DIR "../serve/cockle")

add_custom_target(build-cockle
    DEPENDS build-recipe
    COMMAND npm install
    COMMAND COCKLE_WASM_EXTRA_CHANNEL=${BUILT_PACKAGE_DIR} npm run build
    BYPRODUCTS cockle_wasm_env node_modules ${SERVE_DIR}
)

add_custom_target(rebuild-cockle
    DEPENDS rebuild-recipe clean-env-cockle build-cockle
)

add_custom_target(clean-env-cockle
    COMMAND ${CMAKE_COMMAND} -E remove_directory cockle_wasm_env
)
