object = consoleshell.o ardcon.o
out = consoleshell
$(out) : $(object)
	g++ -Wall -pthread --std=c++2a -o $(out) $(object) -lreadline
#	g++  -o $(out) $(object) -lncurses
#	./$(out) 21-08/11-17:23
#	$(out) $(myArgs)
%.o : %.cpp funcs.h
	g++ -Wall -pthread --std=c++2a -c $<

.PHONY : clean
clean :
	rm -f $(out) $(object)
	