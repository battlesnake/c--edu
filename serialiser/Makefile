CXXFLAGS := -std=c++17 -g -Wall -Wextra -Werror

.PHONY: all
all: serial

.PHONY: run
run: all
	./serial

.PHONY: clean
clean:
	rm -rf -- serial *.oxx *.d

serial: serial.oxx
	$(CXX) $(LDFLAGS) -o $@ $^

%.oxx: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c -o $@ $<

-include $(wildcard *.d)
