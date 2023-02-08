object = baudsvip.o
out = baudsvip
$(out) : $(object)
	g++ -Wall -std=c++11  -o $(out) $(object)
#	./$(out) 21-08/11-17:23
	./$(out) $(myArgs) -h
%.o : %.cpp
	g++ -Wall -std=c++11 -c  $<

.PHONY : clean
clean :
	rm -f $(out) $(object)
	