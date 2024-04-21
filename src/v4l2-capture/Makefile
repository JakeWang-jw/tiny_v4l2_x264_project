
CUR_DIR := $(shell pwd)

SRCS := $(wildcard $(CUR_DIR)/*.c)
OBJS := $(SRCS:.c=.o)

.phony: all
all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.phony: clean
clean:
	$(RM) $(OBJS)
