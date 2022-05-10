FLAGS=

default: FLAGS+=-isysroot /var/sdk
default: benchmark
default:
	ldid -S../common/ent.xml benchmark

m1: benchmark

benchmark:
	clang ${FLAGS} -I. -I../common -lpthread -pthread -lm ../common/*.c *.c -o benchmark

clean:
	rm benchmark

