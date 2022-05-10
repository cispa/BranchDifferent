FLAGS=


default: FLAGS+=-isysroot /var/sdk
default: sender
default: receiver
default:
	ldid -S../common/ent.xml sender
	ldid -S../common/ent.xml receiver

m1: sender
m1: receiver

sender:
	clang ${FLAGS} -I. -I../common -lpthread -pthread -lm ../common/*.c sender.c -o sender
	
receiver:
	clang ${FLAGS} -I. -I../common -lpthread -pthread -lm ../common/*.c receiver.c -o receiver
	
clean:
	rm sender receiver
