CPP=g++
LD=g++
CPPFLAGS=-Wall -Wextra -I. -c -g -ggdb
LDFLAGS=-lm -g -ggdb -fsanitize=address

all: bin/tictactoe

clean:
	rm -rf bin

bin/%: bin/%.cpp.o
	$(LD) $(LDFLAGS) -o $@ $^

bin/%.cpp.o: examples/%.cpp mcts.hpp
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -o $@ $<
