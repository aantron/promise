.PHONY : all
all : c ppx ml simple-demo

.PHONY : simple-demo
simple-demo :
	@echo "\n\nNode.js\n\n"
	node ./lib/js/test/test.js
	@echo "\n\nNative\n\n"
	./lib/bs/native/test.native

.PHONY : ml
ml : bsconfig.json
	npm run build
	npm run build-native

.PHONY : c
c :
	mkdir -p _build/
	cc -c src/native/accessors.c -o _build/libuv_accessors.o

# Build the PPX using ocamlfind, and make a janky package in node_modules
# containing it.
.PHONY : ppx
ppx :
	mkdir -p _build/
	`find . -name refmt3.exe` --print ml src/ppx/ppx_await.re > _build/ppx.ml
	ocamlfind opt \
		-linkpkg -package compiler-libs.common _build/ppx.ml -o _build/ppx_await
	mkdir -p node_modules/ppx_await/
	cp _build/ppx_await node_modules/ppx_await/
	cp src/ppx/package.json node_modules/ppx_await/
	touch node_modules/ppx_await/bsconfig.json

.PHONY : deps
deps :
	which opam
	which node
	which npm
	opam switch 4.02.3+buckle-master
	(opam pin list --short | grep reason) || opam pin add --dev-repo reason
	opam depext ctypes-foreign conf-libuv
	opam install ctypes ctypes-foreign ocamlfind
	npm install

# Use the latest refmt, installed by make deps from opam.
bsconfig.json : bsconfig.in.json
	sed s%PREFIX%`opam config var prefix`%g < bsconfig.in.json > bsconfig.json

.PHONY : clean
clean :
	npm run clean
	rm -rf _build/
