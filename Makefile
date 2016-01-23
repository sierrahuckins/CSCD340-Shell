CC=gcc

lab8:	./history/history.o ./linkedlist/linkedList.o ./linkedlist/listUtils.o ./utils/myUtils.o ./alias/alias.o ./utils/msshrcUtils.o ./pipes/pipes.o ./tokenize/makeArgs.o
	gcc -g -o lab8 cscd340hw8.c ./history/history.o ./linkedlist/linkedList.o ./linkedlist/listUtils.o ./utils/myUtils.o ./alias/alias.o ./utils/msshrcUtils.o ./pipes/pipes.o ./tokenize/makeArgs.o

linkedList.o:	./linkedlist/linkedList.h ./linkedlist/linkedList.c 
	gcc -c -g ./linkedlist/linkedList.c

listUtils.o:	./linkedlist/listUtils.c ./linkedlist/listUtils.h
	gcc -c -g ./linkedlist/listUtils.c

myUtils.o:	./utils/myUtils.c ./utils/myUtils.h
	gcc -c -g ./utils/myUtils.c

msshrcUtils.o:	./utils/msshrcUtils.c ./utils/msshrcUtils.h
	gcc -c -g ./utils/msshrcUtils.c

history.o:	./history/history.c
	gcc -c -g ./history/history.c

alias.o:	./alias/alias.c ./alias/alias.h
	gcc -c -g ./alias/alias.c

pipes.o:	./pipes/pipes.c ./pipes/pipes.h
	gcc -c -g ./pipes/pipes.c

makesArgs.o:	./tokenize/makeArgs.c ./tokenize/makeArgs.h
	gcc -c -g ./tokenize/makeArgs.c

clean:	
	rm history/*.o
	rm alias/*.o
	rm linkedlist/*.o
	rm lab8

