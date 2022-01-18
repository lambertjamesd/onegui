
CC = gcc
LN = gcc
CFLAGS = -Wall -Werror -O0 -g
LDFLAGS = -lgcc

SRC_FILES = $(shell find src/ -type f -name '*.c')
TEST_FILES = $(shell find test/ -type f -name '*.c')

SRC_TEST_OBJECTS = $(patsubst %.c, build/test/%.o, $(SRC_FILES)) $(patsubst %.c, build/test/%.o, $(TEST_FILES))

DEP_FILES = $(shell find build/ -type f -name '*.d')

-include $(DEP_FILES)

build/test/%.o build/test/%.d: %.c
	@mkdir -p $(@D)
	# $(CC) -MM $^ -MF "$(@:.o=.d)" -MT"$@"
	$(CC) $(CFLAGS) -c -o $@ $<

build/test_runner: $(SRC_TEST_OBJECTS)
	$(LN) -g -o build/test_runner $(SRC_TEST_OBJECTS)

.PHONY: test clean
test: build/test_runner
	build/test_runner

clean:
	rm -rfd build/