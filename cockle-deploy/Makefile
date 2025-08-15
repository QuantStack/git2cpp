default: all
.PHONY: all build-deployment build-recipe clean clean-deployment clean-package modify-recipe rebuild serve

EM_FORGE_RECIPES_DIR = em-forge-recipes
GIT2CPP_RECIPE_DIR = recipes/recipes_emscripten/git2cpp
BUILT_PACKAGE_DIR = $(EM_FORGE_RECIPES_DIR)/output

# Note removing the .git directory otherwise `git clean -fxd` will not remove the directory.
$(EM_FORGE_RECIPES_DIR):
	git clone https://github.com/emscripten-forge/recipes --depth 1 $@
	rm -rf $@/.git

modify-recipe: $(EM_FORGE_RECIPES_DIR)
	python modify-recipe.py $(EM_FORGE_RECIPES_DIR)/$(GIT2CPP_RECIPE_DIR)

build-recipe: modify-recipe
	cd $(EM_FORGE_RECIPES_DIR) && pixi run build-emscripten-wasm32-pkg $(GIT2CPP_RECIPE_DIR)

build-deployment: build-recipe
	npm install
	COCKLE_WASM_EXTRA_CHANNEL=./$(BUILT_PACKAGE_DIR) npm run build

all: build-deployment

# Rebuild package and deployment after changing git2cpp source code.
rebuild: clean-package all

# Run `make` before this.
serve:
	npm run serve

clean: clean-deployment
	rm -rf $(EM_FORGE_RECIPES_DIR) node_modules/

clean-package:
	rm -rf $(BUILT_PACKAGE_DIR) cockle_wasm_env/

# Clean the deployment without removing the built package.
clean-deployment:
	rm -rf cockle_wasm_env/ dist/ assets/cockle-config.json
