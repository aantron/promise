.PHONY : default
default :
	@echo Please choose a specific target
	@exit 1

.PHONY : coverage
coverage : clean-coverage native-coverage bucklescript-coverage
	bisect-ppx-report html --expect src/

.PHONY : native-coverage
native-coverage :
	dune clean
	BISECT_ENABLE=yes dune exec test/test_main.exe

.PHONY : bucklescript-coverage
bucklescript-coverage :
	npm run clean
	BISECT_ENABLE=yes npm run test

.PHONY : clean-coverage
clean-coverage :
	rm -rf *.coverage _coverage

WEBPACK := npx webpack --display none --mode production --optimize-minimize

.PHONY : bundle-size-test
bundle-size-test :
	@npm run build
	@$(WEBPACK) \
	  --entry ./test/bundle/main_control.js \
	  --output ./test/bundle/bundle_control.js
	@$(WEBPACK) \
	  --entry ./test/bundle/main_repromise.js \
	  --output ./test/bundle/bundle_repromise.js
	@ls -l lib/js/src/js/promise.js
	@ls -l test/bundle/bundle*.js
