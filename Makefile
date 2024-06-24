.phony default:
default: AirlineCheckinSystem

AirlineCheckinSystem: AirlineCheckinSystem.c linked_list.c
	gcc -pthread AirlineCheckinSystem.c linked_list.c -o AirlineCheckinSystem

.PHONY clean:
clean:
	-rm -rf *.o *.exe