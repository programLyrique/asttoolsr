BEAR := $(shell command -v bear 2> /dev/null)

ifdef BEAR
	BEAR := $(BEAR) --
endif

.PHONY: all build check document test install

all: document build check

build: document
	R CMD build .

check: build
	R CMD check asttoolsr*tar.gz

clean:
	-rm -f asttoolsr*tar.gz
	-rm -fr asttoolsr.Rcheck
	-rm -rf src/*.o src/*.so

document:
	R -e 'devtools::document()'

test:
	R -e 'devtools::test()'

clean_test:
	-rm -rf tests/testthat/_snaps

trace:
	strace Rscript -e 'devtools::test()' 2> trace

lintr:
	R --slave -e "lintr::lint_package()"

install: clean
	$(BEAR) R CMD INSTALL .

