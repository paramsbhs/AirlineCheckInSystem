.phony default:
default: AirlineCheckinSystem

AirlineCheckinSystem: AirlineCheckinSystem.c
	gcc -pthread AirlineCheckinSystem.c -o AirlineCheckinSystem

.PHONY clean:
clean:
	-rm -rf *.o *.exe