# Emscripten-forge recipes directory relative to 'recipe' directory
set(EM_FORGE_RECIPES_DIR "em-forge-recipes")

# Output directory for built emscripten-forge packages relative to EM_FORGE_RECIPES_DIR
set(BUILT_PACKAGE_SUBDIR "output")

# Output directory for built emscripten-forge packages relative to subdirectories of this directory
set(BUILT_PACKAGE_DIR "../recipe/${EM_FORGE_RECIPES_DIR}/${BUILT_PACKAGE_SUBDIR}")
