OBJS = src/biology-proxy-bio-serial.o src/info.o src/main.o
CFLAGS = '-DC_TO_JSON -Wall'
BIN = to-json

.PHONY: clean build

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o src/*.o $(BIN)
