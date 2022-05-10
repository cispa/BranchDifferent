FLAGS=

default: FLAGS+=-isysroot /var/sdk
default: spectre
default:
	ldid -S../common/ent.xml spectre

m1: spectre

spectre:
	clang ${FLAGS} -I. -I../common -lpthread -pthread -lm ../common/*.c *.c -o spectre

clean:
	rm spectre

