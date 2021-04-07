CC = gcc
CFLAGS = -std=gnu17 -Wall -Wextra
obj_files = main.o icmp_send.o icmp_receive.o

all: traceroute

traceroute: $(obj_files)
	$(CC) -o traceroute $(CFLAGS) $(obj_files);

%.o: %.c
	$(CC) $(CFLAGS) -c $<;

clean:
	rm -r $(obj_files);

distclean:
	rm -r $(obj_files) traceroute;