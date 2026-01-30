cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm-recipe)

include("../common.cmake")

set(EM_FORGE_RECIPES_REPO "https://github.com/emscripten-forge/recipes")
set(GIT2CPP_RECIPE_DIR "recipes/recipes_emscripten/git2cpp")
set(PREFIX_CHANNEL "https://repo.prefix.dev/emscripten-forge-4x")
set(RATTLER_ARGS "--package-format tar-bz2 --target-platform emscripten-wasm32 -c ${PREFIX_CHANNEL} -c microsoft -c conda-forge")

# Only want the git2cpp recipe from emscripten-forge/recipes repo, not the whole repo.
# Note removing the .git directory otherwise `git clean -fxd` will not remove the directory.
add_custom_command(
    OUTPUT ${EM_FORGE_RECIPES_DIR}
    COMMAND git clone --no-checkout ${EM_FORGE_RECIPES_REPO} --depth 1 ${EM_FORGE_RECIPES_DIR}
	COMMAND cd ${EM_FORGE_RECIPES_DIR} && git sparse-checkout init
	COMMAND cd ${EM_FORGE_RECIPES_DIR} && git sparse-checkout set ${GIT2CPP_RECIPE_DIR}
	COMMAND cd ${EM_FORGE_RECIPES_DIR} && git checkout main
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${EM_FORGE_RECIPES_DIR}/.git
)

set(MODIFY_RECIPE_ARGS "")
if (NOT USE_RECIPE_PATCHES)
    set(MODIFY_RECIPE_ARGS "--no-patches")
endif()

add_custom_target(modify-recipe
    DEPENDS ${EM_FORGE_RECIPES_DIR}
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/modify-recipe.py ${GIT2CPP_RECIPE_DIR} ${MODIFY_RECIPE_ARGS}
    WORKING_DIRECTORY ${EM_FORGE_RECIPES_DIR}
)

add_custom_command(
    OUTPUT ${EM_FORGE_RECIPES_DIR}/${BUILT_PACKAGE_SUBDIR}
    DEPENDS modify-recipe
    COMMAND bash -c "rattler-build build ${RATTLER_ARGS} --recipe ${GIT2CPP_RECIPE_DIR}"
    WORKING_DIRECTORY ${EM_FORGE_RECIPES_DIR}
)

add_custom_target(build-recipe
    DEPENDS ${EM_FORGE_RECIPES_DIR}/${BUILT_PACKAGE_SUBDIR}
    WORKING_DIRECTORY ${EM_FORGE_RECIPES_DIR}
)

add_custom_target(rebuild-recipe DEPENDS clean-output-dir build-recipe)

add_custom_target(clean-output-dir
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${EM_FORGE_RECIPES_DIR}/${BUILT_PACKAGE_SUBDIR}
)
