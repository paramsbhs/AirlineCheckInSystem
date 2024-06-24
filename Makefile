.phony default:
default: ACS

ACS: ACS.c linked_list.c
	gcc -pthread ACS.c linked_list.c -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe