TESTFILES := $(wildcard **/*.txt) $(wildcard *.txt)
TESTS := $(TESTFILES:.txt=.test)

.PHONY: all

all: $(TESTS)

%.test:
	@rm -f $*.err $*.out
	@touch $*.err $*.out
	@echo "TEST $*"
	@../buddy -i $*.txt > $*.out ;\
	PROG_EXIT_CODE=$$?;\
	echo "Comparing buddy output for $*.txt...";\
	diff -B --ignore-all-space $*.out $*.txt.expected;\
	TAC_DIFF_EXIT=$$?;\
	exit $$TAC_DIFF_EXIT

clean:
	rm -f *.3ac *.out *.err
