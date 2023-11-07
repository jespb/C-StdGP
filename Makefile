

## directorias
SDIR = src
IDIR = include
ODIR = obj
#CC = clang
FLAGS = -I$(IDIR) -Wall

#$OBJS = $(ODIR)/node.o $(ODIR)/main.o

## compila so o 1º ficheiro.o

all: objdir compile clean

objdir:
	mkdir -p $(ODIR)

compile: arguments.o node.o individual.o geneticoperators.o StdGP.o main.o
	$(CC) $(ODIR)/StdGP.o $(ODIR)/geneticoperators.o $(ODIR)/individual.o $(ODIR)/node.o $(ODIR)/main.o -o exec_StdGP  -lm

StdGP.o: $(SDIR)/StdGP.c $(IDIR)/StdGP.h $(IDIR)/geneticoperators.h $(IDIR)/individual.h
	$(CC) -c $(SDIR)/StdGP.c $(FLAGS) -o $(ODIR)/StdGP.o

geneticoperators.o: $(SDIR)/geneticoperators.c $(IDIR)/geneticoperators.h $(IDIR)/individual.h $(IDIR)/node.h
	$(CC) -c $(SDIR)/geneticoperators.c $(FLAGS) -o $(ODIR)/geneticoperators.o

individual.o: $(SDIR)/individual.c $(IDIR)/individual.h $(IDIR)/node.h
	$(CC) -c $(SDIR)/individual.c $(FLAGS) -o $(ODIR)/individual.o

node.o: $(SDIR)/node.c $(IDIR)/node.h
	$(CC) -c $(SDIR)/node.c $(FLAGS) -o $(ODIR)/node.o

arguments.o: $(SDIR)/arguments.c
	$(CC) -c $(SDIR)/arguments.c $(FLAGS) -o $(ODIR)/arguments.o

main.o: $(SDIR)/main.c $(IDIR)/StdGP.h
	$(CC) -c $(SDIR)/main.c $(FLAGS) -o $(ODIR)/main.o 

clean:
	rm -r obj/ 
	#rm exec_StdGP