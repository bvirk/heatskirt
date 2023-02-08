object = consoleShellDeploy.o
out = consoleShellDeploy
$(out) : $(object)
	g++  -std=c++2a -o  $(out) $(object)
#	./$(out) 21-08/11-17:23
	./$(out) $(myArgs)
%.o : %.cpp
	g++ -std=c++2a -c $<

.PHONY : clean
clean :
	rm -f $(out) $(object)
	