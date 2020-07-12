CC = gcc
AR = ar
CFLAGS = -g -Wall -Wfatal-errors -Wpedantic -I.

TARGET = dctoolbox.so

all: $(TARGET)

INCS = -I./IP         \
       -I./MR         \
	   -I./Twiddle    \

SRCS = IP/ip.c                \
       MR/mr.c                \
	   Twiddle/twiddler.c      

OBJS = $(SRCS:.c=.o)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(TARGET): $(OBJS)
	$(AR) -rcs $(TARGET) $(OBJS)

EXPFUNCS = "['_mr_create_from_raw', \
			'_mr_decode_to_raw', \
			'_mr_decode_width_height', \
			'_mr_decode_filesize', \
			'_mr_valid_file', \
			'_mr_buffer_size', \
			'_ip_create_from_fields', \
			'_ip_extract_fields', \
			'_ip_insert_mr', \
			'_ip_extract_mr', \
			'_ip_valid_file']"

web: 
	emcc -O2 -s WASM=1 -s EXPORTED_FUNCTIONS=$(EXPFUNCS) -s EXTRA_EXPORTED_RUNTIME_METHODS="['cwrap', 'getValue', 'setValue']" $(INCS) $(SRCS)
	mv a.out.js site/a.out.js
	mv a.out.wasm site/a.out.wasm

# .PHONY rule keeps make from doing something with a file named clean
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)