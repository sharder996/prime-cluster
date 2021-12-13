# name of the executable to be generated
PROG = main
# space-delimited list of header files
HDRS = msg.h client.h ini.h prime.h
# space-delimited list of source files
SRCS = main.c msg.c client.c ini.c prime.c

#######################
# Don't change these: #
#######################

# directory to store object files
OBJDIR = object
# names of object files
OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS)) 

# name of the compiler
CC = gcc
# additional compiler flags to pass in
CFLAGS = -fopenmp --static -Wall --std=c99 -L. -D_POSIX_C_SOURCE=200809L
# libraries for the linker
LIBS = -lgmp -lm -ljson-c -lz

####################
# Compiling rules: #
####################
# WARNING: *must* have a tab before each definition

# invoked when "make" is run
all : $(OBJDIR) $(PROG)

# links object files into executable
$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(PROG) $(LIBS)

# compiles source files into object files
object/%.o : %.c $(HDRS)
	$(CC) -c $(CFLAGS) $< -o $@ $(LIBS)

# creates directory to store object files
$(OBJDIR) :
	mkdir -p $@/

# cleans up object files and executable
# type "make clean" to use
# Note: you can add your own commands to remove other things (e.g. output files)
clean:
	rm -rf object/
	rm -f $(PROG)
