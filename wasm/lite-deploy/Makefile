default: build
.PHONY: build clean rebuild

include ../common.mk

OUTPUT_DIR=../serve/lite

build:
	jupyter lite --version
	COCKLE_WASM_EXTRA_CHANNEL=$(BUILT_PACKAGE_DIR) jupyter lite build --output-dir $(OUTPUT_DIR)

rebuild: clean build

clean:
	rm -rf .cockle_temp/ .jupyterlite.doit.db cockle_wasm_env/ $(OUTPUT_DIR)
