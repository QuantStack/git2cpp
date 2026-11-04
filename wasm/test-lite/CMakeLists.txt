cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm-test-lite)

include("../common.cmake")

set(SERVE_DIR "dist")

add_custom_target(build-test-lite
    DEPENDS build-recipe cockle

    # Installs cockle from ../cockle so that the correct prepare_wasm script is used.
    COMMAND npm install

    COMMAND jupyter lite --version
    COMMAND COCKLE_WASM_EXTRA_CHANNEL=${BUILT_PACKAGE_DIR} jupyter lite build --output-dir ${SERVE_DIR}
    BYPRODUCTS cockle-config.json .cockle_temp/ .jupyterlite.doit.db cockle_wasm_env/ ${SERVE_DIR}
)

add_custom_target(rebuild-test-lite
    DEPENDS rebuild-recipe clean-env-test-lite build-test-lite
)

add_custom_target(test-lite
    # A pytest fixture ensures that `npm run serve` runs for the duration of the tests.
    COMMAND GIT2CPP_TEST_WASM=lite pytest -v -rP
    WORKING_DIRECTORY ../../test
)

add_custom_target(clean-env-test-lite
    COMMAND ${CMAKE_COMMAND} -E remove_directory cockle_wasm_env
)
