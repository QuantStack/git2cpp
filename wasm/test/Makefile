default: build
.PHONY: build clean clean-env rebuild test

include ../common.mk

build:
	npm install
	COCKLE_WASM_EXTRA_CHANNEL=$(BUILT_PACKAGE_DIR) npm run build

rebuild: clean-env build

test:
	# A pytest fixture ensures that `npm run serve` runs for the duration of the tests.
	cd ../../test && GIT2CPP_TEST_WASM=1 pytest -v -rP test_git.py

clean-env:
	rm -rf cockle_wasm_env/

clean: clean-env
	rm -rf lib/ node_modules/
