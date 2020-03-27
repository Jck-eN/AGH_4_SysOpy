COMPILER = gcc
FLAGS = -Wall
LIB_NAME = lib_diff
SOURCE_LIB_NAME = lib_diff


static:
	$(COMPILER) $(FLAGS) -c $(SOURCE_LIB_NAME).c -o $(LIB_NAME).o
	ar rcs $(LIB_NAME).a $(LIB_NAME).o
	make clean

shared:
	$(COMPILER) -fPIC $(FLAGS) -c $(SOURCE_LIB_NAME).c -o $(LIB_NAME).o
	$(COMPILER) -fPIC $(FLAGS) -shared -o $(LIB_NAME).so $(LIB_NAME).o
	make clean

clean:
	rm -f *.o
