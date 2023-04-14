.SECONDARY:
print-%  : ; @echo $* = $($*)
INCLUDE = -I../include/ -I../../shared/include/ -I../../shared/include/estructuras -I../../shared/include/sockets

CPU_SRCPATH = cpu/src
CPU_OBJPATH = cpu/obj
CPU_INCLPATH = cpu/include
CPU_SRC = $(wildcard $(CPU_SRCPATH)/*.c)
CPU_OBJS =$(patsubst $(CPU_SRCPATH)/%.c,$(CPU_OBJPATH)/%.o,$(CPU_SRC))

CONSOLA_OBJPATH = consola/obj
CONSOLA_SRCPATH = consola/src
CONSOLA_INCLPATH = consola/include
CONSOLA_SRC = $(wildcard $(CONSOLA_SRCPATH)/*.c)
CONSOLA_OBJS = $(patsubst $(CONSOLA_SRCPATH)/%.c,$(CONSOLA_OBJPATH)/%.o,$(CONSOLA_SRC))

KERNEL_OBJPATH = kernel/obj
KERNEL_SRCPATH = kernel/src
KERNEL_INCLPATH = kernel/include
KERNEL_SRC = $(wildcard $(KERNEL_SRCPATH)/*.c)
KERNEL_OBJS = $(patsubst $(KERNEL_SRCPATH)/%.c,$(KERNEL_OBJPATH)/%.o,$(KERNEL_SRC))

MEMORIA_OBJPATH = memoria/obj
MEMORIA_SRCPATH = memoria/src
MEMORIA_INCLPATH = memoria/include
MEMORIA_SRC = $(wildcard $(MEMORIA_SRCPATH)/*.c)
MEMORIA_OBJS = $(patsubst $(MEMORIA_SRCPATH)/%.c,$(MEMORIA_OBJPATH)/%.o,$(MEMORIA_SRC))


SHARED_INCLUDE = -Ishared/include/ -I../include/estructuras/ -I../include/sockets/
SHARED_OBJPATH = shared/obj
SHARED_SRCPATH = shared/src
SHARED_INCPATH = shared/include
SHARED_SRC1 = $(wildcard  $(SHARED_SRCPATH)/*.c)
SHARED_SRC2 = $(wildcard  $(SHARED_SRCPATH)/estructuras/*.c)
SHARED_OBJS =  $(patsubst $(SHARED_SRCPATH)/%.c,$(SHARED_OBJPATH)/%.o,$(SHARED_SRC1)) $(patsubst $(SHARED_SRCPATH)/estructuras/%.c,shared/obj/%.o,$(SHARED_SRC2))

LINK_INCLUDE = -L../include/ -L../../shared/include
EJECUTABLES = cpu/cpu.out kernel/kernel.out memoria/memoria.out consola/consola.out *.out

CC = gcc
CFLAGS = -O0 -g -Wall -pthread
LINK_FLAGS = -Xlinker -lcommons -lpthread -lreadline -lm

#Compila y linkea todos los modulos y genera todos los ejecutables:
all: cpu.out memoria.out kernel.out consola.out

#Genera ejecutable cpu:
cpu.out: $(CPU_OBJS)
			gcc $(CFLAGS) -o cpu.out $(CPU_OBJS) $(SHARED_OBJS) $(LINK_FLAGS)
			mv cpu.out cpu

#Genera ejecutable kernel:
kernel.out: $(KERNEL_OBJS)
			gcc $(CFLAGS) -o kernel.out $^ $(SHARED_OBJS) $(LINK_FLAGS)
			mv kernel.out kernel

#Genera ejecutable memoria:
memoria.out: $(MEMORIA_OBJS)
			gcc $(CFLAGS) -o memoria.out $^ $(SHARED_OBJS) $(LINK_FLAGS)
			mv memoria.out memoria

#Genera ejecutable consola:
consola.out: $(CONSOLA_OBJS)
			gcc $(CFLAGS) -o consola.out $^ $(SHARED_OBJS) $(LINK_FLAGS)
			mv consola.out consola

#Genera objetos consola:
$(CONSOLA_OBJPATH)/%.o: $(CONSOLA_SRCPATH)/%.c $(CONSOLA_INCLPATH)/*.h $(SHARED_OBJS) $(SHARED_INCPATH)/conexion_memoria.h
			gcc $(CFLAGS) $(INCLUDE) -c $<
			mv *.o $(CONSOLA_OBJPATH)

#Genera objetos memoria:
$(MEMORIA_OBJPATH)/%.o: $(MEMORIA_SRCPATH)/%.c $(MEMORIA_INCLPATH)/*.h $(SHARED_OBJS) $(SHARED_INCPATH)/conexion_memoria.h
			gcc $(CFLAGS) $(INCLUDE) -c $<
			mv *.o $(MEMORIA_OBJPATH)

#Genera objetos kernel:
$(KERNEL_OBJPATH)/%.o:  $(KERNEL_SRCPATH)/%.c $(KERNEL_INCLPATH)/*.h $(SHARED_OBJS) $(SHARED_INCPATH)/conexion_memoria.h
			gcc $(CFLAGS) $(INCLUDE) -c $<
			mv *.o $(KERNEL_OBJPATH)

#Genera objetos cpu:
$(CPU_OBJPATH)/%.o: $(CPU_SRCPATH)/%.c $(CPU_INCLPATH)/*.h $(SHARED_OBJS) $(SHARED_INCPATH)/conexion_memoria.h
			gcc $(CFLAGS) $(INCLUDE) -c $<
			mv *.o $(CPU_OBJPATH)

#Genera objetos shared: 
$(SHARED_OBJPATH)/%.o: $(SHARED_SRCPATH)/estructuras/%.c $(SHARED_INCPATH)/estructuras/*.h
			gcc $(CFLAGS) $(SHARED_INCLUDE) -c $<
			mv *.o $(SHARED_OBJPATH)

$(SHARED_OBJPATH)/%.o: $(SHARED_SRCPATH)/%.c $(SHARED_INCPATH)/*.h
			gcc $(CFLAGS) $(SHARED_INCLUDE) -c $<
			mv *.o $(SHARED_OBJPATH)

#Borra todos los objetos y ejecutables:
clean:
	rm -f $(CPU_OBJS) $(KERNEL_OBJS) $(MEMORIA_OBJS) $(CONSOLA_OBJS) $(SHARED_OBJS) $(EJECUTABLES)

clean_log:
	rm -f cpu/logs/cpu_memoria.log cpu/logs/cpu.log cpu/logs/interrupt.log kernel/kernel.log consola/consola.log  memoria/memoria.log memoria/memoria_kernel.log memoria/memoria_cpu.log

