output := diningPhilosopher

CFLAGS :=
#CFLAGS += -DDEBUG

main : main.o  
	gcc -o $(output) main.o -lpthread -lrt

clean:
	rm *.o
	rm $(output)
