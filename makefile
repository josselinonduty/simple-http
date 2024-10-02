# ------------ Directory structure ------------
EXEC=simple-http
BINDIR=bin
CC=gcc
DOCSDIR=docs
INCLUDEDIR=include
SRCDIR=src
TESTDIR=tests
# ------------ Documentation configuration ------------
DOCS=doxygen
DOCSCONFIG=Doxyfile
# ------------ Debug configuration ------------
DEBUG=valgrind
DFLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes
# ------------ Build configuration ------------
SRC=$(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/**/*.c)
OBJ=$(SRC:%.c=%.o)
CFLAGS=-Wall -pedantic -std=c99 -I$(INCLUDEDIR)
LDFLAGS=-lm -lcunit
# ------------ Test configuration ------------
TEST=$(BINDIR)/$(TESTDIR)/run
CFLAGSTEST=-Wall -pedantic -std=c99 -I$(INCLUDEDIR) -I$(TESTDIR)/$(INCLUDEDIR)
LDFLAGSTEST=$(LDFLAGS)
SRCTEST=$(wildcard $(TESTDIR)/$(SRCDIR)/*.c) $(wildcard $(TESTDIR)/$(SRCDIR)/**/*.c)
OBJTEST=$(SRCTEST:%.c=%.o)
# ------------ Lint configuration ------------
LINT=indent
LINTFLAGS=-nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4  -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1
# ---------------------------------
#               Targets            
# ---------------------------------

all: build
.PHONY: all

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) -o $@ -c $< $(CFLAGS)

$(SRCDIR)/%/%.o: $(SRCDIR)/%/%.c
	@$(CC) -o $@ -c $< $(CFLAGS)

docs: $(DOCSDIR)
	mkdir -p $(DOCSDIR)
	$(DOCS) $(DOCSCONFIG)
.PHONY: docs

lint: $(SRC)
	@for file in $^; do \
		$(LINT) $(LINTFLAGS) $$file; \
	done
	@rm -f ./$(SRCDIR)/*.c~ ./$(SRCDIR)/**/*.c~
.PHONY: lint

debug: $(BINDIR)/$(EXEC)
	@$(DEBUG) -s $(DFLAGS) $(BINDIR)/$(EXEC) 2>&1 | tee $(BINDIR)/.valgrind.log
.PHONY: debug

debug/headless: $(EXEC)
	@$(DEBUG) -s $(DFLAGS) $(TEST) 2>&1 | tee $(BINDIR)/$(TESTDIR)/.valgrind.log | \
		grep -q "All heap blocks were freed -- no leaks are possible"; \
		if [ $$? -ne 0 ]; then \
			echo "Memory leaks detected!"; \
			exit 1; \
		fi;
.PHONY: debug/headless

build: $(OBJ)
	@mkdir -p $(BINDIR)
	@gcc -o $(BINDIR)/${EXEC} $^ $(CFLAGS) $(LDFLAGS)
.PHONY: build

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) -fPIC -o $@ -c $< $(CFLAGS)

$(SRCDIR)/%/%.o: $(SRCDIR)/%/%.c
	@$(CC) -fPIC -o $@ -c $< $(CFLAGS)

tests: $(TEST)
	@./$(TEST)
.PHONY: tests

$(TEST): $(OBJTEST)
	@mkdir -p $(BINDIR)/$(TESTDIR)
	$(CC) -o $(TEST) $(OBJTEST) $(LDFLAGSTEST)

$(TESTDIR)/%.o: $(TESTDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGSTEST)

$(TESTDIR)/%/%.o: $(TESTDIR)/%/%.c
	$(CC) -o $@ -c $< $(CFLAGSTEST)

clean: clean/build clean/objects clean/exec clean/docs clean/lint clean/debug
.PHONY: clean

clean/build:
	@rm -f ./$(BINDIR)/$(EXEC)

clean/objects:
	@rm -f ./$(SRCDIR)/*.o ./$(SRCDIR)/**/*.o
	@rm -f ./$(TESTDIR)/$(SRCDIR)/*.o ./$(TESTDIR)/$(SRCDIR)/**/*.o
.PHONY: clean/objects

clean/exec:
	@rm -f ./$(TEST)
.PHONY: clean/exec

clean/docs:
	@rm -rf ./$(DOCSDIR)
.PHONY: clean/docs

clean/lint:
	@rm -f ./$(SRCDIR)/*.c~ ./$(SRCDIR)/**/*.c~
	@rm -f ./$(TESTDIR)/$(SRCDIR)/*.c~ ./$(TESTDIR)/$(SRCDIR)/**/*.c~
.PHONY: clean/lint

clean/debug:
	@rm -f ./$(BINDIR)/$(TESTDIR)/.valgrind.log
.PHONY: clean/debug
