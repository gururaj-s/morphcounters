
OUT = usimm
BINDIR = ../bin
OBJDIR = ../obj
OBJS = $(OBJDIR)/main.o $(OBJDIR)/malloc_lib.o $(OBJDIR)/hash_lib.o $(OBJDIR)/os.o $(OBJDIR)/mcache.o $(OBJDIR)/memory_controller.o $(OBJDIR)/scheduler.o $(OBJDIR)/memOrg.o $(OBJDIR)/stats.o $(OBJDIR)/memsys_control_flow.o $(OBJDIR)/ctr_sim.o  $(OBJDIR)/compression.o  $(OBJDIR)/bpc.o   $(OBJDIR)/filereader.o 
CC = gcc
DEBUG = -g
CFLAGS = -std=c99 -O3 -Wall -c $(DEBUG) 
LFLAGS = -Wall $(DEBUG) -lz -lm


$(BINDIR)/$(OUT): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(BINDIR)/$(OUT)
	chmod 777 $(BINDIR)/$(OUT)

$(OBJDIR)/malloc_lib.o: malloc_lib.c global_types.h malloc_lib.h
	$(CC) $(CFLAGS) malloc_lib.c -o $(OBJDIR)/malloc_lib.o
	chmod 777 $(OBJDIR)/malloc_lib.o

$(OBJDIR)/hash_lib.o: hash_lib.c malloc_lib.h params.h global_types.h os.h hash_lib.h
	$(CC) $(CFLAGS) hash_lib.c -o $(OBJDIR)/hash_lib.o
	chmod 777 $(OBJDIR)/hash_lib.o

$(OBJDIR)/os.o: os.c params.h global_types.h os.h hash_lib.h
	$(CC) $(CFLAGS) os.c -o $(OBJDIR)/os.o
	chmod 777 $(OBJDIR)/os.o

$(OBJDIR)/main.o: main.c processor.h configfile.h memory_controller.h scheduler.h params.h mcache.h stats.h memsys_control_flow.h ctr_sim.h
	$(CC) $(CFLAGS) main.c -o $(OBJDIR)/main.o
	chmod 777 $(OBJDIR)/main.o

$(OBJDIR)/mcache.o: mcache.c global_types.h mcache.h 
	$(CC) $(CFLAGS) mcache.c -o $(OBJDIR)/mcache.o
	chmod 777 $(OBJDIR)/mcache.o

$(OBJDIR)/memory_controller.o: memory_controller.c utlist.h utils.h params.h memory_controller.h scheduler.h processor.h
	$(CC) $(CFLAGS) memory_controller.c -o $(OBJDIR)/memory_controller.o
	chmod 777 $(OBJDIR)/memory_controller.o

$(OBJDIR)/scheduler.o: scheduler.c scheduler.h utlist.h utils.h memory_controller.h params.h
	$(CC) $(CFLAGS) scheduler.c -o $(OBJDIR)/scheduler.o
	chmod 777 $(OBJDIR)/scheduler.o

$(OBJDIR)/memOrg.o: memOrg.c memOrg.h global_types.h
	$(CC) $(CFLAGS) memOrg.c -o $(OBJDIR)/memOrg.o
	chmod 777 $(OBJDIR)/memOrg.o

$(OBJDIR)/stats.o: stats.c stats.h 
	$(CC) $(CFLAGS) stats.c -o $(OBJDIR)/stats.o
	chmod 777 $(OBJDIR)/stats.o

$(OBJDIR)/memsys_control_flow.o: memsys_control_flow.c memsys_control_flow.h memOrg.h
	$(CC) $(CFLAGS) memsys_control_flow.c -o $(OBJDIR)/memsys_control_flow.o
	chmod 777 $(OBJDIR)/memsys_control_flow.o

$(OBJDIR)/ctr_sim.o: ctr_sim.c ctr_sim.h memOrg.h compression.h
	$(CC) $(CFLAGS) ctr_sim.c -o $(OBJDIR)/ctr_sim.o
	chmod 777 $(OBJDIR)/ctr_sim.o

$(OBJDIR)/compression.o: compression.c compression.h bpc.h
	$(CC) $(CFLAGS) compression.c -o $(OBJDIR)/compression.o
	chmod 777 $(OBJDIR)/compression.o

$(OBJDIR)/bpc.o: bpc.c bpc.h
	$(CC) $(CFLAGS) bpc.c -o $(OBJDIR)/bpc.o
	chmod 777 $(OBJDIR)/bpc.o

$(OBJDIR)/filereader.o: filereader.c filereader.h
	$(CC) $(CFLAGS) filereader.c -o $(OBJDIR)/filereader.o
	chmod 777 $(OBJDIR)/filereader.o

clean:
	rm -f $(BINDIR)/$(OUT) $(OBJS)

