CXX := clang++
CXXFLAGS := -Wall -Wextra -std=gnu++17 -I/usr/local/include
LDFLAGS :=

EXES := tree predictor
OBJS := main.o dtree.o tree_predictor.o tree_pred_func.o
tmpfiles := tree_pred_func.cpp
deps := $(OBJS:%.o=.%.o.d)
compdb-dep := $(OBJS:%.o=.%.o.json) # compilation database
compdb := compile_commands.json


ifndef DEBUG
	DEBUG := 1
endif
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -DNDEBUG -O2
endif


.PHONY: all clean run upload tree_pred_func.cpp
all: run $(compdb)

$(compdb): $(compdb-dep)
	sed -e '1s/^/[/' -e '$$s/,$$/]/' $^ > $@

run: predictor
	./predictor data/wine.test

predictor: tree_predictor.o tree_pred_func.o
	$(CXX) $(LDFLAGS) -o $@ $^

tree: main.o dtree.o
	$(CXX) $(LDFLAGS) -o $@ $^

tree_pred_func.cpp: tree
	./tree data/wine.train 0 | clang-format > $@

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -MMD -MF .$@.d -MJ .$@.json $<

scan-build:
	PATH=/usr/local/opt/llvm/bin:$(PATH) scan-build make

upload:
	scp -P 9453 -r *.hpp *.h *.cpp Makefile soyccan@bravo.nctu.me:/home/soyccan/Documents/dsa-hw4/

clean:
	rm -rf $(EXES) $(OBJS) $(deps) $(compdb) $(compdb-dep) $(tmpfiles)

-include $(deps)
