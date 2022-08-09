

## directorias
SDIR = src
IDIR = include
ODIR = obj
FLAGS = -I$(IDIR)# -lm -Wall

#$OBJS = $(ODIR)/node.o $(ODIR)/main.o

## compila so o 1º ficheiro.o

all: objdir compile clean

objdir:
	mkdir -p $(ODIR)

compile: arguments.o node.o individual.o geneticoperators.o STGP.o main.o
	gcc $(ODIR)/STGP.o $(ODIR)/geneticoperators.o $(ODIR)/individual.o $(ODIR)/node.o $(ODIR)/main.o -o exec_StdGP  -lm

STGP.o: $(SDIR)/STGP.c $(IDIR)/STGP.h $(IDIR)/geneticoperators.h $(IDIR)/individual.h
	gcc -c $(SDIR)/STGP.c $(FLAGS) -o $(ODIR)/STGP.o

geneticoperators.o: $(SDIR)/geneticoperators.c $(IDIR)/geneticoperators.h $(IDIR)/individual.h $(IDIR)/node.h
	gcc -c $(SDIR)/geneticoperators.c $(FLAGS) -o $(ODIR)/geneticoperators.o

individual.o: $(SDIR)/individual.c $(IDIR)/individual.h $(IDIR)/node.h
	gcc -c $(SDIR)/individual.c $(FLAGS) -o $(ODIR)/individual.o

node.o: $(SDIR)/node.c $(IDIR)/node.h
	gcc -c $(SDIR)/node.c $(FLAGS) -o $(ODIR)/node.o

arguments.o: $(SDIR)/arguments.c
	gcc -c $(SDIR)/arguments.c $(FLAGS) -o $(ODIR)/arguments.o

main.o: $(SDIR)/main.c $(IDIR)/STGP.h
	gcc -c $(SDIR)/main.c $(FLAGS) -o $(ODIR)/main.o 

clean:
	rm -r obj/ 
	#rm exec_StdGP