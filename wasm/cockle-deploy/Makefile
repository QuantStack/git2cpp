default: build
.PHONY: build clean clean-env rebuild

include ../common.mk

build:
	npm install
	COCKLE_WASM_EXTRA_CHANNEL=$(BUILT_PACKAGE_DIR) npm run build

rebuild: clean-env build

clean: clean-env
	rm -rf ../serve/cockle node_modules/

clean-env:
	rm -rf cockle_wasm_env/
