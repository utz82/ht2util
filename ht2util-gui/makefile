CC		= g++
CXXFLAGS	= -Wall -Os -s
LDFLAGS		= `wx-config --cflags --libs`
DEPS		= ht2util.h
OBJ		= ht2util.o filebrowser.o statebrowser.o statectrl.o menu-events.o


ht2util: $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c $(CXXFLAGS) -o $@ $< $(LDFLAGS)

.PHONY: clean
clean:
	rm *o ht2util
