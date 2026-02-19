cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm-lite-deploy)

include("../common.cmake")

set(SERVE_DIR "../serve/dist/lite")

add_custom_target(build-lite
    DEPENDS build-recipe cockle

    # Ensure cockle is installed in this directory so that we used the correct prepare_wasm script.
    COMMAND npm install ../cockle
    COMMAND npm install

    COMMAND jupyter lite --version
    COMMAND COCKLE_WASM_EXTRA_CHANNEL=${BUILT_PACKAGE_DIR} jupyter lite build --output-dir ${SERVE_DIR}
    BYPRODUCTS cockle-config.json .cockle_temp/ .jupyterlite.doit.db cockle_wasm_env/ ${SERVE_DIR}
)

add_custom_target(rebuild-lite
    DEPENDS rebuild-recipe clean-env-lite build-lite
)

add_custom_target(clean-env-lite
    COMMAND ${CMAKE_COMMAND} -E remove_directory cockle_wasm_env
)
