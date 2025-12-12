default: build
.PHONY: build clean rebuild serve test

build:
	make -C build $@
	make -C cockle-deploy $@
	make -C lite-deploy $@
	make -C test $@

# Rebuild after change in C++ source code.
rebuild:
	make -C build $@
	make -C cockle-deploy $@
	make -C lite-deploy $@
	make -C test $@

# Serve both cockle and JupyterLite deployments.
serve:
	npx static-handler --cors --coop --coep --corp serve

test:
	make -C test $@

clean:
	make -C build $@
	make -C cockle-deploy $@
	make -C lite-deploy $@
	make -C test $@
