
CC = gcc
CFLAGS = -Wall -Werror -O0 -g
LDFLAGS = -lgcc

SRC_FILES = $(shell find src/ -type f -name '*.c')
TEST_FILES = $(shell find test/ -type f -name '*.c')

SRC_TEST_OBJECTS = $(patsubst %.c, build/test/%.o, $(SRC_FILES)) $(patsubst %.c, build/test/%.o, $(TEST_FILES))

build/test/%.o build/test/%.d: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MM $^ -MF "$(@:.o=.d)" -MT"$@"
	$(CC) $(CFLAGS) -c -o $@ $<

build/test_runner: $(SRC_TEST_OBJECTS)
	$(CC) -o build/test_runner -r $(SRC_TEST_OBJECTS)