memcheck: testMem
	valgrind --tool=memcheck --leak-check=yes ./a.out
onCheck: testMem2
	valgrind --tool=memcheck --leak-check=yes ./a.out

ph: ph.o suffix_tree.o 
	g++ -o ph ph.o suffix_tree.o

ph.o: ph.cpp suffix_tree.h
	g++ -c ph.cpp

suffix_tree.o: suffix_tree.cpp suffix_tree.h
	g++ -c suffix_tree.cpp


testMem: suffix_tree.cpp suffix_tree.h
	g++ -g suffix_tree.cpp 

testMem2: vTest.cpp
	g++ -g vTest.cpp

test: suffix_tree.cpp suffix_tree.h
	g++ -o test suffix_tree.cpp

run: test
	./test

clean:
	rm test r1 r2 a.out *.o