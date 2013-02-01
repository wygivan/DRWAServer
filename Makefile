EXE = DRWAServer
SRC = DRWAServer.c
OBJ = $(subst .c,.o,$(SRC))

CXX = gcc
CPPFLAGS = -Wall -O3 -pedantic -Wextra
LIBS = -lpthread

$(EXE): $(OBJ)
	$(CXX) -o $@ $^ $(CPPFLAGS) $(LIBS)

DRWAServer.o: DRWAServer.c

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXE)
