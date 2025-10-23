default: build
.PHONY: build clean clean-output-dir modify-recipe rebuild

include ../common.mk

EM_FORGE_RECIPES_REPO = https://github.com/emscripten-forge/recipes
GIT2CPP_RECIPE_DIR = recipes/recipes_emscripten/git2cpp
RATTLER_ARGS = --package-format tar-bz2 --target-platform emscripten-wasm32 -c https://repo.prefix.dev/emscripten-forge-dev -c microsoft -c conda-forge

# Only want the git2cpp recipe from emscripten-forge/recipes repo, not the whole repo.
# Note removing the .git directory otherwise `git clean -fxd` will not remove the directory.
$(EM_FORGE_RECIPES_DIR):
	git clone --no-checkout ${EM_FORGE_RECIPES_REPO} --depth 1 $@
	cd $@ && git sparse-checkout init
	cd $@ && git sparse-checkout set $(GIT2CPP_RECIPE_DIR)
	cd $@ && git checkout main
	rm -rf $@/.git

modify-recipe: $(EM_FORGE_RECIPES_DIR)
	python modify-recipe.py $(EM_FORGE_RECIPES_DIR)/$(GIT2CPP_RECIPE_DIR)

build: modify-recipe
	cd $(EM_FORGE_RECIPES_DIR) && rattler-build build $(RATTLER_ARGS) --recipe $(GIT2CPP_RECIPE_DIR)

rebuild: clean-output-dir build

clean:
	rm -rf $(EM_FORGE_RECIPES_DIR)

clean-output-dir:
	rm -rf $(BUILT_PACKAGE_DIR)
