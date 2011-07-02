
#AVR_SRCS= ../rf.c ../ee.c ../print.c 
#AVR_OBJS=$(patsubst ../%.c,avr_%.o,$(AVR_SRCS))



#info: $(AVR_SRCS)
#	echo $(AVR_SRCS)
#	echo $(AVR_OBJS)

all: test run

test: 	test.o fake_avr.o rf.o print.o scan.o ee.o prompt.o io.o timer.o server.o client.o
	gcc test.o fake_avr.o rf.o print.o scan.o ee.o prompt.o io.o timer.o server.o client.o -o test 

test.o: test.c
	gcc -Wall -c test.c  -o test.o

fake_avr.o: fake_avr.c
	gcc -Wall -c fake_avr.c  -o fake_avr.o

rf.o: ../rf.c
	gcc -Ifake_avr -Wall -c ../rf.c -o rf.o

ee.o: ../ee.c
	gcc -Ifake_avr -Wall -c ../ee.c -o ee.o

io.o: ../io.c
	gcc -Ifake_avr -Wall -c ../io.c -o io.o

print.o: ../print.c
	gcc -Ifake_avr -Wall -c ../print.c -o print.o

scan.o: ../scan.c
	gcc -Ifake_avr -Wall -c ../scan.c -o scan.o

timer.o: ../timer.c
	gcc -Ifake_avr -Wall -c ../timer.c -o timer.o

server.o: ../server.c
	gcc -Ifake_avr -Wall -c ../server.c -o server.o

client.o: ../client.c
	gcc -Ifake_avr -Wall -c ../client.c -o client.o

prompt.o: ../prompt.c
	gcc -Ifake_avr -Wall -c ../prompt.c -o prompt.o

run: test
	./test

clean:
	rm -rf rf.o print.o scan.o test.o ee.o fake_avr.o prompt.o io.o timer.o server.o client.o test 
