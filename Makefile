#OBJS specifies which files to compile as part of the project
OBJS = src/main.c

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = bin/program

#This is the target that compiles our executable
all : $(OBJS)
	rm bin/*
	gcc $(OBJS) -w -lSDL2 -lSDL2_image -o $(OBJ_NAME)
	./$(OBJ_NAME)

program :
	./$(OBJ_NAME)

clean:
	rm bin/*
