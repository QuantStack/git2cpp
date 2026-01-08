cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm-test)

include("../common.cmake")

add_custom_target(build-test
    DEPENDS build-recipe
    COMMAND npm install
    COMMAND COCKLE_WASM_EXTRA_CHANNEL=${BUILT_PACKAGE_DIR} npm run build
    BYPRODUCTS cockle_wasm_env lib node_modules
)

add_custom_target(rebuild-test
    DEPENDS rebuild-recipe clean-env-test build-test
)

add_custom_target(test
	# A pytest fixture ensures that `npm run serve` runs for the duration of the tests.
    COMMAND GIT2CPP_TEST_WASM=1 pytest -v -rP test_git.py
    WORKING_DIRECTORY ../../test
)

add_custom_target(clean-env-test
    COMMAND ${CMAKE_COMMAND} -E remove_directory cockle_wasm_env
)
