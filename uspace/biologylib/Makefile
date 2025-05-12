OBJS = src/common.o 	\
	src/gen.o 			\
	src/proxy.o

LIB = libbiology.a

.PHONY: clean build

$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS)

%.o : %.c
	$(CC) -o $@ -c $<

clean:
	rm -f *.o src/*.o $(LIB)
