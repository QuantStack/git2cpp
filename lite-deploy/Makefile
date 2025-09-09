default: all
.PHONY: all build-deployment build-recipe clean clean-deployment clean-package modify-recipe rebuild serve

EM_FORGE_RECIPES_DIR = em-forge-recipes
GIT2CPP_RECIPE_DIR = recipes/recipes_emscripten/git2cpp
BUILT_PACKAGE_DIR = $(EM_FORGE_RECIPES_DIR)/output

# Note removing the .git directory otherwise `git clean -fxd` will not remove the directory.
$(EM_FORGE_RECIPES_DIR):
	git clone https://github.com/emscripten-forge/recipes --depth 1 $@
	rm -rf $@/.git

.pixi: $(EM_FORGE_RECIPES_DIR)
	cd $(EM_FORGE_RECIPES_DIR) && pixi run setup

modify-recipe: .pixi
	python modify-recipe.py $(EM_FORGE_RECIPES_DIR)/$(GIT2CPP_RECIPE_DIR)

build-recipe: modify-recipe
	cd $(EM_FORGE_RECIPES_DIR) && pixi run build-emscripten-wasm32-pkg $(GIT2CPP_RECIPE_DIR)

build-deployment: build-recipe
	jupyter lite --version
	COCKLE_WASM_EXTRA_CHANNEL=./$(BUILT_PACKAGE_DIR) jupyter lite build --output-dir dist

all: build-deployment

# Rebuild package and deployment after changing git2cpp source code.
rebuild: clean-package all

# Run `make` before this.
serve:
	npx static-handler dist

clean: clean-deployment
	rm -rf $(EM_FORGE_RECIPES_DIR)

clean-package:
	rm -rf $(BUILT_PACKAGE_DIR) cockle_wasm_env/

# Clean the deployment without removing the built package.
clean-deployment:
	rm -rf .cockle_temp/ .jupyterlite.doit.db cockle-config.json cockle_wasm_env/ dist/
