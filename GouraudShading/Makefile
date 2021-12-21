# compiler to be used
CC = g++

# flags to the compiler
CXX_FLAGS = -Wall -ansi -pedantic

# path to directories containing header files
INC_DIR = -I.

# GL related libraries
GL_LIBS = -lglut -lGLU -lGL

# X related libraries
X_LIBS = -lXext -lm


#### TARGETS ####

template: template.cxx
	$(CC) -o template template.cxx $(CXX_FLAGS) $(INC_DIR) $(GL_LIBS) $(X_LIBS)

run: template
	./template

clean:
	rm -f template  *.o *~
