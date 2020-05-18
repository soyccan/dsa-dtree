CXX := clang++
CXXFLAGS := -Wall -Wextra -std=gnu++17 -I/usr/local/include
LDFLAGS :=

EXES := tree predictor test/rand
OBJS := main.o dtree.o randmain.o randdtree.o maxmain.o maxdtree.o
deps := $(OBJS:%.o=.%.o.d)
compdb-dep := $(OBJS:%.o=.%.o.json) # compilation database
compdb := compile_commands.json

tree_pred_func := tree_pred_func.cpp

DTRAIN := data/wine.train
DTEST := data/wine.test
EPSILON := 0


ifndef DEBUG
	DEBUG := 1
endif
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -DNDEBUG -O2
endif


.PHONY: all clean run rand-run upload scan-build
all: tree randtree maxtree $(compdb)


$(compdb): $(compdb-dep)
	sed -e '1s/^/[/' -e '$$s/,$$/]/' $^ > $@

maxtree: maxmain.o maxdtree.o dtree.o
	$(CXX) $(LDFLAGS) -o $@ $^

randtree: randmain.o randdtree.o dtree.o
	$(CXX) $(LDFLAGS) -o $@ $^

tree: main.o dtree.o
	$(CXX) $(LDFLAGS) -o $@ $^

test/rand: test/rand.c
	clang -O2 -o $@ $^

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -MMD -MF .$@.d -MJ .$@.json $<


# phony targets
#
run: tree
	./tree $(DTRAIN) $(EPSILON) | clang-format > $(tree_pred_func)
	$(CXX) $(CXXFLAGS) -o predictor tree_predictor.cpp $(tree_pred_func)
	./predictor $(DTEST)

randtest: tree test/rand
	test/rand > test/1.in
	./tree test/1.in 0 | clang-format > $(tree_pred_func)
	$(CXX) $(CXXFLAGS) -o predictor tree_predictor.cpp $(tree_pred_func)
	./predictor test/1.in

randtree-run: randtree
	./randtree $(DTRAIN) $(EPSILON) | clang-format > $(tree_pred_func)
	$(CXX) $(CXXFLAGS) -o predictor tree_predictor.cpp $(tree_pred_func)
	./predictor $(DTEST)

scan-build:
	PATH=/usr/local/opt/llvm/bin:$(PATH) scan-build make

upload:
	scp -P 9453 -r *.hpp *.h *.cpp Makefile soyccan@bravo.nctu.me:/home/soyccan/Documents/dsa-hw4/

clean:
	rm -rf $(EXES) $(OBJS) $(deps) $(compdb) $(compdb-dep) $(tree_pred_func)

-include $(deps)
