OBJS = src/command.o 	\
	src/gen.o 			\
	src/proxy.o 		\
	src/main.o

CFLAGS = '-DC_TO_JSON -Wall'
BIN = biologycli

.PHONY: clean build

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) -L$(BIOLOGYLIB) -lbiology

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o src/*.o $(BIN)
