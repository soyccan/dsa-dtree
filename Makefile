CXX := clang++
CXXFLAGS := -Wall -Wextra -Wconversion -std=gnu++17 -I/usr/local/include
LDFLAGS :=

EXES := tree predictor
OBJS := main.o dtree.o tree_predictor.o tree_predict_func.o
tmpfiles := tree_predict_func.cpp
deps := $(OBJS:%.o=.%.o.d)
compdb := $(OBJS:%.o=.%.o.json) # compilation database


ifndef DEBUG
	DEBUG := 1
endif
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -DNDEBUG -O2
endif


.PHONY: all clean run upload
all: run
	sed -e '1s/^/[/' -e '$$s/,$$/]/' $(compdb) > compile_commands.json

run: predictor
	./predictor

predictor: tree_predictor.o tree_predict_func.o
	$(CXX) $(LDFLAGS) -o $@ $^

tree: main.o dtree.o
	$(CXX) $(LDFLAGS) -o $@ $^

tree_predict_func.cpp: tree
	./tree wine.train 0 > tree_predict_func.cpp

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -MMD -MF .$@.d -MJ .$@.json $<

upload:
	scp -P 9453 -r *.hpp *.h *.cpp Makefile soyccan@bravo.nctu.me:/home/soyccan/Documents/dsa-hw4/

clean:
	rm -rf $(OBJS) $(deps) $(compdb) $(EXES) $(tmpfiles)

-include $(deps)
