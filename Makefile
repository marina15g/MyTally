OBJS	=root.o MyDataStructs.o Sorter.o SplitterMerger.o
SOURCE	=root.c MyDataStructs.c Sorter.c SplitterMerger.c
HEADER	=MyDataStructs.h
OUT0	=mytally
OUT1	=Sorter
OUT2	=SplitterMerger
CC	=gcc
FLAGS	=-g3 -c
MATH	=-lm

all: $(OBJS)
	$(CC) -o $(OUT0) root.o MyDataStructs.o $(MATH)
	$(CC) -o $(OUT1) Sorter.o MyDataStructs.o 
	$(CC) -o $(OUT2) SplitterMerger.o MyDataStructs.o

MyDataStructs.o: MyDataStructs.c MyDataStructs.h
	$(CC) $(FLAGS) MyDataStructs.c

root.o: root.c MyDataStructs.h
	$(CC) $(FLAGS) root.c

Sorter.o: Sorter.c MyDataStructs.h
	$(CC) $(FLAGS) Sorter.c

SplitterMerger.o: SplitterMerger.c MyDataStructs.h
	$(CC) $(FLAGS) SplitterMerger.c

clean:
	rm -f $(OUT0) $(OUT1) $(OUT2) $(OBJS) pipe* resultspercenter.jpg results.jpg plot1.p
